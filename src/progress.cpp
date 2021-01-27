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

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "fcli/progress.hpp"
#include "fcli/text.hpp"

using namespace fcli::internal;
using namespace fcli;
using namespace std;

/*
 * Constructors and copy assignment operator.
 */

Progress::Progress(string_view t_text, bool t_determined,
    unsigned short t_width, ostream& t_ostream):
    m_text(t_text), m_determined(t_determined),
    m_width(t_width), m_ostream(t_ostream) {

  if (t_width < MIN_WIDTH) {
    throw no_space_error();
  }
}

Progress::Progress(const Progress& t_other):
    m_determined(t_other.m_determined.load()),
    m_width(t_other.m_width.load()),
    m_append_dots(t_other.m_append_dots.load()),
    m_percents(t_other.m_percents.load()) {

  lock_guard lock(t_other.m_mutex);
  copy_non_atomic(t_other);
}

auto Progress::operator=(const Progress& t_other) -> Progress& {
  if (this != &t_other) {
    m_determined = t_other.m_determined.load();
    m_width = t_other.m_width.load();
    m_append_dots = t_other.m_append_dots.load();
    m_percents = t_other.m_percents.load();

    scoped_lock locks(m_mutex, t_other.m_mutex);
    copy_non_atomic(t_other);
  }
  return *this;
}

void Progress::copy_non_atomic(const Progress& t_other) {
  m_text = t_other.m_text;
  m_indicator = t_other.m_indicator;
  m_palette = t_other.m_palette;
  m_colors_support = t_other.m_colors_support;
  m_styles = t_other.m_styles;

  m_success_symbol = t_other.m_success_symbol;
  m_failure_symbol = t_other.m_failure_symbol;
}

/*
 * Functions.
 */

void Progress::show() {
  if (!m_hidden) {
    return;
  }

  m_hidden = false;
  m_updater = thread(&Progress::update, this);
}

void Progress::hide() {
  if (m_hidden) {
    return;
  }

  m_hidden = true;
  m_force_update.notify_one();

  if (m_updater.joinable()) {
    m_updater.join();
  }
}

void Progress::finish(bool t_success, string_view t_message, bool t_format) {
  hide();

  string message(t_message);
  if (t_format) {
    Text::format(message, m_colors_support, m_palette);
  }

  string prefix = " ";
  if (t_success) {
    prefix += m_styles[Style::SUCCESS_SYMBOL] + m_success_symbol;
  } else {
    prefix += m_styles[Style::FAILURE_SYMBOL] + m_failure_symbol;
  }
  prefix += "<r> ";

  Text::format(prefix, m_colors_support, m_palette);
  m_ostream << prefix + message << endl;
}

auto Progress::operator++() -> Progress& {
  set_percents(m_percents + 1.0);
  return *this;
}

/*
 * Main logic.
 */

