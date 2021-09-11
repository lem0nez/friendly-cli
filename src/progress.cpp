/*
 * Copyright © 2021 Nikita Dudko. All rights reserved.
 * Contacts: <nikita.dudko.95@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>
#include <iomanip>
#include <sstream>

#include "fcli/progress.hpp"
#include "fcli/text.hpp"

using namespace std;
using namespace chrono;
using namespace chrono_literals;

using namespace fcli;
using namespace fcli::internal;

Progress::Progress(string_view t_text, bool t_determined,
    unsigned short t_width,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    const Palette& t_palette):

    m_text(t_text), m_determined(t_determined),
    m_width(min(t_width, MAX_WIDTH)),
    m_formatted_styles(format_default_styles(t_colors_support, t_palette)) {

  if (t_width < MIN_WIDTH) {
    throw no_space_error();
  }
}

Progress::Progress(const Progress& t_other):
    m_determined(t_other.m_determined.load()),
    m_width(t_other.m_width.load()),
    m_append_dots(t_other.m_append_dots.load()),
    m_info_update_interval(t_other.m_info_update_interval.load()) {

  copy_percents(t_other);
  lock_guard lock(t_other.m_mut);
  copy_non_atomic(t_other);
}

auto Progress::operator=(const Progress& t_other) -> Progress& {
  if (this != &t_other) {
    m_determined = t_other.m_determined.load();
    m_width = t_other.m_width.load();
    m_append_dots = t_other.m_append_dots.load();
    m_info_update_interval = t_other.m_info_update_interval.load();

    copy_percents(t_other);
    scoped_lock locks{m_mut, t_other.m_mut};
    copy_non_atomic(t_other);
    notify();
  }
  return *this;
}

void Progress::copy_percents(const Progress& t_other) {
  const auto pending_percents = t_other.m_pending_percents.load();
  if (pending_percents >= 0.0) {
    m_percents = pending_percents;
  } else {
    m_percents = t_other.m_percents.load();
  }
}

void Progress::copy_non_atomic(const Progress& t_other) {
  if (t_other.m_pending_text) {
    m_text = *t_other.m_pending_text;
  } else {
    m_text = t_other.m_text;
  }
  m_formatted_styles = t_other.m_formatted_styles;

  m_indicator = t_other.m_indicator;
  m_invalidate_frame_it = true;

  m_success_symbol = t_other.m_success_symbol;
  m_failure_symbol = t_other.m_failure_symbol;
}

void Progress::show() {
  if (!m_hidden) {
    return;
  }

  m_hidden = m_force_update = false;
  m_invalidate_frame_it = true;
  m_updater = thread(&Progress::update, this);
}

void Progress::hide() {
  if (m_hidden) {
    return;
  }
  m_hidden = true;
  notify();

  if (m_updater.joinable()) {
    m_updater.join();
  }
}

void Progress::finish(bool t_success, string_view t_message) {
  hide();

  string prefix = " ";
  if (t_success) {
    prefix += m_formatted_styles[Style::SUCCESS_SYMBOL] + m_success_symbol;
  } else {
    prefix += m_formatted_styles[Style::FAILURE_SYMBOL] + m_failure_symbol;
  }
  prefix += m_formatted_styles[Style::PLAIN] + ' ';

  m_ostream << prefix + string(t_message) << endl;
}

void Progress::notify() {
  m_force_update_mut.lock();
  m_force_update = true;
  m_force_update_mut.unlock();
  m_force_update_cv.notify_one();
}

/*
 * Operators.
 */

auto Progress::operator++() -> Progress& {
  set_percents(m_percents + 1.0);
  return *this;
}

auto Progress::operator+=(double t_percents) -> Progress& {
  set_percents(m_percents + t_percents);
  return *this;
}

auto Progress::operator=(double t_percents) -> Progress& {
  set_percents(t_percents);
  return *this;
}

auto Progress::operator=(string_view t_text) -> Progress& {
  set_text(t_text);
  return *this;
}

auto Progress::operator=(const pair<string, double>& t_info) -> Progress& {
  set_info(t_info.first, t_info.second);
  return *this;
}

/*
 * Main logic.
 */

