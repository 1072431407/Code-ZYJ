cmake_minimum_required(VERSION 3.4.1)

file(GLOB_RECURSE SRC_FILES  *)
source_group( "src"       FILES ${SRC_FILES} )

#message(STATUS "yolo SRC_FILES = ${SRC_FILES}")
add_library(yolo
    STATIC 
    ${SRC_FILES}
	)
	
target_link_libraries(yolo
	${TNN_LIBS} 
	${OpenCV_LIBS} 
	${SYS_LIBS} 
)

