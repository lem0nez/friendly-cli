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

#include <chrono>
#include <forward_list>
#include <string>

namespace fcli {
  // Used by undetermined progress.
  struct Indicator {
    std::chrono::milliseconds update_interval;
    /*
     * Each frame should be one character size. Using string because char
     * can't store Unicode characters and wide char type isn't portable.
     *
     * Using forward_list because random access to an element doesn't
     * required and iteration through elements is always from begin to end.
     */
    std::forward_list<std::string> frames;
  };
} // Namespace fcli.
