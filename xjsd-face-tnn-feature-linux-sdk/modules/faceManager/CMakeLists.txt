cmake_minimum_required(VERSION 3.4.1)

file(GLOB_RECURSE SRC_FILES  *)
source_group( "src"       FILES ${SRC_FILES} )

file(GLOB_RECURSE INCLUDE_FILES  ../../include/*)
source_group( "include"       FILES ${INCLUDE_FILES} )

#message(STATUS "faceManager SRC_FILES = ${SRC_FILES}")
#message(STATUS "faceManager INCLUDE_FILES = ${INCLUDE_FILES}")
if(NATIVE_ENABLE)
	add_library(faceManager
			STATIC
			${SRC_FILES}
			${INCLUDE_FILES}
			)
else()
	add_library(faceManager
			SHARED
			${SRC_FILES}
			${INCLUDE_FILES}
			)
endif()

target_link_libraries(faceManager
		alignment
		detector
		kfTrack
		recognizer
		spoofjudger
		utility
	    ${TNN_LIBS} 
	    ${OpenCV_LIBS} 
	    ${SYS_LIBS} 
	)

