cmake_minimum_required(VERSION 3.4.1)

file(GLOB_RECURSE SRC_FILES  *)
source_group( "src"       FILES ${SRC_FILES} )

#message(STATUS "faceManager-jni SRC_FILES = ${SRC_FILES}")
add_library(faceManager-jni
		SHARED
    ${SRC_FILES}
	)
	
target_link_libraries(faceManager-jni
		faceManager
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

