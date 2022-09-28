package com.linux.face;
/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class NativeLib {

    /**
     * A native method that is implemented by the 'tnn' native library,
     * which is packaged with this application.
     */
    public static native String stringFromJNI();
    public static native long XJFaceCreate();
    public static native void XJFaceDestory(long handle);
    public static native int XJFaceInit(long handle
            , byte[] detector_proto
            , byte[] detector_model
            , byte[] recognizer_proto
            , byte[] recognizer_model
            , byte[] spoofjudger_proto
            , byte[] spoofjudger_model);
    public static native XJFaceBoxInfo[] XJFaceDetect(long handle, byte[] img, int width, int height, int rotation, int imageType, boolean isTrackingMode);
    public static native void setLogLevel(long handle,int level);
    public static native String getVersionName(long handle);
    public static native String getDetectorModelVersion(long handle);
    public static native String getRecognizerModelVersion(long handle);
    public static native String getSpoofJudgerModelVersion(long handle);
    public static native float[] extrackFeatureByImage(long handle, byte[] img, int width, int height, int rotation, int imageType);
    public static native float[] extrackFeatureByTrackId(long handle, int trackId);
    public static native float antiSpoofingByImage(long handle, byte[] img, int width, int height, int rotation, int imageType);
    public static native float antiSpoofingByTrackId(long handle, int trackId);
    public static native float faceCompare(long handle, float[] feature1, float[] feature2);
    public static native float cutRight(byte[] imagebytes, int srcColumn, int srcRow,int dstColumn, int dstRow,int tailDeal);
}
