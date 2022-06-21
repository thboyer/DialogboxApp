project(DialogboxApp)

cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

# Add all c source files under the src directory
file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

# Add all headers files under the include directory
# target_include_directories(${PROJECT_NAME} PRIVATE include)

# Add compiler errors/warnings flags
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>)

add_subdirectory(Dialog)
target_link_libraries(DialogboxApp dialog)

# add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
