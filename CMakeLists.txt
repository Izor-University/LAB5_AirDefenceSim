# Минимальная версия CMake (повышаем до 3.14 для поддержки FetchContent)
cmake_minimum_required(VERSION 3.14)

project(AirDefenseSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Указываем, где искать наши заголовочные файлы
include_directories(${CMAKE_SOURCE_DIR}/include)

# Собираем все .cpp файлы
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES})

# =====================================================================
# МАГИЯ CMAKE: Автоматическое скачивание и сборка SFML
# =====================================================================
include(FetchContent)

# Объявляем откуда качать SFML (берем стабильную версию 2.6.1 с GitHub)
FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
)

# Заставляем CMake скачать исходники прямо сейчас
message(STATUS "Скачивание SFML... (Это может занять пару минут)")
FetchContent_MakeAvailable(SFML)

# Линкуем SFML с нашим проектом
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)

# Копируем DLL-файлы SFML (если они нужны) в папку с нашим .exe
# (Актуально для Windows, чтобы программа запускалась)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )
endif()

file(COPY "${CMAKE_SOURCE_DIR}/arial.ttf"
        DESTINATION "${CMAKE_BINARY_DIR}")