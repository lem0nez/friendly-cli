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

namespace fcli::internal {
  // E is enumerator class and V is values type.
  template<class E, class V, auto size = static_cast<std::size_t>(E::_COUNT)>
  // Wrapper around standard array, where
  // key is a field of the enumerator class.
  class EnumArray {
    using arr_t = std::array<V, size>;

  public:
    constexpr EnumArray() = default;
    constexpr explicit EnumArray(arr_t orig): m_arr(std::move(orig)) {}

    [[nodiscard]] constexpr auto get(E elem) const
        { return m_arr.at(static_cast<std::size_t>(elem)); }
    constexpr void set(E elem, const V& val)
        { m_arr.at(static_cast<std::size_t>(elem)) = val; }

    [[nodiscard]] constexpr auto exists(E elem) const
        { return static_cast<std::size_t>(elem) < size; }

    constexpr auto operator=(const arr_t& orig) -> EnumArray& {
      m_arr = orig;
      return *this;
    }
    [[nodiscard]] constexpr auto operator[](E elem) -> V&
        { return m_arr[static_cast<std::size_t>(elem)]; }

  private:
    arr_t m_arr;
  };
} // Namespace fcli::internal.