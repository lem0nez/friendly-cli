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

#include <cstdlib>
#include <optional>
#include <string>
#include <unistd.h>

namespace fcli {
  class Terminal {
  public:
    enum class ColorsSupport {
      HAS_8_COLORS,
      HAS_256_COLORS
    };

    explicit Terminal(
        // Pass an OPENED file descriptor of output.
        int out_file_desc = STDOUT_FILENO,
        std::string_view name = std::getenv("TERM")):
        m_out_file_desc(out_file_desc), m_name(name) {}

    [[nodiscard]] auto get_columns_count() const -> unsigned short;
    // Try to find out how many colors a terminal supports.
    [[nodiscard]] auto find_out_supported_colors() const ->
        std::optional<ColorsSupport>;

    /*
     * Getters / setters.
     */

    [[nodiscard]] inline auto get_out_file_desc() const
        { return m_out_file_desc; }
    [[nodiscard]] inline auto get_name() const { return m_name; }

    inline void set_out_file_desc(int out_file_desc)
        { m_out_file_desc = out_file_desc; }
    inline void set_name(std::string_view name) { m_name = name; }

    [[nodiscard]] static inline auto get_cached_colors_support()
        { return s_cached_colors_support; }
    [[nodiscard]] static inline auto get_cached_columns_count()
        { return s_cached_columns_count; }

    static inline void cache_colors_support(ColorsSupport colors_support)
        { s_cached_colors_support = colors_support; }
    static inline void cache_columns_count(unsigned short columns_count)
        { s_cached_columns_count = columns_count; }

  private:
    int m_out_file_desc;
    std::string_view m_name;

    static inline std::optional<ColorsSupport> s_cached_colors_support;
    static inline std::optional<unsigned short> s_cached_columns_count;
  };
} // Namespace fcli.
