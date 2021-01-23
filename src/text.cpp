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

#include <array>
#include <map>

#include "fcli/text.hpp"

using namespace fcli;
using namespace std;

void Text::format(
    string& t_str,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    Palette t_palette) {

  const bool colors_supported = t_colors_support.has_value();
  // Using string instead of string_view to keep the binary plus operator.
  static const string
      esc_seq_start = "\x1b[",
      esc_seq_end = "m";

  /*
   * Styles.
   */

  static const map<char, unsigned short> styles{
    {'r', 0}, // Reset.
    {'b', 1}, // Bold.
    {'u', 4}, // Underline.
    {'i', 7} // Inverse.
  };

  for (const auto& [letter, style] : styles) {
    string esc_seq;

    if (colors_supported) {
      esc_seq = esc_seq_start + to_string(style) + esc_seq_end;
    }
    replace_all(t_str, "<"s + letter + '>', esc_seq);
  }

  /*
   * Colors.
   */

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
      t_palette.dim.ascii_code != Palette::Color::INVALID_ASCII_CODE;
  bool use_8_color_palette = !passed_256_color_palette;

  if (colors_supported &&
      t_colors_support == Terminal::ColorsSupport::HAS_8_COLORS) {
    use_8_color_palette = true;

    if (passed_256_color_palette) {
      // Use default 8 color palette.
      t_palette = Theme::get_palette(Theme::Name::DEFAULT);
    }
  }

  string
      foreground_esc_seq,
      background_esc_seq;

  if (use_8_color_palette) {
    foreground_esc_seq = "3";
    background_esc_seq = "4";
  } else {
    foreground_esc_seq = "38;5;";
    background_esc_seq = "48;5;";
  }

  for (const auto& [letter, color] : colors) {
    const bool remove = !colors_supported ||
        color->ascii_code == Palette::Color::INVALID_ASCII_CODE;
    const auto color_code_str = to_string(color->ascii_code);
    string esc_seq;

    // Foreground.

    if (!remove) {
      esc_seq = esc_seq_start + foreground_esc_seq +
          color_code_str + esc_seq_end;
    }
    replace_all(t_str, "~"s + letter + '~', esc_seq);

    // Background.

    const auto upper_letter = static_cast<char>(toupper(letter));
    if (!remove) {
      esc_seq = esc_seq_start + background_esc_seq +
          color_code_str + esc_seq_end;
    }
    replace_all(t_str, "~"s + upper_letter + "!~", esc_seq);

    // Background with automatic foreground.

    if (!remove) {
      esc_seq = esc_seq_start;
      if (color->invert_text) {
        esc_seq += to_string(styles.at('i')) + ';' + foreground_esc_seq;
      } else {
        esc_seq += background_esc_seq;
      }
      esc_seq += color_code_str + esc_seq_end;
    }
    replace_all(t_str, "~"s + upper_letter + '~', esc_seq);
  }
}

auto Text::format_copy(
    string t_str,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    const Palette& t_palette) -> string {

  format(t_str, t_colors_support, t_palette);
  return t_str;
};

auto Text::format_message(
    Message t_message, string_view str,
    const optional<Terminal::ColorsSupport>& t_colors_support,
    const Palette& t_palette) -> string {

  constexpr auto PREFIXES_COUNT = static_cast<size_t>(Message::_COUNT);
  static const array<string_view*, PREFIXES_COUNT> prefixes{
    &s_error_prefix, &s_warning_prefix, &s_note_prefix
  };

  const string prefix(*prefixes.at(static_cast<size_t>(t_message)));
  return format_copy(prefix + string(str), t_colors_support, t_palette);
}

void Text::replace_all(string& t_str, string_view t_from, string_view t_to) {
  if (t_from.empty()) {
    return;
  }

  size_t start_pos = 0;
  while ((start_pos = t_str.find(t_from, start_pos)) != string::npos) {
    t_str.replace(start_pos, t_from.length(), t_to);
    start_pos += t_to.length();
  }
}

auto Text::replace_all_copy(string t_str,
    string_view t_from, string_view t_to) -> string {
  replace_all(t_str, t_from, t_to);
  return t_str;
}
