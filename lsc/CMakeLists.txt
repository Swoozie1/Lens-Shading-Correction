find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( lsc lsc.cpp )
target_link_libraries( lsc ${OpenCV_LIBS} )