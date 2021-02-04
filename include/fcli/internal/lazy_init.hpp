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
#include <memory>

namespace fcli::internal {
  // Initializes member of type T on the first access.
  template<class T> class LazyInit {
    // Don't use std::function to preserve the
    // noexcept specifier of a constructor.
    using initializer_t = T(*)();

  public:
    explicit LazyInit(initializer_t initializer) noexcept:
        m_initializer(initializer) {}

    [[nodiscard]] inline auto operator*() -> T& { return *get(); }
    [[nodiscard]] inline auto operator->() { return get(); }

  private:
    [[nodiscard]] auto get() -> T*;

    const initializer_t m_initializer;
    std::unique_ptr<T> m_data;
  };
} // Namespace fcli::internal.

#include "lazy_init.inl"
