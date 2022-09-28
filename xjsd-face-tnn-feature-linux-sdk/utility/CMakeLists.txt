cmake_minimum_required(VERSION 3.4.1)

file(GLOB_RECURSE SRC_FILES  *)
file(GLOB_RECURSE XLOG_FILES  xlog/*)
set(SRC_FILES 
    ${XLOG_FILES}
    ${SRC_FILES})
source_group( "src"       FILES ${SRC_FILES} )

add_library(utility
    STATIC 
    ${SRC_FILES}
	)

