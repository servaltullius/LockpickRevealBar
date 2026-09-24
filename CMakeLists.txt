cmake_minimum_required(VERSION 3.28)
project(LockpickRevealBar VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

set(LRB_BUILD_ROOT "G:/skyrim-build/LockpickRevealBar" CACHE PATH "External build artifact root")
set(LRB_PACKAGE_DIR "${LRB_BUILD_ROOT}/package" CACHE PATH "External package output directory")

find_package(CommonLibSSE CONFIG REQUIRED)

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
    SOURCES
        src/main.cpp
)
target_sources(${PROJECT_NAME} PRIVATE
    src/Runtime/BarRenderer.cpp
    src/Runtime/LockpickHook.cpp
)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
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
