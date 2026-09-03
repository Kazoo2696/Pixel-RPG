cmake_minimum_required(VERSION 3.20)
project(LuminaQuest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LuminaQuest WIN32 src/main.cpp)
target_compile_definitions(LuminaQuest PRIVATE UNICODE _UNICODE NOMINMAX)
target_link_libraries(LuminaQuest PRIVATE user32 gdi32 gdiplus)

if(MSVC)
  target_compile_options(LuminaQuest PRIVATE /W4 /permissive-)
else()
  target_compile_options(LuminaQuest PRIVATE -Wall -Wextra -Wpedantic)
endif()
