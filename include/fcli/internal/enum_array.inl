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

namespace fcli::internal {
  template<class E, class V, std::size_t size>
  void EnumArray<E, V, size>::for_each(
      const std::function<void(E)>& t_function) const {

    for (std::size_t i = 0U; i != size; ++i) {
      t_function(static_cast<E>(i));
    }
  }
} // Namespace fcli::internal.
