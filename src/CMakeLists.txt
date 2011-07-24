cmake_minimum_required( VERSION 2.6 )
 
set( BIN doom2d )
project( ${BIN} )
 
set( SOURCE_ROOT ../src )
 
aux_source_directory( ${SOURCE_ROOT} SOURCES )
 
include_directories( "${SOURCE_ROOT}"
			"/usr/include"
			"/usr/include/SDL" )
 
set( INSTALL_PATH /usr/local/bin/ )
 
set( REQUIRED_LIBRARIES 
			SDL
			SDL_mixer )
 
set( CMAKE_CXX_FLAGS "-O2 -Wall -MMD -MP -MF" ) 
 
add_executable( ${BIN} ${SOURCES} )

target_link_libraries( ${BIN} ${REQUIRED_LIBRARIES} )
 
install( TARGETS ${BIN} DESTINATION ${INSTALL_PATH} )
