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
#include "fcli/theme.hpp"

using namespace fcli;
using namespace std;

auto Theme::get_default_palette() noexcept -> Palette {
  return {
    {1, false}, {2, false}, {3, false},
    {4, false}, {5, false}, {6, false},
    {/* Dim color doesn't exist in the 8 color palette. */}
  };
}

auto Theme::get_palette(Name t_name) -> Palette {
  const auto theme_idx = static_cast<size_t>(t_name);

  if (theme_idx >= static_cast<size_t>(Name::_COUNT)) {
    throw runtime_error("invalid theme index");
  }

  constexpr auto THEMES_COUNT = static_cast<size_t>(Name::_COUNT);
  const array<Palette, THEMES_COUNT> palettes{{
    get_default_palette(),
    // Material Light.
    {
      {196, true}, {35, true}, {214, false},
      {33, true}, {207, true}, {45, false},
      {249, false}
    },
    // Material Dark.
    {
      {202, false}, {41, true}, {220, true},
      {75, false}, {207, true}, {51, true},
      {246, false}
    },
    // Arctic Dark.
    {
      {167, false}, {150, true}, {222, true},
      {110, true}, {182, true}, {116, true},
      {249, true}
    }
  }};

  return palettes.at(theme_idx);
}

void Theme::set_theme(Name t_name) {
  s_palette = get_palette(t_name);
  s_theme = t_name;
}

void Theme::set_pallete(const Palette& t_palette) {
  s_palette = t_palette;
  s_theme = Name::_USER;
}
