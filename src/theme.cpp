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

#include <stdexcept>

#include "fcli/internal/enum_array.hpp"
#include "fcli/theme.hpp"

using namespace fcli;
using namespace std;

auto Theme::get_palette(Name t_name) -> Palette {
  using namespace fcli::internal;

  const EnumArray<Name, Palette> palettes{{
    get_default_palette(),
    // Material Light.
    {
      {196U, true}, {35U, true}, {214U, false},
      {33U, true}, {207U, true}, {45U, false},
      {249U, false}
    },
    // Material Dark.
    {
      {202U, false}, {41U, true}, {220U, true},
      {75U, false}, {207U, true}, {51U, true},
      {246U, false}
    },
    // Arctic Dark.
    {
      {167U, false}, {150U, true}, {222U, true},
      {110U, true}, {182U, true}, {116U, true},
      {249U, true}
    }
  }};

  if (!palettes.exists(t_name)) {
    throw out_of_range("invalid palette index");
  }
  return palettes.get(t_name);
}

auto Theme::get_default_palette() noexcept -> Palette {
  return {
    {1U, false}, {2U, false}, {3U, false},
    {4U, false}, {5U, false}, {6U, false},
    // Dim color doesn't exist in the 8 color palette.
    {Palette::Color::INVALID_ASCII_CODE, {}}
  };
}

void Theme::set_pallete(const Palette& t_palette) {
  s_palette = t_palette;
  s_theme = Name::_USER;
}

void Theme::set_theme(Name t_name) {
  s_palette = get_palette(t_name);
  s_theme = t_name;
}
