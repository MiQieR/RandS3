cmake_minimum_required(VERSION 3.16)
project(m5sticks3_sim C)

set(CMAKE_C_STANDARD 11)

set(LVGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../components/lvgl CACHE PATH "LVGL dir")

# Use vendored mini-SDL-like backends: try SDL2 first, fall back to no-window
option(SIM_USE_SDL2 "Use SDL2 for windowed display" ON)

file(GLOB_RECURSE LVGL_SOURCES ${LVGL_DIR}/src/*.c)
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*/demos/.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*/examples/.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*/draw/sdl/.*")

add_library(lvgl STATIC ${LVGL_SOURCES})
target_include_directories(lvgl PUBLIC ${LVGL_DIR} ${LVGL_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../components)
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

if(SIM_USE_SDL2)
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        add_compile_definitions(SIM_HAS_SDL2=1)
    else()
        message(STATUS "SDL2 not found; building headless simulator (no window).")
    endif()
endif()

file(GLOB APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core/i18n.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core/settings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core/answer_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core/mbti_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core/slot_game.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core/wifi_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui/app_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui/ui_theme.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui/screens/*.c
)
add_executable(simulator main_sim.c ${APP_SOURCES})
target_include_directories(simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../main
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui/screens
    ${CMAKE_CURRENT_SOURCE_DIR}/../components
    ${LVGL_DIR} ${LVGL_DIR}/src
)
target_compile_definitions(simulator PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(simulator PRIVATE lvgl m)
if(SIM_HAS_SDL2)
    target_link_libraries(simulator PRIVATE SDL2::SDL2)
endif()

add_executable(test_auto test_auto.c ${APP_SOURCES})
target_include_directories(test_auto PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../main
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/ui/screens
    ${CMAKE_CURRENT_SOURCE_DIR}/../components
    ${LVGL_DIR} ${LVGL_DIR}/src
)
target_compile_definitions(test_auto PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(test_auto PRIVATE lvgl m)
