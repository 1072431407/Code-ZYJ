cmake_minimum_required(VERSION 3.4.1)
project(xjsd-face)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_DOC "build api reference document" OFF)
if (BUILD_DOC)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
      MESSAGE(FATAL_ERROR "Doxygen not found, please install Doxygen first.")
  endif()
	execute_process(COMMAND doxygen doc/Doxyfile WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

if (WIN32)
  add_definitions(-DSYS_WINDOWS)
  MESSAGE(STATUS "current system is windows")
elseif (APPLE)
  if(IOS)
    add_definitions(-DSYS_IOS)
    MESSAGE(STATUS "current system is ios")
  else()
    add_definitions(-DSYS_MACOS)
    MESSAGE(STATUS "current system is macos")
  endif()
elseif (UNIX)
  if(ANDROID)
    add_definitions(-DSYS_ANDROID)
    MESSAGE(STATUS "current system is android")
  else()
    add_definitions(-DSYS_LINUX)
    MESSAGE(STATUS "current system is linux")
  endif()
endif ()

if(ANDROID)
  add_compile_definitions(TEST_DATA_PATH="./data")
  add_compile_definitions(RECORD_DATA_PATH="/sdcard/records")
else(ANDROID)
  add_compile_definitions(TEST_DATA_PATH="${CMAKE_SOURCE_DIR}/data")
  add_compile_definitions(RECORD_DATA_PATH="${CMAKE_SOURCE_DIR}/records")
endif()



include_directories(third_part/opencv4.1.1/include)
#include_directories(third_part/libyuv/include)
include_directories(third_part/TNN/include)
if (NATIVE_ENABLE)
  if (LINUX)
    include_directories(${JAVA_HOME}/include)
    include_directories(${JAVA_HOME}/include/linux)
  endif ()
#  todo android
endif ()


if(MSVC)
  add_definitions(-DBUILDING_DLL)
  if(CMAKE_CL_64)
        set(CURRENT_MSVC_PLATFORM "x64")
        message(STATUS "Current msvc platform is ${CURRENT_MSVC_PLATFORM}")
  else()
      set(CURRENT_MSVC_PLATFORM "win32")
      message(STATUS "Current msvc platform is ${CURRENT_MSVC_PLATFORM}")
  endif()
   # opencv
   set(OpenCV_LIBS
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/opencv_world411d.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/opencv_world411.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/IlmImfd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/IlmImf.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/ade.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/ade.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/ippiwd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/ippiw.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/ittnotifyd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/ittnotify.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/libjasperd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/libjasper.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/libjpeg-turbod.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/libjpeg-turbo.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/libpngd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/libpng.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/libprotobufd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/libprotobuf.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/libtiffd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/libtiff.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/libwebpd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/libwebp.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/opencv_img_hash411d.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/opencv_img_hash411.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/opencv_ts411d.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/opencv_ts411.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/quircd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/quirc.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Debug/zlibd.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/${CURRENT_MSVC_PLATFORM}/Release/zlib.lib
  )
  # libyuv
    set(YUV_LIBS
      debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/libyuv/windows/${CURRENT_MSVC_PLATFORM}/yuv.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/libyuv/windows/${CURRENT_MSVC_PLATFORM}/yuv.lib
    )
  # TNN
  set(TNN_LIBS
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/TNN/windows/${CURRENT_MSVC_PLATFORM}/Debug/TNN.lib optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/TNN/windows/${CURRENT_MSVC_PLATFORM}/Release/TNN.lib
  )
  set(TNN_DLLS
    debug ${CMAKE_CURRENT_SOURCE_DIR}/third_part/TNN/windows/${CURRENT_MSVC_PLATFORM}/Debug/TNN.dll optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_part/TNN/windows/${CURRENT_MSVC_PLATFORM}/Release/TNN.dll
  )
elseif(ANDROID)
  message(STATUS "CMAKE_ANDROID_ARCH_ABI=${CMAKE_ANDROID_ARCH_ABI}")
  # opencv
  set(OpenCV_LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/libopencv_world.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/libIlmImf.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/liblibjpeg-turbo.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/liblibjasper.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/liblibpng.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/liblibtiff.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/liblibwebp.a
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/android/${CMAKE_ANDROID_ARCH_ABI}/libcpufeatures.a
    )
  # libyuv
  set(YUV_LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/libyuv/android/${CMAKE_ANDROID_ARCH_ABI}/libyuv.a
  )
  # TNN
  set(TNN_LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_part/TNN/android/Release/${CMAKE_ANDROID_ARCH_ABI}/libTNN.so
  )
elseif(LINUX)
  message(STATUS SYS_LINUX)

  # opencv
  set(OpenCV_LIBS
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/libIlmImf.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/libippiw.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/libittnotify.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/liblibjasper.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/liblibjpeg-turbo.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/liblibpng.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/liblibprotobuf.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/liblibtiff.a
          
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/liblibwebp.a
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/libopencv_world.so
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/opencv4.1.1/libs/linux/ubuntu/libquirc.a
          )
  # libyuv
#  set(YUV_LIBS
#          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/libyuv/android/${CMAKE_ANDROID_ARCH_ABI}/libyuv.a
#          )
  # TNN
  set(TNN_LIBS
          ${CMAKE_CURRENT_SOURCE_DIR}/third_part/TNN/linux/ubuntu/libTNN.so
         )
else()
  message(FATAL_ERROR "not support this platform.")
endif()
#message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
#message(STATUS "YUV_LIBS = ${YUV_LIBS}")
#message(STATUS "TNN_LIBS = ${TNN_LIBS}")

# platform
if(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(/MP)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")
elseif(APPLE)
	  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
	  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  if(IOS)
	   SET (CMAKE_OSX_SYSROOT iphoneos)
	   SET (CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0)
	   SET (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")
  endif()
elseif(LINUX)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -fPIC" )
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -lz -frtti -fexceptions -fPIC" )  # -fdump-class-hierarchy : dump RTTI etc.
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC" )  # -fdump-class-hierarchy : dump RTTI etc.
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")
endif()

if(MSVC)
	set(SYS_LIBS
		"Ws2_32.lib"
		"winmm.lib"
		"Strmiids.lib"
	)
elseif(ANDROID)
	set(SYS_LIBS
		"log"
    "z"
	)
endif()
set(SYS_LIBS
  ${SYS_LIBS}
)
message(STATUS "SYS_LIBS = ${SYS_LIBS}")


function(set_share_library_properties)
  set(options OPTIONEND)
  set(oneValueArgs TARGET IDENTIFIER ONEVALUEEND)
  set(multiValueArgs HEADERS MULTIVALUEEND)
  cmake_parse_arguments(SLP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(IOS)
      set_target_properties(${SLP_TARGET} PROPERTIES
        FRAMEWORK FALSE
        # FRAMEWORK_VERSION "2.2.6"
        # MACOSX_FRAMEWORK_IDENTIFIER "com.cmake.hiar.${target}"
        MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
        PUBLIC_HEADER "${INTERFACE_FILES}"
        XCODE_ATTRIBUTE_PRODUCT_NAME "leia-box_$(PLATFORM_NAME)"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${SLP_IDENTIFIER}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "8.0"
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "Yes"
        XCODE_ATTRIBUTE_DYLIB_INSTALL_NAME_BASE "@rpath"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "$(inherited) @executable_path/Frameworks @loader_path/Frameworks"
        XCODE_ATTRIBUTE_COPY_PHASE_STRIP NO
        XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT NO
        XCODE_ATTRIBUTE_SEPARATE_STRIP NO
        XCODE_ATTRIBUTE_ENABLE_BITCODE NO
      )
  endif(IOS)
endfunction(set_share_library_properties)



include_directories(modules)
include_directories(include)
include_directories(utility)

ADD_SUBDIRECTORY(modules)
ADD_SUBDIRECTORY(utility)
ADD_SUBDIRECTORY(test)