void Progress::update() {
  // Used to measure passed time.
  auto prev_time_point = steady_clock::now();
  milliseconds
      // Maximum time to wait for next update.
      wait_time,
      // Passed time after waiting.
      passed_time,
      // Set maximum value to immediately update indicator.
      update_frame_passed_time = milliseconds::max(),
      update_dots_passed_time;

  decltype(Indicator::frames)::const_iterator frame_it;
  // Current number of displayed dots. If text size is more than
  // space_for_text + MAX_DOTS, then static MAX_DOTS dots will be displayed.
  size_t dots_count = 0U;

  unsigned short space_for_text = 0U;
  size_t loading_bar_end_pos = 0U;
  string indicator, text, dots, percents, result;

  ostringstream percents_oss;
  percents_oss << fixed;
  percents_oss.precision(1);

  // Cached values (that used at least twice during
  // progress generation) of atomic members.
  bool determined_cached = false;
  unsigned short width_cached = 0U;
  double percents_cached = 0.0;
  milliseconds info_update_interval_cached;
  time_point<steady_clock> next_info_update_cached;

  // Don't use milliseconds::max() as it leads to overflow.
  constexpr milliseconds MAX_WAIT_TIME = 1h;

  while (true) {
    // Wait ONLY for new changes outside if no part
    // of the progress should be updated automatically.
    wait_time = MAX_WAIT_TIME;
    determined_cached = m_determined;
    percents_cached = m_percents;
    info_update_interval_cached = m_info_update_interval;
    next_info_update_cached = m_next_info_update;
    space_for_text = width_cached = m_width;

    /*
     * Begin of progress generation.
     */

    m_mut.lock();

    if (determined_cached) {
      // Empty stream and reset any error flags.
      percents_oss.str({});
      percents_oss.clear();

      // Is it time to release all pending values?
      if (next_info_update_cached <= prev_time_point) {
        const auto pending_percents = m_pending_percents.load();
        // Is there pending percents value?
        if (pending_percents >= 0.0) {
          m_percents = percents_cached = pending_percents;
          m_pending_percents = -1.0;
          m_next_info_update = prev_time_point + info_update_interval_cached;
        }
      }
      percents_oss << percents_cached << '%';
      percents = percents_oss.str();

      // Plus one space that will be placed later.
      space_for_text -= percents.length() + 1U;
    } else {
      // Iterator invalidates when new indicator is set.
      if (m_invalidate_frame_it) {
        frame_it = m_indicator.frames.cbegin();
        m_invalidate_frame_it = false;
        // Immediately show new frame.
        update_frame_passed_time = m_indicator.update_interval;
      } else {
        update_frame_passed_time += passed_time;
      }

      if (update_frame_passed_time >= m_indicator.update_interval) {
        update_frame_passed_time = 0ms;

        // Don't store formatted styles here as they can be changed and
        // user will wait for new indicator iteration to see changes.
        indicator = frame_it->substr(0U, Indicator::MAX_FRAME_SIZE);
        // 2U is spaces around indicator.
        space_for_text -= 2U + m_indicator.fixed_visible_length;

        if (++frame_it == m_indicator.frames.cend()) {
          frame_it = m_indicator.frames.cbegin();
        }
      }
      wait_time = m_indicator.update_interval - update_frame_passed_time;
    }

    if (m_append_dots) {
      space_for_text -= MAX_DOTS;

      if (space_for_text < m_text.length()) {
        // Use static dots if text doesn't fit terminal width.
        dots = string(MAX_DOTS, '.');
      } else {
        update_dots_passed_time += passed_time;

        if (update_dots_passed_time >= DOTS_UPDATE_INTERVAL) {
          update_dots_passed_time = 0ms;

          if (++dots_count > MAX_DOTS) {
            dots_count = 0U;
          }
          dots = string(dots_count, '.');
        }
        wait_time =
            min(DOTS_UPDATE_INTERVAL - update_dots_passed_time, wait_time);
      }
    } else {
      dots.clear();
    }

    if (next_info_update_cached <= prev_time_point) {
      if (m_pending_text) {
        m_text = *m_pending_text;
        m_pending_text.reset();
        m_next_info_update = prev_time_point + info_update_interval_cached;
      }
    } else {
      wait_time = min(duration_cast<milliseconds>(
          next_info_update_cached - prev_time_point), wait_time);
    }
    // Trim text from the end if need.
    text = m_text.substr(0U, space_for_text);

    if (determined_cached) {
      result = text + dots;
      result += string(width_cached -
          (result.length() + percents.length() + 1U), ' ') + ' ';

      loading_bar_end_pos = static_cast<size_t>(
          round(static_cast<double>(width_cached) *
          (percents_cached / MAX_PERCENTS)));

      if (loading_bar_end_pos >= result.length()) {
        percents.insert(loading_bar_end_pos - result.length(),
            m_formatted_styles[Style::PLAIN] +
            m_formatted_styles[Style::PERCENTS]);
      } else {
        result.insert(loading_bar_end_pos, m_formatted_styles[Style::PLAIN]);
      }

      result = m_formatted_styles[Style::PLAIN] +
          m_formatted_styles[Style::LOADING_BAR] + result +
          m_formatted_styles[Style::PERCENTS] + percents;
    } else {
      result = ' ' + m_formatted_styles[Style::INDICATOR] + indicator +
          m_formatted_styles[Style::PLAIN] + ' ' + text + dots;
    }

    /*
     * End of progress generation.
     */

    m_ostream << get_empty_line(width_cached) + result << flush;
    m_mut.unlock();

    unique_lock update_lock(m_force_update_mut);
    m_force_update_cv.wait_for(update_lock, wait_time,
        [this] { return m_force_update; });
    m_force_update = false;
    update_lock.unlock();

    if (m_hidden) {
      lock_guard lock(m_mut);
      m_ostream << get_empty_line(width_cached) << flush;
      return;
    }

    passed_time = duration_cast<milliseconds>(
        steady_clock::now() - prev_time_point);
    prev_time_point = steady_clock::now();
  }
}

