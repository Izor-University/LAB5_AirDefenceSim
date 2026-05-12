# Минимальная версия CMake
cmake_minimum_required(VERSION 3.14)

project(AirDefenseSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# =====================================================================
# МАГИЯ CMAKE: Скачивание SFML, ядра ImGui и моста ImGui-SFML
# =====================================================================
include(FetchContent)

# 1. Загружаем SFML
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.1)
message(STATUS "Скачивание SFML...")
FetchContent_MakeAvailable(SFML)

# 2. Загружаем ЯДРО Dear ImGui (Версия 1.89 отлично работает с SFML)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.89.9)
message(STATUS "Скачивание ядра ImGui...")
FetchContent_MakeAvailable(imgui)

# 3. Настройки для моста ImGui-SFML
# Говорим мосту, где лежит ядро ImGui:
set(IMGUI_DIR ${imgui_SOURCE_DIR} CACHE PATH "Path to imgui core" FORCE)
# Отключаем поиск SFML в системе (используем тот, что скачали в шаге 1):
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Do not search for SFML" FORCE)

# 4. Загружаем биндинг ImGui-SFML
FetchContent_Declare(ImGui-SFML GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git GIT_TAG 2.6.x)
message(STATUS "Скачивание моста ImGui-SFML...")
FetchContent_MakeAvailable(ImGui-SFML)

# =====================================================================
# ЛИНКОВКА
# =====================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        ImGui-SFML::ImGui-SFML
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )
endif()

file(COPY "${CMAKE_SOURCE_DIR}/arial.ttf" DESTINATION "${CMAKE_BINARY_DIR}")