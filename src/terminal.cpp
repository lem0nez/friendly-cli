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
#include <cstdlib>
#include <sys/ioctl.h>

#include "fcli/terminal.hpp"

using namespace fcli;
using namespace std;

auto Terminal::get_width() const -> unsigned short {
  struct winsize size{};
  const int err = ioctl(m_out_file_desc, TIOCGWINSZ, &size);

  if (err != 0) {
    throw runtime_error("couldn't get terminal width");
  }
  return size.ws_col;
}

auto Terminal::find_out_supported_colors() const -> optional<ColorsSupport> {
  optional<ColorsSupport> colors_support;

  if (m_name.empty()) {
    return colors_support;
  }

  constexpr size_t COLORED_TERMS_COUNT = 14U;
  constexpr array<string_view, COLORED_TERMS_COUNT> colored_terms{
    "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm",
    "linux", "msys", "putty", "rxvt", "screen","vt100", "xterm"
  };

  for (const auto& t : colored_terms) {
    if (m_name.find(t) == 0U) {
      // Name starts with t.
      colors_support = ColorsSupport::HAS_8_COLORS;

      if (m_name.find(string(t) + "-256") != string::npos) {
        colors_support = ColorsSupport::HAS_256_COLORS;
      }
      break;
    }
  }
  return colors_support;
}

auto Terminal::getenv(string_view t_name) -> string {
  const auto val = std::getenv(string(t_name).c_str());

  if (val == nullptr) {
    return {};
  }
  return val;
}
