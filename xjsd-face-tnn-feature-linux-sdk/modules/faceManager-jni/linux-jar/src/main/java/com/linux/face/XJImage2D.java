package com.linux.face;

/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class XJImage2D {
    //image data format type
    public static int IMAGE_FORMAT_TYPE_BGR = 1;				//opencv rgb image
    public static int IMAGE_FORMAT_TYPE_BGRA = 2;				//opencv rgba image
    public static int  IMAGE_FORMAT_TYPE_ARGB = 3;				//bitmap argb image
    public static int  IMAGE_FORMAT_TYPE_NV12 = 4;				//android nv12 image
    public static int  IMAGE_FORMAT_TYPE_NV21 = 5;				//android nv21 image
    public static int IMAGE_FORMAT_TYPE_I420 = 6;				//yuv i420 image


    private final byte[] data;
    private final int width;
    private final int height;
    private final int rotation;
    private final int type;
    public XJImage2D(byte[] data, int width, int height, int rotation, int type) {
        this.data = data;
        this.type = type;
        this.height = height;
        this.width = width;
        this.rotation = rotation;
    }


    public byte[] getData() {
        return data;
    }

    public int getHeight() {
        return height;
    }

    public int getRotation() {
        return rotation;
    }

    public int getType() {
        return type;
    }

    public int getWidth() {
        return width;
    }
}

