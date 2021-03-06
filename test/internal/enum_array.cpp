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

#include "doctest/doctest.h"
#include "fcli/internal/enum_array.hpp"

using namespace fcli::internal;

TEST_CASE("Functions and operators") {
  enum class Digit {ZERO, ONE, TWO, _COUNT};
  EnumArray<Digit, unsigned short> arr;

  CHECK_FALSE(arr.exists(Digit::_COUNT));
  arr = {0U, 1U, 2U};

  arr.for_each([&arr] (Digit name, unsigned short& /* val */) {
    REQUIRE(arr.exists(name));
    CHECK(arr[name] == static_cast<unsigned short>(name));
  });
}
