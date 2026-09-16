# Add Google Test
include(FetchContent)

# Option for using system-installed googletest instead of fetching
option(USE_SYSTEM_GOOGLETEST "Use system-installed googletest instead of fetching" OFF)

# Define the version of googletest to use
set(GOOGLETEST_VERSION 1.15.2)

# Handle googletest: use system version or fetch
if(USE_SYSTEM_GOOGLETEST)
    find_package(GTest ${GOOGLETEST_VERSION} REQUIRED CONFIG)
    message(STATUS "Using system-installed googletest")
    # System GTest uses different target names
    set(GTEST_MAIN_TARGET GTest::gtest_main)
    set(GTEST_TARGET GTest::gtest)
else()
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v${GOOGLETEST_VERSION}/googletest-${GOOGLETEST_VERSION}.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
    message(STATUS "Using fetched googletest")
    # FetchContent GTest uses these target names
    set(GTEST_MAIN_TARGET gtest_main)
    set(GTEST_TARGET gtest)
endif()

# Add test executable
add_executable(mnxdom_tests
    test_beams.cpp
    test_document.cpp
    test_global.cpp
    test_layouts.cpp
    test_main.cpp
    test_parts.cpp
    test_scores.cpp
    test_sequences.cpp
    test_ottavas.cpp
    validation/test_document.cpp
    validation/test_examples.cpp
    validation/test_global.cpp
    validation/test_layouts.cpp
    validation/test_parts.cpp
    validation/test_scores.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    # Extra warnings for Clang/AppleClang
    target_compile_options(mnxdom_tests PRIVATE -Wall -Wextra -Werror -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Extra warnings for MSVC
    target_compile_options(mnxdom_tests PRIVATE /Zc:__cplusplus /W4 /WX)
endif()

target_include_directories(mnxdom_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mnxdom_tests PRIVATE
    ${GTEST_MAIN_TARGET}
    mnxdom
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(mnxdom_tests
    DISCOVERY_TIMEOUT 60 # Set timeout to 60 seconds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
    PROPERTIES
        ENVIRONMENT "GTEST_COLOR=1"
    EXTRA_ARGS --gtest_color=yes
)