void Progress::update() {
  using namespace chrono;
  using namespace chrono_literals;

  // Used to measure passed time.
  auto prev_time_point = steady_clock::now();
  milliseconds
      // Maximum time to wait for next update.
      wait_time,
      // Passed time after waiting.
      passed_time,
      update_frame_passed_time,
      update_dots_passed_time = DOTS_UPDATE_INTERVAL;

  decltype(Indicator::frames)::const_iterator frame_it;
  // Current number of displayed dots. If text size is more than
  // space_for_text + MAX_DOTS, then static MAX_DOTS dots will be displayed.
  size_t dots_count = MAX_DOTS;

  unsigned short space_for_text = 0U;
  ostringstream percents_oss;
  size_t loading_bar_end_pos = 0U;
  string indicator, text, dots, percents, result;

  // Cached values (that used at least twice during
  // progress generation) of atomic members.
  bool determined_cached = false;
  unsigned short width_cached = 0U;
  double percents_cached = 0.0;

  constexpr auto FIXED_INDICATOR_WIDTH = size(" - ") - 1U;

  while (true) {
    // Wait ONLY for new changes outside of thread if no
    // part of the progress should be updated automatically.
    wait_time = milliseconds::max();
    determined_cached = m_determined;
    percents_cached = m_percents;
    space_for_text = width_cached = m_width;

    /*
     * Begin of progress generation.
     */

    m_mutex.lock();

    if (determined_cached) {
      // Empty stream and reset any error flags.
      percents_oss.str(string());
      percents_oss.clear();

      percents_oss << fixed << setprecision(1) << percents_cached << '%';
      percents = percents_oss.str();

      // Plus one space that will be placed later.
      space_for_text -= percents.length() + 1U;
    } else {
      update_frame_passed_time += passed_time;

      if (update_frame_passed_time >= m_indicator.update_interval) {
        update_frame_passed_time = 0ms;

        // Iterator invalidates when container of frames is updated.
        if (m_invalidate_frame_it) {
          frame_it = m_indicator.frames.cbegin();
          m_invalidate_frame_it = false;
        }

        indicator = ' ' + m_styles[Style::INDICATOR] + *frame_it + "<r> ";
        space_for_text -= FIXED_INDICATOR_WIDTH;

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
      dots = string();
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
            "<r>" + m_styles[Style::PERCENTS]);
      } else {
        result.insert(loading_bar_end_pos, "<r>");
      }

      result = m_styles[Style::LOADING_BAR] + result +
          m_styles[Style::PERCENTS] + percents;
    } else {
      result = indicator + text + dots;
    }

    /*
     * End of progress generation.
     */

    Text::format(result, m_colors_support, m_palette);
    m_ostream << get_empty_line(width_cached) + result << flush;

    m_mutex.unlock();

    unique_lock update_lock(m_force_update_mutex);
    m_force_update.wait_for(update_lock, wait_time);
    update_lock.unlock();

    if (m_hidden) {
      lock_guard lock(m_mutex);
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
  lock_guard lock(m_mutex);
  return m_text;
}

void Progress::set_text(string_view t_text) {
  m_mutex.lock();
  m_text = t_text;
  m_mutex.unlock();
  m_force_update.notify_one();
}

void Progress::set_determined(bool t_determined) {
  m_determined = t_determined;
  m_force_update.notify_one();
}

void Progress::set_width(unsigned short t_width) {
  if (t_width < MIN_WIDTH) {
    throw no_space_error();
  }

  m_width = t_width;
  m_force_update.notify_one();
}

auto Progress::get_ostream() -> ostream& {
  lock_guard lock(m_mutex);
  return m_ostream;
}

void Progress::set_append_dots(bool t_enable) {
  m_append_dots = t_enable;
  m_force_update.notify_one();
}

void Progress::set_percents(double t_percents) {
  m_percents = min(t_percents, MAX_PERCENTS);
  m_force_update.notify_one();
}

auto Progress::get_indicator() const -> Indicator {
  lock_guard lock(m_mutex);
  return m_indicator;
}

void Progress::set_indicator(const Indicator& t_indicator) {
  m_mutex.lock();
  m_indicator = t_indicator;
  m_mutex.unlock();

  m_invalidate_frame_it = true;
  m_force_update.notify_one();
}

auto Progress::get_palette() const -> Palette {
  lock_guard lock(m_mutex);
  return m_palette;
}

void Progress::set_palette(const Palette& t_palette) {
  m_mutex.lock();
  m_palette = t_palette;
  m_mutex.unlock();
  m_force_update.notify_one();
}

auto Progress::get_colors_support() const -> optional<Terminal::ColorsSupport> {
  lock_guard lock(m_mutex);
  return m_colors_support;
}

void Progress::set_colors_support(
    const optional<Terminal::ColorsSupport>& t_colors_support) {

  m_mutex.lock();
  m_colors_support = t_colors_support;
  m_mutex.unlock();
  m_force_update.notify_one();
}

auto Progress::get_style(Style t_name) const -> string {
  lock_guard lock(m_mutex);
  return m_styles.get(t_name);
}

void Progress::set_style(Style t_name, string_view t_style) {
  m_mutex.lock();
  m_styles.set(t_name, string(t_style));
  m_mutex.unlock();
  m_force_update.notify_one();
}

/*
 * Static functions.
 */

auto Progress::get_indicator(BuiltInIndicator t_name) -> Indicator {
  using namespace chrono_literals;

  const EnumArray<BuiltInIndicator, Indicator> indicators({{
    {125ms, {"-", "\\", "|", "/", "-", "\\", "|", "/"}},
    {500ms, {"\u2022", " "}}
  }});
  return indicators.get(t_name);
}

auto Progress::get_success_symbol(SuccessSymbol t_name) -> string {
  constexpr EnumArray<SuccessSymbol, string_view> symbols{{
    "+", "\u2022", "\u2713"
  }};
  return string(symbols.get(t_name));
}

auto Progress::get_failure_symbol(FailureSymbol t_name) -> string {
  constexpr EnumArray<FailureSymbol, string_view> symbols{{
    "-", "\u2022", "\u2716"
  }};
  return string(symbols.get(t_name));
}
