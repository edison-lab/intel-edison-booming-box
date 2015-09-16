cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE 1)

set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)
set(INSTALL_DIR ${CMAKE_SOURCE_DIR}/libs)

# include(ExternalProject)
#
# ExternalProject_Add(libsndfile_static
#     BUILD_IN_SOURCE 1
#     SOURCE_DIR "${VENDOR_DIR}/libsndfile"
#     URL "http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.25.tar.gz"
#     CONFIGURE_COMMAND ./configure --prefix=${INSTALL_DIR} --enable-static --disable-shared --disable-external-libs --host=i586-linux
# )

# ExternalProject_Add(rtaudio
#     BUILD_IN_SOURCE 1
#     SOURCE_DIR "${VENDOR_DIR}/rtaudio"
#     URL "http://www.music.mcgill.ca/~gary/rtaudio/release/rtaudio-4.1.1.tar.gz"
#     CONFIGURE_COMMAND ./configure --prefix=${INSTALL_DIR} --host=i586-linux
# )

project(main)

set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/libs/include
)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/libs/lib
)

link_libraries(
	rtaudio
	pthread
	sndfile_static
	mraa
)

if(APPLE)
	find_library(COREAUDIO_FRAMEWORK CoreAudio)
	find_library(COCOA_FRAMEWORK Cocoa)

	link_libraries(
		${COREAUDIO_FRAMEWORK}
		${COCOA_FRAMEWORK}
	)
else()
	link_libraries(
		asound
	)
endif()

add_executable(main src/main.cpp)
