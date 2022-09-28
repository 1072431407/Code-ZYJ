#include <jni.h>
#include <string>
#include <XJFace.h>
#include <stdio.h>
#include <cstring>

#define  LOG_TAG "xjface_jni"
#ifdef SYS_ANDROID
#include <android/log.h>
#define LOGDT(fmt, tag, ...)  __android_log_print(ANDROID_LOG_DEBUG, tag, (fmt), ##__VA_ARGS__);                                                                      
#define LOGIT(fmt, tag, ...)  __android_log_print(ANDROID_LOG_INFO, tag, (fmt), ##__VA_ARGS__);                                                                      
#define LOGET(fmt, tag, ...)  __android_log_print(ANDROID_LOG_ERROR, tag, (fmt), ##__VA_ARGS__);                                                                      
#else
#define LOGDT(fmt, tag, ...) fprintf(stdout, ("D/%s: " fmt), tag, ##__VA_ARGS__)
#define LOGIT(fmt, tag, ...) fprintf(stdout, ("I/%s: " fmt), tag, ##__VA_ARGS__)
#define LOGET(fmt, tag, ...) fprintf(stderr, ("E/%s: " fmt), tag, ##__VA_ARGS__)
#endif  //__ANDROID__

#define LOGD(fmt, ...) LOGDT(fmt, LOG_TAG, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOGIT(fmt, LOG_TAG, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOGET(fmt, LOG_TAG, ##__VA_ARGS__)


#ifdef SYS_ANDROID
#define JNI_FUNCTION(n) Java_com_xjsd_face_NativeLib_##n
#endif

#ifdef SYS_LINUX
//todo com.upuphone.xr.face.business do jar
//#define JNI_FUNCTION(n) Java_com_upuphone_xr_face_business_NativeLib_##n
#define JNI_FUNCTION(n) Java_com_linux_face_NativeLib_##n
#endif
//#define CLASS_NAME(n)        "com/linux/face/"##n

static jclass clsRecogInfo = nullptr;
static jmethodID midconstructorRecogInfo = nullptr;
static jfieldID fidx = nullptr;
static jfieldID fidy = nullptr;
static jfieldID fidwidth = nullptr;
static jfieldID fidheight = nullptr;
static jfieldID fidconf = nullptr;
static jfieldID fidtrackid = nullptr;
static jfieldID fidlandmarks = nullptr;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("JNI_LOAD");
    return JNI_VERSION_1_6;
}

extern "C" {
JNIEXPORT jstring JNICALL JNI_FUNCTION(stringFromJNI(JNIEnv * env, jobject /* this */)) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jlong JNICALL JNI_FUNCTION(XJFaceCreate(JNIEnv * env, jobject /* this */)) {
    if (clsRecogInfo == NULL) {
        clsRecogInfo = static_cast<jclass>(env->NewGlobalRef(
                env->FindClass("com/linux/face/XJFaceBoxInfo")));
        midconstructorRecogInfo = env->GetMethodID(clsRecogInfo, "<init>", "()V");
        fidx = env->GetFieldID(clsRecogInfo, "x", "F");
        fidy = env->GetFieldID(clsRecogInfo, "y", "F");
        fidwidth = env->GetFieldID(clsRecogInfo, "width", "F");
        fidheight = env->GetFieldID(clsRecogInfo, "height", "F");
        fidtrackid = env->GetFieldID(clsRecogInfo, "trackId", "I");
        fidconf = env->GetFieldID(clsRecogInfo, "confidence", "F");
        fidlandmarks = env->GetFieldID(clsRecogInfo, "facePoints", "[F");
    }
    return (jlong) new XJFace();
}

JNIEXPORT void JNICALL JNI_FUNCTION(XJFaceDestory(JNIEnv * env, jobject /* this */, jlong handle)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        delete xjface;
    }
}

