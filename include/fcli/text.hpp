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

#include <optional>
#include <string>

#include "terminal.hpp"
#include "theme.hpp"

namespace fcli {
  class Text {
  public:
    enum class Message {
      ERROR,
      WARNING,
      NOTE,

      _COUNT
    };

    /*
     * Style specifier ('r', 'b', 'u', 'i') wraps
     * in the angle brackets ('<' and '>').
     *
     * Foreground color specifier ('r', 'g', 'y', 'b',
     * 'm', 'c', 'd') wraps in the two '~' characters.
     *
     * Background color specifier is uppercase specifier of foreground color.
     * To disable automatic inverting of foreground color when using the
     * background color specifier, add the '!' mark before letter.
     */
    static void format(
        std::string&,
        const std::optional<Terminal::ColorsSupport>& =
            Terminal::get_cached_colors_support(),
        Palette = Theme::get_palette());

    [[nodiscard]] static auto format_copy(
        std::string,
        const std::optional<Terminal::ColorsSupport>& =
            Terminal::get_cached_colors_support(),
        const Palette& = Theme::get_palette()) -> std::string;

    [[nodiscard]] static auto format_message(
        Message, std::string_view,
        const std::optional<Terminal::ColorsSupport>& =
            Terminal::get_cached_colors_support(),
        const Palette& = Theme::get_palette()) -> std::string;

    // Just disable colors support to remove specifiers.
    static inline void remove_specifiers(std::string& str) { format(str, {}); }
    [[nodiscard]] static inline auto remove_specifiers_copy(const std::string& str)
        { return format_copy(str, {}); }

    static void replace_all(std::string& str,
        std::string_view from, std::string_view to);
    [[nodiscard]] static auto replace_all_copy(std::string str,
        std::string_view from, std::string_view to) -> std::string;

    /*
     * Getters / setters.
     */

    [[nodiscard]] static inline auto get_error_prefix()
        { return s_error_prefix; }
    [[nodiscard]] static inline auto get_warning_prefix()
        { return s_warning_prefix; }
    [[nodiscard]] static inline auto get_note_prefix() { return s_note_prefix; }

    static inline void set_error_prefix(std::string_view prefix)
        { s_error_prefix = prefix; }
    static inline void set_warning_prefix(std::string_view prefix)
        { s_warning_prefix = prefix; }
    static inline void set_note_prefix(std::string_view prefix)
        { s_note_prefix = prefix; }

  private:
    static inline std::string_view
        s_error_prefix = "<b>~r~Error ~d~|<r> ",
        s_warning_prefix = "<b>~y~Warning ~d~|<r> ",
        s_note_prefix = "<b>~c~Note ~d~|<r> ";
  };

  namespace literals {
    [[nodiscard]] inline auto operator""
        _fmt(const char* str, size_t /* Unused. */) {
      return Text::format_copy(str);
    }

    [[nodiscard]] inline auto operator""
        _err(const char* message, size_t /* Unused. */) {
      return Text::format_message(Text::Message::ERROR, message);
    }

    [[nodiscard]] inline auto operator""
        _warn(const char* message, size_t /* Unused. */) {
      return Text::format_message(Text::Message::WARNING, message);
    }

    [[nodiscard]] inline auto operator""
        _note(const char* message, size_t /* Unused. */) {
      return Text::format_message(Text::Message::NOTE, message);
    }
  } // Namespace literals.
} // Namespace fcli.
