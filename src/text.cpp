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

#include <cctype>
#include <map>

#include "fcli/text.hpp"

using namespace fcli;
using namespace std;

void Text::format(
    string& t_str,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    Palette t_palette) {
  using namespace string_literals;

  constexpr string_view
      ESC_SEQ_START = "\033[",
      ESC_SEQ_END = "m";

  string esc_seq;
  const bool colors_supported = t_colors_support.has_value();

  /*
   * Expand styles.
   */

  static const map<char, unsigned short> styles{
    {'r', 0U}, // Reset.
    {'b', 1U}, // Bold.
    {'u', 4U}, // Underline.
    {'i', 7U} // Inverse.
  };

  for (const auto& [letter, code] : styles) {
    esc_seq.clear();
    if (colors_supported) {
      esc_seq = string(ESC_SEQ_START) + to_string(code) + string(ESC_SEQ_END);
    }
    replace_specifier(t_str, "<"s + letter + '>', esc_seq);
  }

  /*
   * Expand colors.
   */

  // Don't use static as palette can be changed,
  // that will lead to pointers invalidation.
  const map<char, const Palette::Color*> colors{
    {'r', &t_palette.red},
    {'g', &t_palette.green},
    {'y', &t_palette.yellow},
    {'b', &t_palette.blue},
    {'m', &t_palette.magenta},
    {'c', &t_palette.cyan},
    {'d', &t_palette.dim}
  };

  const bool passed_256_color_palette =
      t_palette.dim.code != Palette::Color::INVALID_CODE;
  bool use_8_color_palette = !passed_256_color_palette;

  if (colors_supported &&
      t_colors_support == Terminal::ColorsSupport::HAS_8_COLORS) {
    use_8_color_palette = true;

    if (passed_256_color_palette) {
      // Use the default 8 color palette.
      t_palette = Theme::get_palette(Theme::Name::DEFAULT);
    }
  }

  string foreground_esc_seq, background_esc_seq;
  if (use_8_color_palette) {
    foreground_esc_seq = "3";
    background_esc_seq = "4";
  } else {
    foreground_esc_seq = "38;5;";
    background_esc_seq = "48;5;";
  }

  for (const auto& [letter, color] : colors) {
    const bool remove = !colors_supported ||
        color->code == Palette::Color::INVALID_CODE;
    const auto code_str = to_string(color->code);
    esc_seq.clear();

    // Foreground.
    if (!remove) {
      esc_seq = string(ESC_SEQ_START) + foreground_esc_seq +
          code_str + string(ESC_SEQ_END);
    }
    replace_specifier(t_str, "~"s + letter + '~', esc_seq);

    // Background.
    const char upper_letter =
        // Letter should be converted to unsigned
        // char because cctype functions require it.
        static_cast<char>(toupper(static_cast<unsigned char>(letter)));
    if (!remove) {
      esc_seq = string(ESC_SEQ_START) + background_esc_seq +
          code_str + string(ESC_SEQ_END);
    }
    replace_specifier(t_str, "~"s + upper_letter + "!~", esc_seq);

    // Background with automatic foreground.
    if (!remove) {
      esc_seq = ESC_SEQ_START;

      if (color->invert_text) {
        esc_seq += to_string(styles.at('i')) + ';' + foreground_esc_seq;
      } else {
        esc_seq += background_esc_seq;
      }

      esc_seq += code_str + string(ESC_SEQ_END);
    }
    replace_specifier(t_str, "~"s + upper_letter + '~', esc_seq);
  }
}

auto Text::format_copy(
    string t_str,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    const Palette& t_palette) -> string {

  format(t_str, t_colors_support, t_palette);
  return t_str;
};

void Text::replace_specifier(string& t_str,
    string_view t_from, string_view t_to) {
  if (t_from.empty()) {
    return;
  }

  size_t start_pos = 0U;
  while ((start_pos = t_str.find(t_from, start_pos)) != string::npos) {
    if (start_pos != 0U && t_str[start_pos - 1U] == '\033') {
      // Skip escaped specifier.
      t_str.erase(start_pos - 1U, 1U);
      continue;
    }

    t_str.replace(start_pos, t_from.length(), t_to);
    start_pos += t_to.length();
  }
}

auto Text::init_prefixes() -> prefixes_t {
  return prefixes_t{{
    "<b>~r~Error<r> ~d~|<r> ",
    "<b>~y~Warning<r> ~d~|<r> ",
    "<b>~c~Note<r> ~d~|<r> "
  }};
}
