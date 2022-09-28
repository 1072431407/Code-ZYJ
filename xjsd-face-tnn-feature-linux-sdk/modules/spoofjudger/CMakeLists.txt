cmake_minimum_required(VERSION 3.4.1)

file(GLOB_RECURSE SRC_FILES  *)
source_group( "src"       FILES ${SRC_FILES} )

#message(STATUS "spoofjudger SRC_FILES = ${SRC_FILES}")
add_library(spoofjudger
    STATIC 
    ${SRC_FILES}
	)
	
target_link_libraries(spoofjudger
	    ${TNN_LIBS} 
	    ${OpenCV_LIBS} 
	)

