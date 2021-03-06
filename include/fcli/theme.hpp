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
#include "palette.hpp"

namespace fcli {
  class Theme {
  public:
    enum class Name {
      // For terminals that only support 8 colors. Dark background implied.
      DEFAULT,
      // For light backgrounds.
      MATERIAL_LIGHT,
      // For dark backgrounds.
      MATERIAL_DARK,
      ARCTIC_DARK,

      _COUNT,
      // Manually implemented palette.
      _USER
    };

    [[nodiscard]] static auto get_palette(Name) -> Palette;
    [[nodiscard]] static inline auto get_palette() { return s_palette; }
    [[nodiscard]] static inline auto get_theme() { return s_theme; }

    static void set_pallete(const Palette&);
    static void set_theme(Name);

  private:
    [[nodiscard]] static auto get_default_palette() noexcept -> Palette;

    static inline Name s_theme{Name::DEFAULT};
    static inline Palette s_palette{get_default_palette()};
  };
} // Namespace fcli.
