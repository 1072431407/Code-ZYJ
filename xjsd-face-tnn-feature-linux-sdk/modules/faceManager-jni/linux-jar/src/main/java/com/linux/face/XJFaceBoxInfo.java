package com.linux.face;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class XJFaceBoxInfo {
    public int trackId = 0;                     //人脸跟踪的trackId
    public float x = 0;                         //人脸bounding box左上角x坐标，单位是像素
    public float y = 0;                         //人脸bounding box左上角y坐标，单位是像素
    public float width = 0;                     //人脸bounding box宽度，单位是像素
    public float height = 0;                    //人脸bounding box高度，单位是像素
    public float confidence = 0;                //人脸检测阈值
    public float[] facePoints =  new float[10]; //人脸landmark坐标，单位是像素，顺序是左眼、右眼、鼻子、左嘴角、右嘴角的方式排列

    public XJFaceBoxInfo(){
    }

    /**
     * 获取人脸框rect
     */
    public XJRectF getFaceRect(){
        return new XJRectF(x,y,x+width,y+height);
    }

    /**
     * 获取人脸landmark坐标，顺序是左眼、右眼、鼻子、左嘴角、右嘴角
     */
    public List<XJPointF> getLandmarks(){
        List<XJPointF> landmarks = new ArrayList<>();
        for(int i=0;i<5;i++){
            XJPointF point = new XJPointF(facePoints[i*2],facePoints[i*2+1]);
            landmarks.add(point);
        }
        return landmarks;
    }

    @Override
    public String toString() {
        return "FaceBoxInfo{" +
                "trackId=" + trackId +
                ", x=" + x +
                ", y=" + y +
                ", width=" + width +
                ", height=" + height +
                ", confidence=" + confidence +
                ", facePoints=" + Arrays.toString(facePoints) +
                '}';
    }
}

