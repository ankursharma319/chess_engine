include(FetchContent)

if(CHESS_ENGINE_USE_EXTERNAL_JSON)
    find_package(nlohmann_json 3.2.0 REQUIRED)
else()
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG v3.10.4)

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        FetchContent_Populate(json)
        add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

set(INSTALL_GTEST OFF CACHE BOOL "Install GTest - no need since embedded project")
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  glog
  GIT_REPOSITORY https://github.com/google/glog.git
  GIT_TAG v0.6.0
)
set(WITH_GFLAGS OFF)

FetchContent_MakeAvailable(googletest glog)
