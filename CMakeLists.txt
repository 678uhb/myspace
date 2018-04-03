cmake_minimum_required (VERSION 2.6)
project (myspace)
set (myspace_VERSION_MAJOR 1)
set (myspace_VERSION_MINOR 0)

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}") 
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")  
    MESSAGE(STATUS "current platform: Linux ")  
	set(MYSPACE_LINUX 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")  
    MESSAGE(STATUS "current platform: Windows")  
	set(MYSPACE_WINDOWS 1)
ELSE ()  
    MESSAGE(FATAL_ERROR "unsupported platform: ${CMAKE_SYSTEM_NAME}")  
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

include_directories ("${PROJECT_SOURCE_DIR}/..")

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-std=c++11)
	add_compile_options(-g)
	add_compile_options(-pthread)
	set(CMAKE_EXE_LINKER_FLAGS "-g -pthread")
endif(CMAKE_COMPILER_IS_GNUCXX)

configure_file (
	"${PROJECT_SOURCE_DIR}/myspace.cmake"
	"${PROJECT_SOURCE_DIR}/myspace_config.h"
	)

SET(src ${src} 
${PROJECT_SOURCE_DIR}/logger/logger.cpp
${PROJECT_SOURCE_DIR}/threadpool/threadpool.cpp) 
# SET(src ${src} ${PROJECT_SOURCE_DIR}/threadpool/threadpool.cpp) 

ADD_LIBRARY(${PROJECT_NAME}_shared SHARED ${src})
target_compile_definitions(${PROJECT_NAME}_shared PRIVATE MYSPACE_EXPORTS MYSPACE_SHARED)

ADD_LIBRARY(${PROJECT_NAME}_static STATIC ${src})

INSTALL(TARGETS ${PROJECT_NAME}_shared ${PROJECT_NAME}_static
		RUNTIME	DESTINATION	${PROJECT_SOURCE_DIR}/lib
		LIBRARY	DESTINATION	${PROJECT_SOURCE_DIR}/lib
		ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/lib)
 
