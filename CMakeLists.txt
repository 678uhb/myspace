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

include_directories ("${PROJECT_SOURCE_DIR}")

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-std=c++11)
	add_compile_options(-g)
	add_compile_options(-pthread)
	set(CMAKE_EXE_LINKER_FLAGS "-g -pthread")
endif(CMAKE_COMPILER_IS_GNUCXX)

SET(MYSPACE_SOURCES ${MYSPACE_SOURCES} 
${PROJECT_SOURCE_DIR}/myspace/logger/logger.cpp
${PROJECT_SOURCE_DIR}/myspace/threadpool/threadpool.cpp) 

ADD_LIBRARY(${PROJECT_NAME}_shared SHARED ${MYSPACE_SOURCES})
target_compile_definitions(${PROJECT_NAME}_shared PRIVATE MYSPACE_EXPORTS MYSPACE_SHARED)

ADD_LIBRARY(${PROJECT_NAME}_static STATIC ${MYSPACE_SOURCES})


INSTALL(TARGETS myspace_shared myspace_static
		LIBRARY	DESTINATION	lib
		ARCHIVE	DESTINATION lib)

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/myspace
		DESTINATION	"include"
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp")		
