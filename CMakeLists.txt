cmake_minimum_required (VERSION 2.6)
project (myspace)

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}") 
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")  
    MESSAGE(STATUS "current platform: Linux ")  
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	SET(GCC_COVERAGE_LINK_FLAGS    "${GCC_COVERAGE_LINK_FLAGS} 	-pthread")
	SET(GCC_COVERAGE_LINK_FLAGS    "${GCC_COVERAGE_LINK_FLAGS}	-g")
	set(myspace_linux 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")  
    MESSAGE(STATUS "current platform: Windows")  
	set(myspace_windows 1)
ELSE ()  
    MESSAGE(FATAL_ERROR "unsupported platform: ${CMAKE_SYSTEM_NAME}")  
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")



configure_file (
	"${PROJECT_SOURCE_DIR}/myspace.cmake"
	"${PROJECT_SOURCE_DIR}/myspace_config.h"
	)

SET(src ${src} ${PROJECT_SOURCE_DIR}/logger/logger.cpp) 
SET(src ${src} ${PROJECT_SOURCE_DIR}/threadpool/threadpool.cpp) 

#include (CheckFunctionExists.cmake)
#check_function_exists (epoll myspace_has_epoll)
#check_function_exists (select myspace_has_select)
 
set (myspace_VERSION_MAJOR 1)
set (myspace_VERSION_MINOR 0)

ADD_LIBRARY(myspace SHARED ${src}) 
ADD_LIBRARY(myspace_static STATIC ${src})

#check_function_exists (epoll HAVE_EPOLL)

include_directories ("${PROJECT_SOURCE_DIR}/..")


INSTALL(TARGETS myspace  myspace_static
    LIBRARY DESTINATION libs   
    ARCHIVE DESTINATION libs) 
 
