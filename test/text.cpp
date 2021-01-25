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

TEST_CASE("Format string") {
  using namespace fcli::literals;

  CHECK(Text::remove_specifiers_copy(
        "<r>\\<b><U>~r~~g!~~Y~~B!~~!M~") == "<b><U>~g!~~!M~");

  CHECK(Text::format_copy("<i>t~c~e~D~s~R~t~G!~",
        Terminal::ColorsSupport::HAS_8_COLORS,
        // 256 color palette should be downgraded to 8
        // color palette according to terminal abilities.
        Theme::get_palette(Theme::Name::MATERIAL_LIGHT)) ==
        "\x1b[7mt\x1b[36mes\x1b[41mt\x1b[42m");

  Text::set_message_prefix(Text::Message::ERROR, "prefix ");
  CHECK("test"_err == "prefix test");
}
