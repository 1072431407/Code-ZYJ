package com.linux.face;


/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class XJRectF {
    private float l, t, r, b;
    public XJRectF(float l, float t, float r, float b){
        this.l = l;
        this.t = t;
        this.r = r;
        this.b = b;
    }

    public void set(XJRectF rectF) {
        this.l = rectF.l;
        this.t = rectF.t;
        this.r = rectF.r;
        this.b = rectF.b;
    }

    public void set(float l, float t, float r, float b) {
        this.l = l;
        this.t = t;
        this.r = r;
        this.b = b;
    }

    public float getTop() {
        return t;
    }

    public float getRight() {
        return r;
    }

    public float getBottom() {
        return b;
    }

    public float getLeft() {
        return l;
    }
}
