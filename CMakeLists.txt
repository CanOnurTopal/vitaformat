cmake_minimum_required(VERSION 3.14)

project(VitaFormat VERSION 1.0.0 LANGUAGES CXX)

# are we the top-level project?
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(VITA_FORMAT_MAIN_PROJECT ON)
else()
    set(VITA_FORMAT_MAIN_PROJECT OFF)
endif()

option(VITA_FORMAT_BUILD_TESTS "Build tests" ${VITA_FORMAT_MAIN_PROJECT})
option(VITA_FORMAT_BUILD_BENCHMARKS "Build benchmarks" ${VITA_FORMAT_MAIN_PROJECT})
option(VITA_FORMAT_INSTALL "Generate install target" ${VITA_FORMAT_MAIN_PROJECT})

# header-only lib
add_library(vita_format INTERFACE)
add_library(Vita::format ALIAS vita_format)

target_include_directories(vita_format INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(vita_format INTERFACE cxx_std_11)

# tests
if(VITA_FORMAT_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.12.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # windows
    FetchContent_MakeAvailable(googletest)

    add_executable(vita_format_tests tests/test_format.cpp)
    target_link_libraries(vita_format_tests PRIVATE vita_format GTest::gtest GTest::gtest_main)
    target_compile_options(vita_format_tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -pedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
    )

    include(GoogleTest)
    gtest_discover_tests(vita_format_tests)
endif()

# benchmarks
if(VITA_FORMAT_BUILD_BENCHMARKS)
    add_executable(vita_format_benchmark benchmarks/benchmark.cpp)
    target_link_libraries(vita_format_benchmark PRIVATE vita_format)
    target_compile_options(vita_format_benchmark PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O2>
        $<$<CXX_COMPILER_ID:MSVC>:/O2>
    )
    target_compile_definitions(vita_format_benchmark PRIVATE NDEBUG)
endif()

# install
if(VITA_FORMAT_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY vita/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vita
        FILES_MATCHING PATTERN "*.hpp"
    )

    install(TARGETS vita_format
        EXPORT VitaFormatTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT VitaFormatTargets
        FILE VitaFormatTargets.cmake
        NAMESPACE Vita::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VitaFormat
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/VitaFormatConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/VitaFormatConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VitaFormat
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/VitaFormatConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/VitaFormatConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/VitaFormatConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VitaFormat
    )
endif()
