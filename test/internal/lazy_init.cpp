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
#include <string>

#include "doctest/doctest.h"
#include "fcli/internal/lazy_init.hpp"

using namespace fcli::internal;
using namespace std;
using namespace string_literals;

TEST_CASE("Initializer call on the first access") {
  LazyInit<bool> exception([] {
    throw runtime_error("");
    return false;
  });
  CHECK_THROWS_AS(static_cast<void>(*exception), runtime_error);
}

TEST_CASE("Operators") {
  LazyInit<string> str([] { return "test"s; });
  REQUIRE(*str == "test");
  str->clear();
  CHECK(str->empty());
}