//init
JNIEXPORT jint JNICALL JNI_FUNCTION(XJFaceInit(JNIEnv * env, jobject /* this */, jlong handle
        , jbyteArray detector_proto
        , jbyteArray detector_model
        , jbyteArray recognizer_proto
        , jbyteArray recognizer_model
        , jbyteArray spoofjudger_proto
        , jbyteArray spoofjudger_model)) {
    int retcode = -1;
    if (handle) {
        jbyte *p;
        jsize size;
        p = (jbyte *) env->GetByteArrayElements(detector_proto, 0);
        size = env->GetArrayLength(detector_proto);
        std::string detector_proto_content((char *) p, size);

        p = (jbyte *) env->GetByteArrayElements(detector_model, 0);
        size = env->GetArrayLength(detector_model);
        std::string detector_model_content((char *) p, size);

        p = (jbyte *) env->GetByteArrayElements(recognizer_proto, 0);
        size = env->GetArrayLength(recognizer_proto);
        std::string recognizer_proto_content((char *) p, size);

        p = (jbyte *) env->GetByteArrayElements(recognizer_model, 0);
        size = env->GetArrayLength(recognizer_model);
        std::string recognizer_model_content((char *) p, size);

        p = (jbyte *) env->GetByteArrayElements(spoofjudger_proto, 0);
        size = env->GetArrayLength(spoofjudger_proto);
        std::string spoofjudger_proto_content((char *) p, size);

        p = (jbyte *) env->GetByteArrayElements(spoofjudger_model, 0);
        size = env->GetArrayLength(spoofjudger_model);
        std::string spoofjudger_model_content((char *) p, size);

        XJFace *xjface = (XJFace *) handle;
        retcode = xjface->init(
                detector_proto_content, detector_model_content, recognizer_proto_content,
                recognizer_model_content, spoofjudger_proto_content, spoofjudger_model_content
        );
        if (retcode) {
            LOGE("XJFace init failed!retcode=%d", retcode);
        } else {
            LOGE("XJFace init success!");
        }
    }
    return retcode;
}

//deteclinux-jart
JNIEXPORT jobjectArray JNICALL JNI_FUNCTION(
        XJFaceDetect(JNIEnv * env, jobject /* this */, jlong handle,
        jbyteArray img_bytes, jint width, jint height, jint rotation,
        jint imageType, jboolean isTrackingMode)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        std::vector<XJFace::FaceBoxInfo> faceBoxInfoList;
        static vector<uint8_t> img_data;
        jsize size = env->GetArrayLength(img_bytes);
        if (img_data.size() != size) {
            img_data.resize(size);
        }
        env->GetByteArrayRegion(img_bytes, 0, size, (jbyte *) img_data.data());
        //LOGI("XJFaceDetect width=%d,height=%d,rotation=%d,imageType=%d,isTrackingMode=%d", width,height, rotation, imageType, isTrackingMode);
        int detectR = xjface->detect(img_data.data(), width, height, rotation,
                                     static_cast<XJFace::FaceImageType>(imageType), faceBoxInfoList,
                                     (isTrackingMode) ? true : false);
        //LOGI("detect result size = %d", faceBoxInfoList.size());
        if (faceBoxInfoList.size() > 0) {
            jobjectArray faceFaceBoxInfoArray = env->NewObjectArray(faceBoxInfoList.size(),clsRecogInfo, NULL);
            for (int i=0;i<faceBoxInfoList.size();i++) {
                const XJFace::FaceBoxInfo& faceBoxInfo = faceBoxInfoList[i];
                jobject objFaceBoxInfo = env->NewObject(clsRecogInfo, midconstructorRecogInfo);
                env->SetFloatField(objFaceBoxInfo, fidx, faceBoxInfo.rect.x * width);
                env->SetFloatField(objFaceBoxInfo, fidy, faceBoxInfo.rect.y * height);
                env->SetFloatField(objFaceBoxInfo, fidwidth, faceBoxInfo.rect.width * width);
                env->SetFloatField(objFaceBoxInfo, fidheight, faceBoxInfo.rect.height * height);
                env->SetFloatField(objFaceBoxInfo, fidconf, faceBoxInfo.sore);
                env->SetIntField(objFaceBoxInfo, fidtrackid, faceBoxInfo.trackId);
                jobject field_data = env->GetObjectField(objFaceBoxInfo, fidlandmarks);
                auto *fa = reinterpret_cast<jfloatArray *>(&field_data);
                float *fa_raw = env->GetFloatArrayElements(*fa, nullptr);
                int landmarkNum = faceBoxInfo.landmarks.size();
                for (int k = 0; k < landmarkNum; k++) {
                    fa_raw[k * 2] = faceBoxInfo.landmarks[k].x * width;
                    fa_raw[k * 2 + 1] = faceBoxInfo.landmarks[k].y * height;
                }
                env->SetObjectArrayElement(faceFaceBoxInfoArray, i, objFaceBoxInfo);
                env->DeleteLocalRef(objFaceBoxInfo);
                env->ReleaseFloatArrayElements(*fa, fa_raw, 1);
            }
            return faceFaceBoxInfoArray;
        }
        return nullptr;
    }
}

