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
#include "fcli/terminal.hpp"

using namespace doctest;
using namespace fcli;
using namespace std;

TEST_CASE("Width handling") {
  CHECK_THROWS_AS(Progress({}, {}, 0U), Progress::no_space_error);

  Progress progress({}, {}, numeric_limits<unsigned short>::max());
  CHECK_THROWS_AS(progress.set_width(0U), Progress::no_space_error);
}

TEST_CASE("Progress is displayed" *
          description("it should be hidden immediately after a request") *
          timeout(0.1)) {

  ostringstream oss;
  Progress progress({}, false, oss);
  progress.show();
  progress.hide();
  CHECK_FALSE(oss.str().empty());
}

TEST_CASE("Result messages") {
  ostringstream oss;
  Progress progress({}, {}, oss);

  progress.set_style(Progress::Style::PLAIN, "<r>",
      Terminal::ColorsSupport::HAS_8_COLORS);
  progress.set_style(Progress::Style::SUCCESS_SYMBOL, "<b>",
      Terminal::ColorsSupport::HAS_8_COLORS);
  progress.set_success_symbol("+");
  progress.finish(true, "success");
  CHECK(oss.str() == " \033[1m+\033[0m success\n");
  oss.str({});

  progress.set_style(Progress::Style::PLAIN, "<u>", {});
  progress.set_style(Progress::Style::FAILURE_SYMBOL, "<i>", {});
  progress.set_failure_symbol("-");
  progress.finish(false, "failure");
  CHECK(oss.str() == " - failure\n");
}
