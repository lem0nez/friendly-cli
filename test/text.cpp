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
#include "fcli/text.hpp"

using namespace fcli;
using namespace std;

TEST_CASE("String replacement") {
  // Multiple matches.
  CHECK(Text::replace_all_copy("test", "t", "_") == "_es_");
  // Unicode character.
  CHECK(Text::replace_all_copy("st\u2022ng", "\u2022", "ri") == "string");

  // Empty parameters.
  CHECK(Text::replace_all_copy("", "string", "test").empty());
  CHECK(Text::replace_all_copy("string", "", "test") == "string");
  CHECK(Text::replace_all_copy("test", "test", "").empty());
}

TEST_CASE("Format string") {
  CHECK(Text::remove_specifiers_copy("<r><B>~r~~g!~~Y~~B!~~!M~") ==
        "<B>~g!~~!M~");

  CHECK(Text::format_copy("<u>t~c~e~D~s~R~t~G!~",
        Terminal::ColorsSupport::HAS_8_COLORS,
        Theme::get_palette(Theme::Name::MATERIAL_LIGHT)) ==
        "\x1b[4mt\x1b[36mes\x1b[41mt\x1b[42m");

  using namespace fcli::literals;
  CHECK("test"_note == "Note | test");
}
