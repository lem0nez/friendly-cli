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

#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "indicator.hpp"
#include "internal/enum_array.hpp"
#include "terminal.hpp"
#include "theme.hpp"

namespace fcli {
  class Progress {
  public:
    // Styles of progress parts.
    enum class Style {
      // Determined progress only.
      LOADING_BAR,
      PERCENTS,
      // Undetermined progress only.
      INDICATOR,
      // Result messages.
      SUCCESS_SYMBOL,
      FAILURE_SYMBOL,

      _COUNT
    };

    enum class BuiltInIndicator {
      // ASCII characters.
      SPINNER,
      // Unicode characters.
      UNICODE_FLASHING_BULLET,

      _COUNT,
      _DEFAULT = SPINNER
    };

    // Placed in the front of success messages.
    enum class SuccessSymbol {
      PLUS,
      UNICODE_BULLET,
      UNICODE_CHECK_MARK,

      _COUNT,
      _DEFAULT = PLUS
    };

    // Placed in the front of failure messages.
    enum class FailureSymbol {
      MINUS,
      UNICODE_BULLET,
      UNICODE_HEAVY_MULTIPLICATION,

      _COUNT,
      _DEFAULT = MINUS
    };

    class no_space_error : public std::exception {
    public:
      [[nodiscard]] inline auto what() const noexcept -> const char* override
          { return "not enough terminal columns"; }
    };

    Progress() = default;
    // Attention: text formatting isn't supported.
    Progress(std::string_view text, bool determined,
        unsigned short width = Terminal().get_width(),
        std::ostream& ostream = std::cout);
    inline ~Progress() { hide(); }

    // Attention: output stream and hide status are not copied.
    Progress(const Progress&);
    auto operator=(const Progress&) -> Progress&;
    // Output stream can't be moved.
    Progress(Progress&&) = delete;
    auto operator=(Progress&&) -> Progress& = delete;

    // Initially, progress is hidden.
    void show();
    void hide();
    // Result message automatically hides progress.
    void finish(bool success, std::string_view message, bool format = true);

    // Add 1 to percents.
    auto operator++() -> Progress&;

    /*
     * Getters / setters.
     */

    [[nodiscard]] auto get_text() const -> std::string;
    void set_text(std::string_view);

    [[nodiscard]] inline auto is_determined() const
        { return m_determined.load(); }
    void set_determined(bool);

    [[nodiscard]] inline auto get_width() const { return m_width.load(); }
    void set_width(unsigned short);

    [[nodiscard]] auto get_ostream() -> std::ostream&;

    [[nodiscard]] inline auto is_dots_used() const
        { return m_append_dots.load(); }
    void set_append_dots(bool);

    [[nodiscard]] inline auto get_percents() const { return m_percents.load(); }
    void set_percents(double);

    [[nodiscard]] auto get_indicator() const -> Indicator;
    void set_indicator(const Indicator&);
    inline void set_indicator(BuiltInIndicator name)
        { set_indicator(get_indicator(name)); }

    [[nodiscard]] auto get_palette() const -> Palette;
    void set_palette(const Palette&);

    [[nodiscard]] auto get_colors_support() const ->
        std::optional<Terminal::ColorsSupport>;
    void set_colors_support(const std::optional<Terminal::ColorsSupport>&);

    [[nodiscard]] auto get_style(Style) const -> std::string;
    void set_style(Style, std::string_view);

    [[nodiscard]] inline auto get_success_symbol() const
        { return m_success_symbol; }
    inline void set_success_symbol(std::string_view symbol)
        { m_success_symbol = symbol; }
    inline void set_success_symbol(SuccessSymbol name)
        { set_success_symbol(get_success_symbol(name)); }

    [[nodiscard]] auto is_hidden() const -> bool;

    [[nodiscard]] inline auto get_failure_symbol() const
        { return m_failure_symbol; }
    inline void set_failure_symbol(std::string_view symbol)
        { m_failure_symbol = symbol; }
    inline void set_failure_symbol(FailureSymbol name)
        { set_failure_symbol(get_failure_symbol(name)); }

    /*
     * Static functions.
     */

    [[nodiscard]] static auto get_indicator(BuiltInIndicator) -> Indicator;
    // Using string instead of char to store Unicode characters.
    [[nodiscard]] static auto get_success_symbol(SuccessSymbol) -> std::string;
    [[nodiscard]] static auto get_failure_symbol(FailureSymbol) -> std::string;

  private:
    // Main function that updates progress.
    void update();
    // Attention: it doesn't lock mutex automatically.
    void copy_non_atomic(const Progress&);

    [[nodiscard]] static inline auto get_empty_line(unsigned short width)
        { return '\r' + std::string(width, ' ') + '\r'; }

    static constexpr std::chrono::milliseconds DOTS_UPDATE_INTERVAL{1000};
    static constexpr std::size_t MAX_DOTS = 3U;
    static constexpr double MAX_PERCENTS = 100.0;
    static constexpr unsigned short
        MAX_WIDTH = static_cast<std::size_t>(MAX_PERCENTS),
        MIN_WIDTH = std::max(
            // Determined progress size. 1U is terminating null.
            MAX_DOTS + std::size(" 100.0%") - 1U,
            // Undetermined progress size. 2U is spaces around an indicator.
            2U + Indicator::MAX_FRAME_SIZE + MAX_DOTS);

    std::string m_text;
    std::atomic<bool> m_determined{};
    std::atomic<unsigned short> m_width{Terminal().get_width()};
    std::ostream& m_ostream{std::cout};

    std::atomic<bool> m_append_dots{true};
    // Determined progress only.
    std::atomic<double> m_percents{};
    // Undetermined progress only.
    Indicator m_indicator{get_indicator(BuiltInIndicator::_DEFAULT)};
    // Set to true when indicator is changed. Used by updater.
    std::atomic<bool> m_invalidate_frame_it{};

    Palette m_palette{Theme::get_palette()};
    std::optional<Terminal::ColorsSupport>
        m_colors_support{Terminal::get_cached_colors_support()};

    internal::EnumArray<Style, std::string> m_styles
        {{"~B~", "<b>", "<b>~y~", "<b>~g~", "<b>~r~"}};
    // Updater doesn't use this members, so mutex lock doesn't required.
    std::string
        m_success_symbol{get_success_symbol(SuccessSymbol::_DEFAULT)},
        m_failure_symbol{get_failure_symbol(FailureSymbol::_DEFAULT)};

    // Executes the update function.
    std::thread m_updater;
    // Locked before reading / writing for non-atomic members.
    mutable std::mutex m_mut;

    bool m_hidden{true};
    // Used to notify updater for new changes.
    std::condition_variable m_force_update;
    mutable std::mutex m_force_update_mut;
  };
} // Namespace fcli.
