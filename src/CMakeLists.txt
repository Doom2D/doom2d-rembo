cmake_minimum_required( VERSION 2.6 )

set( BIN doom2d )
project( ${BIN} )

set( SOURCE_ROOT ../src )

find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)

aux_source_directory( ${SOURCE_ROOT} SOURCES )

include_directories( "${SOURCE_ROOT}"
        "${SDL_INCLUDE_DIR}"
        "${SDLMIXER_INCLUDE_DIR}" )

set( INSTALL_PATH /usr/local/bin/ )

set( CMAKE_CXX_FLAGS "-O2 -Wall -MMD -MP -MF" ) 

add_executable( ${BIN} ${SOURCES} )

target_link_libraries( ${BIN} "${SDL_LIBRARY}" "${SDLMIXER_LIBRARY}" )

install( TARGETS ${BIN} DESTINATION ${INSTALL_PATH} )
