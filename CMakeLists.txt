cmake_minimum_required (VERSION 2.8)
project (omviewer)

file(GLOB source
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB headers
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

find_package(Boost REQUIRED filesystem system)
find_package(X11)

source_group(headers FILES ${headers})
source_group(source FILES ${source})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/
	${Boost_INCLUDE_DIRS}
)

if(APPLE)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
 	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(APPLE)

add_executable(omviewer ${source} ${headers})
message(${X11_LIBRARIES})
target_link_libraries(omviewer
                      pthread
		      ${Boost_LIBRARIES} 
		      ${X11_LIBRARIES})
