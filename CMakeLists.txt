cmake_minimum_required(VERSION 3.1)
project(uvbind LANGUAGES CXX VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	#target_compile_definitions(${PROJECT_NAME} PRIVATE "-O0")
	set(CMAKE_CXX_FLAGS "-O0")
endif()

find_package(uv REQUIRED)
set(uv_shared "${LIBUV_LIBRARIES}")
set(uv_static "${LIBUV_LIBRARIES_STATIC}")
if(" ${uv_static}" STREQUAL " ")
	set(uv_static "${uv_shared}")
endif()

file(
	GLOB_RECURSE headers
	LIST_DIRECTORIES true
	RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
	"uvbind/*.h"
	"uvbind/*.hpp"
	)
file(
	GLOB_RECURSE sources
	LIST_DIRECTORIES true
	RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
	"uvbind/*.cpp"
	)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${headers} ${sources})

add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
target_include_directories("${PROJECT_NAME}"
	SYSTEM PRIVATE "${LIBUV_INCLUDE_DIR}"
	PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
	)
target_link_libraries(${PROJECT_NAME} PUBLIC uv)

add_subdirectory(tests)

set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "cpphttpx.org/repos/uvbind")

include(CMakePackageConfigHelpers)
configure_package_config_file(
	cmake/config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
	INSTALL_DESTINATION lib
	)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-version.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
	)

install(
	TARGETS ${PROJECT_NAME}
	EXPORT ${POJECT_NAME}-export
	LIBRARY DESTINATION  lib
	PUBLIC_HEADER DESTINATION include
	COMPONENT "${PROJECT_NAME}"
	)
#INSTALL (
#DIRECTORY ${CMAKE_SOURCE_DIR}/include/
#DESTINATION include
#FILES_MATCHING PATTERN "*.h*"
#)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-version.cmake"
	DESTINATION lib
	COMPONENT "${PROJECT_NAME}"
	)

include(CPack)