/*
 * Getters / setters.
 */

auto Progress::get_text() const -> string {
  lock_guard lock(m_mut);
  return m_text;
}

void Progress::set_determined(bool t_determined) {
  m_determined = t_determined;
  notify();
}

void Progress::set_width(unsigned short t_width) {
  if (t_width < MIN_WIDTH) {
    throw no_space_error();
  }
  m_width = min(t_width, MAX_WIDTH);
  notify();
}

auto Progress::get_ostream() -> ostream& {
  lock_guard lock(m_mut);
  return m_ostream;
}

void Progress::set_append_dots(bool t_enable) {
  m_append_dots = t_enable;
  notify();
}

void Progress::set_info(const optional<string>& t_text,
    optional<double> t_percents) {
  if (t_percents) {
    t_percents = clamp(*t_percents, 0.0, MAX_PERCENTS);
  }
  lock_guard lock(m_mut);

  if (const auto update_interval = m_info_update_interval.load();
      update_interval != 0ms) {
    const auto current_time = steady_clock::now();
    if (m_next_info_update.load() > current_time) {
      if (t_text) {
        m_pending_text = t_text;
      }
      if (t_percents) {
        m_pending_percents = *t_percents;
      }
      return;
    }
    m_next_info_update = current_time + update_interval;
  }

  if (t_text) {
    m_text = *t_text;
  } else if (m_pending_text) {
    m_text = *m_pending_text;
  }
  m_pending_text.reset();

  if (t_percents) {
    m_percents = *t_percents;
  } else if (const auto pending_percents = m_pending_percents.load();
      pending_percents >= 0.0) {
    m_percents = pending_percents;
  }
  m_pending_percents = -1.0;

  notify();
}

void Progress::set_info_update_interval(milliseconds t_interval) {
  m_info_update_interval = t_interval;
  const auto
      next_info_update = m_next_info_update.load(),
      current_time = steady_clock::now();
  if (next_info_update > current_time) {
    const auto update_wait_time =
        duration_cast<milliseconds>(next_info_update - current_time);
    m_next_info_update = current_time + min(update_wait_time, t_interval);
    notify();
  }
}

auto Progress::get_pending_text() const -> optional<string> {
  lock_guard lock(m_mut);
  return m_pending_text;
}

auto Progress::get_pending_percents() const -> optional<double> {
  const auto percents = m_pending_percents.load();
  if (percents >= 0.0) {
    return percents;
  }
  return {};
}

auto Progress::get_indicator() const -> Indicator {
  lock_guard lock(m_mut);
  return m_indicator;
}

void Progress::set_indicator(const Indicator& t_indicator) {
  lock_guard lock(m_mut);
  m_indicator = t_indicator;
  m_invalidate_frame_it = true;
  notify();
}

void Progress::set_style(Style t_part, string_view t_style,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    const Palette& t_palette) {

  lock_guard lock(m_mut);
  m_formatted_styles.set(t_part,
      Text::format_copy(string(t_style), t_colors_support, t_palette));
  notify();
}

/*
 * Static functions.
 */

auto Progress::format_default_styles(
    const optional<Terminal::ColorsSupport>& t_colors_support,
    const Palette& t_palette) -> styles_t {

  styles_t formatted_styles;
  s_default_styles->for_each([&] (Style name, string& style) {
    formatted_styles.set(name,
        Text::format_copy(style, t_colors_support, t_palette));
  });
  return formatted_styles;
}

auto Progress::get_indicator(BuiltInIndicator t_name) -> Indicator {
  const EnumArray<BuiltInIndicator, Indicator> indicators({{
    {125ms, 1U, {"-", "\\", "|", "/", "-", "\\", "|", "/"}},
    {500ms, 1U, {"\u2022", " "}}
  }});
  return indicators.get(t_name);
}

auto Progress::get_success_symbol(SuccessSymbol t_name) -> string {
  constexpr EnumArray<SuccessSymbol, string_view> symbols({
    "+", "\u2022", "\u2713", "\u2714"
  });
  return string(symbols.get(t_name));
}

auto Progress::get_failure_symbol(FailureSymbol t_name) -> string {
  constexpr EnumArray<FailureSymbol, string_view> symbols({
    "-", "\u2022", "\u00d7", "\u2716"
  });
  return string(symbols.get(t_name));
}