//setLogLevel
JNIEXPORT void JNICALL JNI_FUNCTION(
        setLogLevel(JNIEnv * env, jobject /* this */, jlong handle,jint level)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        xjface->setLogLevel(level);
    }
}

//getVersionName
JNIEXPORT jstring JNICALL JNI_FUNCTION(
        getVersionName(JNIEnv * env, jobject /* this */, jlong handle)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        string version = xjface->getVersionName();
        return env->NewStringUTF(version.c_str());
    }
    return env->NewStringUTF("");
}


//getDetectorModelVersion
JNIEXPORT jstring JNICALL JNI_FUNCTION(
        getDetectorModelVersion(JNIEnv * env, jobject /* this */, jlong handle)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        string version = xjface->getDetectorModelVersion();
        return env->NewStringUTF(version.c_str());
    }
    return env->NewStringUTF("");
}

//getRecognizerModelVersion
JNIEXPORT jstring JNICALL JNI_FUNCTION(
        getRecognizerModelVersion(JNIEnv * env, jobject /* this */, jlong handle)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        string version = xjface->getRecognizerModelVersion();
        return env->NewStringUTF(version.c_str());
    }
    return env->NewStringUTF("");
}


//getSpoofJudgerModelVersion
JNIEXPORT jstring JNICALL JNI_FUNCTION(
        getSpoofJudgerModelVersion(JNIEnv * env, jobject /* this */, jlong handle)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        string version = xjface->getSpoofJudgerModelVersion();
        return env->NewStringUTF(version.c_str());
    }
    return env->NewStringUTF("");
}


//extrackFeature
JNIEXPORT jfloatArray JNICALL JNI_FUNCTION(
        extrackFeatureByImage(JNIEnv * env, jobject /* this */,
                              jlong handle, jbyteArray img_bytes, jint width, jint height, jint rotation, jint image_type)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        LOGI("extrackFeatureByImage width=%d,height=%d,rotation=%d,imageType=%d", width,height, rotation, image_type);
        static vector<uint8_t> img_data;
        jsize size = env->GetArrayLength(img_bytes);
        if (img_data.size() != size) {
            img_data.resize(size);
        }
        env->GetByteArrayRegion(img_bytes, 0, size, (jbyte *) img_data.data());
        vector<float> features;
        int ret = xjface->extrackFeature(img_data.data(), width, height,rotation, static_cast<XJFace::FaceImageType>(image_type), features);
        if(ret != 0){
            LOGE("extrackFeatureByImage error!ret=%d", ret);
        }
        jfloatArray fArray = env->NewFloatArray(features.size());
        env->SetFloatArrayRegion(fArray, 0, features.size(), features.data());
        return fArray;
    }else{
        return nullptr;
    }
}

