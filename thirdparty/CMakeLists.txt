include(FetchContent)

if(CHESS_ENGINE_USE_EXTERNAL_JSON)
    find_package(nlohmann_json 3.2.0 REQUIRED)
else()
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG 97795502d1824062547085e9e329d0c0d0c499f3)

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        FetchContent_Populate(json)
        add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

block()
    set(WITH_GFLAGS OFF)
    set(WITH_GTEST OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_TESTING OFF)

    FetchContent_Declare(
      glog
      GIT_REPOSITORY https://github.com/google/glog.git
      GIT_TAG b33e3bad4c46c8a6345525fd822af355e5ef9446
    )
    FetchContent_MakeAvailable(glog)
endblock()

block()
    set(INSTALL_GTEST OFF CACHE BOOL "Install GTest - no need since embedded project")
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "Only because I dont need it yet")
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    FetchContent_MakeAvailable(googletest)
endblock()

block()
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "Dont automatically download and build benchmark deps")
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "dont build tests on benchmarks")
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "dont build tests on benchmarks")
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "not needed for embedded project")
    set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "not needed for embedded project")
    FetchContent_Declare(
      benchmark
      GIT_REPOSITORY https://github.com/google/benchmark/
      GIT_TAG fba5dd147d34b793e9ae2554f34f36a0bf507c6e
    )
    FetchContent_MakeAvailable(benchmark)
endblock()
