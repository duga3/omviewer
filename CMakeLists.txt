cmake_minimum_required (VERSION 2.8)
project (omviewer)

file(GLOB source
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB headers
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)
source_group(headers FILES ${headers})
source_group(source FILES ${source})


if(APPLE and BUILD_STATIC)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

find_package(Boost REQUIRED filesystem system)
find_package(PNG)
find_package(JPEG)
find_package(TIFF)
find_package(LibLZMA)

if(APPLE)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
endif()
find_package(X11)

if(APPLE)
	FIND_LIBRARY(IMAGEIO_LIBARY ImageIO)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
 	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(APPLE)


## Add defines for library support in CImg
add_definitions( -Dcimg_use_png )
add_definitions( -Dcimg_use_jpeg )
add_definitions( -Dcimg_use_tiff )
add_definitions( ${PNG_DEFINITIONS})
add_definitions( ${JPEG_DEFINITIONS})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/
	${Boost_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${JPEG_INCLUDE_DIRS}
	${TIFF_INCLUDE_DIRS}
	${LIBLZMA_INCLUDE_DIRS}
)
message("Boost libs ${Boost_LIBRARIES}")
message("X11 libs ${X11_LIBRARIES}")
message("LibPNG libs ${PNG_LIBRARIES}")
message("LZMA libs ${LIBLZMA_LIBRARIES}")
message("Extra libs ${EXTRA_LIBS}")
add_executable(omviewer ${source} ${headers})
target_link_libraries(	omviewer 
						${Boost_LIBRARIES} 
						${X11_LIBRARIES}
						${PNG_LIBRARIES}
						${JPEG_LIBRARIES}
						${TIFF_LIBRARIES}
						${LIBLZMA_LIBRARIES}
						${EXTRA_LIBS}
					 )
