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
    {1, false, false},
    {2, false, true},
    {3, false, true},
    {4, false, false},
    {5, false, false},
    {6, false, true},
    {/* Dim color doesn't exist in the 8 color palette. */}
  };
}

void Theme::set(Name t_name) {
  constexpr auto THEMES_COUNT = static_cast<size_t>(Name::_COUNT);
  const array<Palette, THEMES_COUNT> palettes{{
    get_default_palette(),
    // Material Light.
    {
      {196, true, true},
      {35, true, true},
      {214, false, false},
      {33, true, true},
      {207, true, true},
      {45, false, false},
      {249, false, false}
    },
    // Material Dark.
    {
      {202, false, false},
      {41, true, true},
      {220, true, true},
      {75, false, false},
      {207, true, true},
      {51, true, true},
      {246, false, false}
    },
    // Arctic Dark.
    {
      {167, false, false},
      {150, true, true},
      {222, true, true},
      {110, true, true},
      {182, true, true},
      {116, true, true},
      {249, true, true}
    }
  }};

  s_palette = palettes.at(static_cast<size_t>(t_name));
}
