cmake_minimum_required(VERSION 3.28)
project(LockpickRevealBar VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

set(LRB_BUILD_ROOT "G:/skyrim-build/LockpickRevealBar" CACHE PATH "External build artifact root")
set(LRB_PACKAGE_DIR "${LRB_BUILD_ROOT}/package" CACHE PATH "External package output directory")

# CommonLibSSE-NG (alandtse) v9 from the submodule: one DLL for SE 1.5.97, AE 1.6.x and AE 1.7.x.
# 1.7.x ships a format-5 Address Library that older CommonLib releases cannot read.
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/CommonLibSSE-NG/CMakeLists.txt")
    message(FATAL_ERROR "CommonLibSSE-NG is missing. Run: git submodule update --init")
endif()
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(REX_OPTION_INI OFF CACHE BOOL "" FORCE)
set(REX_OPTION_JSON OFF CACHE BOOL "" FORCE)
set(REX_OPTION_TOML OFF CACHE BOOL "" FORCE)
set(SKSE_SUPPORT_XBYAK OFF CACHE BOOL "" FORCE)
add_subdirectory(external/CommonLibSSE-NG EXCLUDE_FROM_ALL)
include(external/CommonLibSSE-NG/cmake/CommonLibSSE.cmake)

add_library(lrb_core STATIC
    src/Core/Config.cpp
    src/Core/Heat.cpp
    src/Core/LockLayout.cpp
    src/Core/Palette.cpp
    src/Core/RevealField.cpp
    src/Core/Style.cpp
)
target_include_directories(lrb_core PUBLIC src)

add_commonlibsse_plugin(${PROJECT_NAME}
    NAME "LockpickRevealBar"
    VERSION "${PROJECT_VERSION}"
    USE_ADDRESS_LIBRARY
    SOURCES
        src/main.cpp
)
target_sources(${PROJECT_NAME} PRIVATE
    src/Runtime/BarRenderer.cpp
    src/Runtime/LockpickHook.cpp
)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
# CommonLib v9 pulls in <Windows.h>; keep its min/max macros away from std::min/std::max.
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
target_link_libraries(${PROJECT_NAME} PRIVATE lrb_core)
target_include_directories(${PROJECT_NAME} PRIVATE src)

add_custom_target(package_mod
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LRB_PACKAGE_DIR}/SKSE/Plugins"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:LockpickRevealBar>"
        "${LRB_PACKAGE_DIR}/SKSE/Plugins/LockpickRevealBar.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/installer/Data/SKSE/Plugins/LockpickRevealBar.ini"
        "${LRB_PACKAGE_DIR}/SKSE/Plugins/LockpickRevealBar.ini"
    DEPENDS LockpickRevealBar
)

if(BUILD_TESTING)
    foreach(test_name ConfigTests HeatTests LockLayoutTests RevealFieldTests StyleTests)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE lrb_core)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()
