package com.linux.face;

/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class XJPointF {
    private float x,y;
    public XJPointF(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public void set(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public void set(XJPointF pointF) {
        this.x = pointF.x;
        this.y = pointF.y;
    }

    public void setX(float x) {
        this.x = x;
    }

    public void setY(float y) {
        this.y = y;
    }

    public float getX() {
        return x;
    }

    public float getY() {
        return y;
    }
}
