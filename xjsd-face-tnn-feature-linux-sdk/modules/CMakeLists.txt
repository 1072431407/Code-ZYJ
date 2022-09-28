cmake_minimum_required(VERSION 3.4.1)


ADD_SUBDIRECTORY(kfTrack)
ADD_SUBDIRECTORY(alignment)
ADD_SUBDIRECTORY(yolo)
ADD_SUBDIRECTORY(detector)
ADD_SUBDIRECTORY(recognizer)
ADD_SUBDIRECTORY(spoofjudger)
ADD_SUBDIRECTORY(faceManager)
if(NATIVE_ENABLE)
    ADD_SUBDIRECTORY(faceManager-jni)
endif()

# 1. 打开允许创建文件夹的开关
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# 2. 把工程加到文件夹中
set_target_properties(kfTrack PROPERTIES FOLDER "modules")
set_target_properties(alignment PROPERTIES FOLDER "modules")
set_target_properties(yolo PROPERTIES FOLDER "modules")
set_target_properties(detector PROPERTIES FOLDER "modules")
set_target_properties(recognizer PROPERTIES FOLDER "modules")
set_target_properties(spoofjudger PROPERTIES FOLDER "modules")
set_target_properties(faceManager PROPERTIES FOLDER "modules")
if(NATIVE_ENABLE)
    set_target_properties(faceManager-jni PROPERTIES FOLDER "modules")
endif()
# 3. 给 cmake 自动创建的工程重新命名, 默认名为 "CMakePredefinedTargets"
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "modules")
