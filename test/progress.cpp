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

#include <limits>
#include <sstream>

#include "doctest/doctest.h"
#include "fcli/progress.hpp"

using namespace doctest;
using namespace fcli;
using namespace std;

TEST_CASE("Width handling") {
  CHECK_THROWS_AS(Progress({}, {}, 0U), Progress::no_space_error);

  Progress progress({}, {}, numeric_limits<unsigned short>::max());
  CHECK_THROWS_AS(progress.set_width(0U), Progress::no_space_error);
}

TEST_CASE("Progress is displayed" *
          description("Progress should be hidden immediately after a request") *
          timeout(0.1)) {

  ostringstream ostream;
  Progress progress({}, false, numeric_limits<unsigned short>::max(), ostream);
  progress.show();
  progress.hide();
  CHECK_FALSE(ostream.str().empty());
}
