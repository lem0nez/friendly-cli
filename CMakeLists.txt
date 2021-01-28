cmake_minimum_required(VERSION 3.18)
project(friendly-cli
        VERSION 0.1.0
        DESCRIPTION "\
Modern C++ library for Linux that provides an \
easy-to-use API for user interaction through CLI"
        HOMEPAGE_URL https://github.com/lem0nez/friendly-cli
        LANGUAGES CXX)

option(BUILD_TESTING "Build the unit tests (doctest framework required)" ON)

if(NOT (UNIX AND NOT APPLE))
  message(FATAL_ERROR "Only Linux is supported!")
endif()

# Require exact the C++17 library.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -pthread option provides most portability:
# it sets flags for both the preprocessor and linker.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# + ------- +
# + Library +
# + ------- +

set(SOURCES
    src/progress.cpp
    src/terminal.cpp
    src/text.cpp
    src/theme.cpp)

# Include the install directory variables (CMAKE_INSTALL_<dir>).
include(GNUInstallDirs)

add_library(fcli ${SOURCES})
set_target_properties(fcli PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(fcli PRIVATE include)
target_link_libraries(fcli PRIVATE Threads::Threads)

install(TARGETS fcli LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/fcli DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# @ONLY prevents expanding variables of the form ${VAR}.
configure_file(fcli.pc.in fcli.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/fcli.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

# + ----- +
# + Tests +
# + ----- +

set(TEST_SOURCES
    test/main.cpp
    test/progress.cpp
    test/terminal.cpp
    test/text.cpp
    test/theme.cpp)

if(BUILD_TESTING)
    find_package(doctest REQUIRED)
    add_executable(test ${TEST_SOURCES})

    target_include_directories(test PRIVATE include)
    target_link_libraries(test PRIVATE doctest::doctest fcli)
endif()
