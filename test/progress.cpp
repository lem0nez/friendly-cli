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

#include <chrono>
#include <limits>
#include <sstream>
#include <thread>
#include <utility>

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

TEST_CASE("Update interval of information") {
  using namespace chrono_literals;

  ostringstream oss;
  Progress progress({}, true, oss);
  progress.show();

  progress.set_info_update_interval(10ms);
  progress = "abc";
  // Postpone this information.
  progress = pair("def", 1.0);
  CHECK(progress.get_text() == "abc");
  CHECK(progress.get_percents() == Approx(0.0));
  CHECK(progress.get_pending_text() == "def");
  CHECK(progress.get_pending_percents() == Approx(1.0));

  const auto progress_copy = progress;
  // Pending information must be copied.
  CHECK(progress_copy.get_text() == "def");
  CHECK(progress_copy.get_percents() == Approx(1.0));

  // Wait for the pending values to be applied.
  this_thread::sleep_for(15ms);
  CHECK(progress.get_text() == "def");
  CHECK(progress.get_percents() == Approx(1.0));
  CHECK_FALSE(progress.get_pending_text().has_value());
  CHECK_FALSE(progress.get_pending_percents().has_value());

  progress.set_info_update_interval({});
  progress = "xyz";
  // Percents value must be applied immediately.
  progress = 100.0;
  CHECK(progress.get_text() == "xyz");
  CHECK(progress.get_percents() == Approx(100.0));
}
