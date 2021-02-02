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

#include "internal/enum_array.hpp"
#include "internal/lazy_init.hpp"
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
     * background color specifier, add the '!' mark after letter.
     *
     * To escape specifier, add escape character '\033' in front of him.
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

    [[nodiscard]] static inline auto format_message(
        Message type, std::string_view message,
        const std::optional<Terminal::ColorsSupport>& colors_support =
            Terminal::get_cached_colors_support(),
        const Palette& palette = Theme::get_palette()) {

      return format_copy(get_message_prefix(type) + std::string(message),
          colors_support, palette);
    }

    [[nodiscard]] static inline auto get_message_prefix(Message type) ->
        std::string { return s_prefixes->get(type); }
    static inline void set_message_prefix(Message type, std::string_view prefix)
        { s_prefixes->set(type, std::string(prefix)); }

    // Just disable colors support to remove specifiers.
    static inline void remove_specifiers(std::string& str) { format(str, {}); }
    [[nodiscard]] static inline auto
        remove_specifiers_copy(const std::string& str)
        { return format_copy(str, {}); }

  private:
    static void replace_specifier(std::string& str,
        std::string_view from, std::string_view to);

    using prefixes_t = internal::EnumArray<Message, std::string>;
    [[nodiscard]] static auto init_prefixes() -> prefixes_t;
    // Don't initialize prefixes when declaring because
    // string can throws an exception that can't be caught.
    static inline internal::LazyInit<prefixes_t> s_prefixes{init_prefixes};
  };

  namespace literals {
    [[nodiscard]] inline auto operator""
        _fmt(const char* str, std::size_t /* Unused. */) {
      return Text::format_copy(str);
    }

    [[nodiscard]] inline auto operator""
        _err(const char* message, std::size_t /* Unused. */) {
      return Text::format_message(Text::Message::ERROR, message);
    }

    [[nodiscard]] inline auto operator""
        _warn(const char* message, std::size_t /* Unused. */) {
      return Text::format_message(Text::Message::WARNING, message);
    }

    [[nodiscard]] inline auto operator""
        _note(const char* message, std::size_t /* Unused. */) {
      return Text::format_message(Text::Message::NOTE, message);
    }
  } // Namespace literals.
} // Namespace fcli.
