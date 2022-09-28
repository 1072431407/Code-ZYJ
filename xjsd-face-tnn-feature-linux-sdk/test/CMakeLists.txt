cmake_minimum_required(VERSION 3.4.1)


#Alignment_test
add_executable(AlignmetTest
	alignment_test.cpp
)
target_link_libraries(AlignmetTest
    alignment
    ${TNN_LIBS} 
    ${OpenCV_LIBS} 
    ${SYS_LIBS} 
)


# kfTrackTest
add_executable(kfTrackTest
    kfTrackTest.cpp
    )
target_link_libraries(kfTrackTest
    kfTrack
    utility
    ${OpenCV_LIBS} 
    ${SYS_LIBS} 
    )

    
# yoloTest
add_executable(yoloTest
    yolo_test.cpp
)
target_link_libraries(yoloTest
    yolo
    utility
    ${OpenCV_LIBS} 
    ${TNN_LIBS} 
    ${SYS_LIBS} 
)

# detectorTest
add_executable(detectorTest
    detector_test.cpp
)
target_link_libraries(detectorTest
    detector
    utility
    ${OpenCV_LIBS} 
    ${TNN_LIBS} 
    ${SYS_LIBS} 
)

# spoofJudgerTest
add_executable(spoofJudgerTest
    spoofJudger_test.cpp
)
target_link_libraries(spoofJudgerTest
    spoofjudger
    utility
    ${OpenCV_LIBS} 
    ${TNN_LIBS} 
    ${SYS_LIBS} 
)

# recognizerTest
add_executable(recognizerTest
    recognizer_test.cpp
)
target_link_libraries(recognizerTest
    recognizer
    utility
    ${OpenCV_LIBS} 
    ${TNN_LIBS} 
    ${SYS_LIBS} 
)


# faceManagerTest
add_executable(faceManagerTest
    faceManager_test.cpp
)
target_link_libraries(faceManagerTest
    faceManager
    utility
    ${OpenCV_LIBS} 
    ${TNN_LIBS} 
    ${SYS_LIBS} 
)

#############################################################
# 1. 打开允许创建文件夹的开关
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# 2. 把工程加到文件夹中
set_target_properties(AlignmetTest PROPERTIES FOLDER "test")
set_target_properties(yoloTest PROPERTIES FOLDER "test")
set_target_properties(kfTrackTest PROPERTIES FOLDER "test")
set_target_properties(detectorTest PROPERTIES FOLDER "test")
set_target_properties(spoofJudgerTest PROPERTIES FOLDER "test")
set_target_properties(recognizerTest PROPERTIES FOLDER "test")
set_target_properties(faceManagerTest PROPERTIES FOLDER "test")
# 3. 给 cmake 自动创建的工程重新命名, 默认名为 "CMakePredefinedTargets" 
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "test")
