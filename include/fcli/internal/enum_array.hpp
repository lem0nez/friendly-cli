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
#include <array>
#include <type_traits>

namespace fcli::internal {
  // E is enumerator class and V is value type.
  template<class E, class... V>
  class EnumArray {
    using val_t = typename std::common_type<V...>::type;
    using arr_t = std::array<val_t, static_cast<std::size_t>(E::_COUNT)>;

  public:
    constexpr EnumArray() = default;
    constexpr explicit EnumArray(V... vals): m_arr{vals...} {}
    constexpr explicit EnumArray(arr_t orig): m_arr(std::move(orig)) {}

    [[nodiscard]] constexpr auto get(E elem) const
        { return m_arr.at(static_cast<std::size_t>(elem)); }
    constexpr void set(E elem, const val_t& val)
        { m_arr.at(static_cast<std::size_t>(elem)) = val; }

    constexpr auto operator=(const arr_t& orig) -> EnumArray& {
      m_arr = orig;
      return *this;
    }
    [[nodiscard]] constexpr auto operator[](E elem) -> val_t&
        { return m_arr[static_cast<std::size_t>(elem)]; }

  private:
    arr_t m_arr;
  };
} // Namespace fcli::internal.
