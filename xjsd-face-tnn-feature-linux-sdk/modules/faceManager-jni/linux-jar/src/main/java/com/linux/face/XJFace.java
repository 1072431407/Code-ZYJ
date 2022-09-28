package com.linux.face;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;


/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class XJFace {
    public static int XJFACE_LOG_DEBUG = 0x01;                //debug log level
    public static int XJFACE_LOG_INFO = 0x02;                    //info log level
    public static int XJFACE_LOG_WARNING = 0x04;                //warning log level
    public static int XJFACE_LOG_ERROR = 0x08;                //error log level

    private final long handle;
    public int initState = -1;
    List<byte[]> readBytesList = new ArrayList<>();
    boolean loadAllModels = false;
    private final static String TAG = "XJFace";

    public XJFace() {
        handle = NativeLib.XJFaceCreate();
    }

    /**
     * 初始化，加载模型和配置
     *
     * @return 初始化成功返回0，失败放回错误码
     */
    public int init(String[] strs) {
        for (String str : strs) {
            loadAllModels = true;
            byte[] readBytes = XJUtil.ReadAssetsFilesToBytes(str);
            if (readBytes != null) {
                readBytesList.add(readBytes);
            } else {
                loadAllModels = false;
            }
        }
        if (loadAllModels) {
            initState = init(readBytesList.get(0), readBytesList.get(1), readBytesList.get(2), readBytesList.get(3), readBytesList.get(4), readBytesList.get(5));
        }
        return -1;
    }

    public int init(byte[] detector_proto
            , byte[] detector_model
            , byte[] recognizer_proto
            , byte[] recognizer_model
            , byte[] spoofjudger_proto
            , byte[] spoofjudger_model) {
        return NativeLib.XJFaceInit(handle, detector_proto, detector_model, recognizer_proto, recognizer_model, spoofjudger_proto, spoofjudger_model);
    }


    /**
     * 设置算法log输出等级
     *
     * @param level log等级,包含XJFACE_LOG_DEBUG,XJFACE_LOG_INFO,XJFACE_LOG_WARNING,XJFACE_LOG_ERROR，
     *              可以用位组合使用
     */
    public void setLogLevel(int level) {
        NativeLib.setLogLevel(handle, level);
    }

    /**
     * 人脸检测
     *
     * @param inTrackingMode 是否跟踪模式，跟踪模式
     * @return 人脸检测框列表信息
     */
    public List<XJFaceBoxInfo> detect(XJImage2D image, boolean inTrackingMode) {
        XJFaceBoxInfo[] faces = NativeLib.XJFaceDetect(handle, image.getData(), image.getWidth(), image.getHeight(), image.getRotation(), image.getType(), inTrackingMode);
        if (faces != null) {
            return Arrays.asList(faces);
        } else {
            return Collections.emptyList();
        }
    }

    /**
     * 获取算法版本信息
     *
     * @return 版本信息
     */
    public String getVersionName() {
        return NativeLib.getVersionName(handle);
    }

    /**
     * 获取人脸检测模型版本
     *
     * @return 版本信息
     */
    public String getDetectorModelVersion() {
        return NativeLib.getDetectorModelVersion(handle);
    }

    /**
     * 获取人脸识别模型版本信息
     *
     * @return 版本信息
     */
    public String getRecognizerModelVersion() {
        return NativeLib.getRecognizerModelVersion(handle);
    }

    /**
     * 获取活检模型版本信息
     *
     * @return 版本信息
     */
    public String getSpoofJudgerModelVersion() {
        return NativeLib.getSpoofJudgerModelVersion(handle);
    }

    /**
     * 图片数据人脸特征提取
     *
     * @return 特征数组
     */
    public float[] extrackFeature(XJImage2D image) {
        return NativeLib.extrackFeatureByImage(handle, image.getData(), image.getWidth(), image.getHeight(), image.getRotation(), image.getType());
    }

    /**
     * 通过跟踪id提取人脸特征
     *
     * @param trackId 跟踪id
     */
    public float[] extrackFeature(int trackId) {
        return NativeLib.extrackFeatureByTrackId(handle, trackId);
    }


    /**
     * 图片数据人脸活检
     *
     * @return 活检分数，大于0.3判定为活体
     */
    public float antiSpoofing(XJImage2D image) {
        return NativeLib.antiSpoofingByImage(handle, image.getData(), image.getWidth(), image.getHeight(), image.getRotation(), image.getType());
    }

    /**
     * 通过跟踪id提取人脸活检
     *
     * @return 活检分数，大于0.3判定为活体
     */
    public float antiSpoofing(int trackId) {
        return NativeLib.antiSpoofingByTrackId(handle, trackId);
    }

    /**
     * 人脸特征对比
     *
     * @return 匹配分数，大于0.7判定为同一个人
     */
    public float faceCompare(float[] feature1, float[] feature2) {
        return NativeLib.faceCompare(handle, feature1, feature2);
    }
}


