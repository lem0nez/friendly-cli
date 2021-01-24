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
#include "fcli/terminal.hpp"

using namespace fcli;
using namespace std;

TEST_CASE("Detect colors support") {
  Terminal term;

  term.set_name("");
  CHECK_FALSE(term.find_out_supported_colors().has_value());

  term.set_name("linux");
  CHECK(term.find_out_supported_colors() ==
        Terminal::ColorsSupport::HAS_8_COLORS);

  term.set_name("xterm-256color");
  CHECK(term.find_out_supported_colors() ==
        Terminal::ColorsSupport::HAS_256_COLORS);
}

TEST_CASE("get_columns_count throws exception") {
  // Pass invalid file descriptor.
  Terminal term(-1);
  CHECK_THROWS(static_cast<void>(term.get_columns_count()));
}