//extrackFeature
JNIEXPORT jfloatArray JNICALL JNI_FUNCTION(
        extrackFeatureByTrackId(JNIEnv * env, jobject /* this */,
                              jlong handle, jint trackId)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        LOGI("extrackFeatureByTrackId trackId=%d", trackId);
        vector<float> features;
        int ret = xjface->extrackFeature(trackId, features);
        if(ret != 0){
            LOGE("extrackFeatureByTrackId error!ret=%d", ret);
            return nullptr;
        }else {
            jfloatArray fArray = env->NewFloatArray(features.size());
            env->SetFloatArrayRegion(fArray, 0, features.size(), features.data());
            return fArray;
        }
    }else{
        return nullptr;
    }
}


//antiSpoofing
JNIEXPORT jfloat JNICALL JNI_FUNCTION(
        antiSpoofingByImage(JNIEnv * env, jobject /* this */,
                              jlong handle, jbyteArray img_bytes, jint width, jint height, jint rotation, jint image_type)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        LOGI("antiSpoofingByImage width=%d,height=%d,rotation=%d,imageType=%d", width,height, rotation, image_type);
        static vector<uint8_t> img_data;
        jsize size = env->GetArrayLength(img_bytes);
        if (img_data.size() != size) {
            img_data.resize(size);
        }
        env->GetByteArrayRegion(img_bytes, 0, size, (jbyte *) img_data.data());
        float score;
        int ret = xjface->antiSpoofing(img_data.data(), width, height,rotation, static_cast<XJFace::FaceImageType>(image_type), score);
        if(ret != 0){
            LOGE("antiSpoofingByImage error!ret=%d", ret);
        }
        return score;
    }else{
        return 0;
    }
}

//antiSpoofing
JNIEXPORT jfloat JNICALL JNI_FUNCTION(
        antiSpoofingByTrackId(JNIEnv * env, jobject /* this */,
                                jlong handle, jint trackId)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        LOGI("antiSpoofingByTrackId trackId=%d", trackId);
        float score = 0;
        int ret = xjface->antiSpoofing(trackId, score);
        if(ret != 0){
            LOGE("antiSpoofingByTrackId error!ret=%d", ret);
        }
        return score;
    }else{
        return 0;
    }
}


//faceCompare
JNIEXPORT jfloat JNICALL JNI_FUNCTION(
        faceCompare(JNIEnv * env, jobject /* this */,
                              jlong handle, jfloatArray feature1, jfloatArray feature2)) {
    if (handle) {
        XJFace *xjface = (XJFace *) handle;
        jsize size1 = env->GetArrayLength(feature1);
        jsize size2 = env->GetArrayLength(feature2);
        vector<float> feature_list1(size1);
        vector<float> feature_list2(size2);
        env->SetFloatArrayRegion(feature1,0,size1,(jfloat*)feature_list1.data());
        env->SetFloatArrayRegion(feature2,0,size2,(jfloat*)feature_list2.data());
        return xjface->faceCompare(feature_list1, feature_list2);
    }else{
        return 0;
    }
}

JNIEXPORT inline jfloat JNICALL JNI_FUNCTION(
        cutRight(
        JNIEnv* env, jobject /* this */,
        jbyteArray imagebytes, jint srcColumn, jint srcRow,
        jint dstColumn, jint dstRow,jint tailDeal)) {
    jbyte* dataPtr= env->GetByteArrayElements(imagebytes,0);
    jbyte* temPtrSrc = dataPtr + srcColumn;
    jbyte* temPtrDst = dataPtr + dstColumn;

    for (int i = 0; i < dstRow; ++i) {
        memcpy(temPtrDst, temPtrSrc, dstColumn);
        temPtrSrc+=srcColumn;
        temPtrDst+=dstColumn;
    }
    if(tailDeal&1==1){
        const int tailSize = srcColumn*srcRow-dstColumn*dstRow;
        memset(temPtrDst,0,sizeof(jbyte)*tailSize);
    }
    env->ReleaseByteArrayElements(imagebytes,dataPtr,0);
    return 0;
}

}