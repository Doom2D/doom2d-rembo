cmake_minimum_required(VERSION 3.6)
project(doom2d C)

option(WITH_OPENGL "Build with OpenGL render" ON)
option(WITH_SOFTWARE "Build with Software render" ON)

set(D2D_GAME_ROOT .)
set(D2D_OPENGL_ROOT ${D2D_GAME_ROOT}/gl)
set(D2D_SOFTWARE_ROOT ${D2D_GAME_ROOT}/soft)
set(D2D_SDL_ROOT ${D2D_GAME_ROOT}/sdl)
set(D2D_SDLMIXER_ROOT ${D2D_GAME_ROOT}/sdlmixer)

find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)
if (WITH_OPENGL)
  find_package(OpenGL REQUIRED)
endif (WITH_OPENGL)

aux_source_directory(${D2D_GAME_ROOT} D2D_GAME_SRC)
aux_source_directory(${D2D_OPENGL_ROOT} D2D_OPENGL_SRC)
aux_source_directory(${D2D_SOFTWARE_ROOT} D2D_SOFTWARE_SRC)
aux_source_directory(${D2D_SDL_ROOT} D2D_SDL_SRC)
aux_source_directory(${D2D_SDLMIXER_ROOT} D2D_SDLMIXER_SRC)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_BUILD_TYPE)
set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3") # -DNDEBUG
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

message(STATUS "=== BUILD OPTIONS ===")
message(STATUS "BUILD:    " ${CMAKE_BUILD_TYPE})
message(STATUS "CFLAGS:   " ${CMAKE_C_FLAGS})

message(STATUS "=== RENDERS ===")
message(STATUS "SOFTWARE: " ${WITH_SOFTWARE})
message(STATUS "OPENGL:   " ${WITH_OPENGL})

if (WITH_OPENGL)
  add_executable(doom2d-gl ${D2D_GAME_SRC} ${D2D_SDL_SRC} ${D2D_SDLMIXER_SRC} ${D2D_OPENGL_SRC})
  target_include_directories(doom2d-gl PRIVATE "${D2D_GAME_ROOT}" "${SDL_INCLUDE_DIR}" "${SDLMIXER_INCLUDE_DIR}" "${OPENGL_INCLUDE_DIR}")
  target_link_libraries(doom2d-gl "${SDL_LIBRARY}" "${SDLMIXER_LIBRARY}" "${OPENGL_LIBRARY}")
endif (WITH_OPENGL)

if (WITH_SOFTWARE)
  add_executable(doom2d-soft ${D2D_GAME_SRC} ${D2D_SDL_SRC} ${D2D_SDLMIXER_SRC} ${D2D_SOFTWARE_SRC})
  target_include_directories(doom2d-soft PRIVATE "${D2D_GAME_ROOT}" "${SDL_INCLUDE_DIR}" "${SDLMIXER_INCLUDE_DIR}")
  target_link_libraries(doom2d-soft "${SDL_LIBRARY}" "${SDLMIXER_LIBRARY}")
endif (WITH_SOFTWARE)
