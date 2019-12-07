#include <string>
#include <jni.h>
#include "Base64.h"
#include <android/log.h>

#define LOGGING_TAG "Base64"
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOGGING_TAG, __VA_ARGS__)

using namespace androidboot;

Base64 base64Handler;


/**
 * 判断字节数组是否为空
 * @param env jni环境
 * @param byteArray 字节数组
 * @return 判断结果
 */
bool isByteArrayNull(JNIEnv *env, jbyteArray byteArray) {
    if (byteArray == nullptr) {
        jclass npe = env->FindClass("java/lang/NullPointerException");
        env->ThrowNew(npe, "byte[] is null!");
        return true;
    }
    return false;
}

/**
 * 判断参数是否异常
 * @param env jni环境
 * @param realLength 字节数组长度
 * @param offset 偏移量
 * @param length
 * @return 判断结果
 */
bool isOutOfRange(JNIEnv *env, int realLength, int offset, int length) {
    if (offset < 0) {
        jclass lae = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(lae, "offset was a minus!");
        return true;
    }
    if (length < 0 && LEN_DEFAULT != length) {
        jclass lae = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(lae, "length was a minus!");
        return true;
    }
    if (length > realLength) {
        jclass lae = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(lae, "array length is less than length!");
        return true;
    }
    if (offset > realLength) {
        jclass lae = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(lae, "offset is more than array length!");
        return true;
    }
    if ((offset + length) > realLength && LEN_DEFAULT != length) {
        jclass lae = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(lae, "offset and length is more than array length!");
        return true;
    }
    return false;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_androidboot_base64_AndroidBase64_nativeEncode(JNIEnv *env, jclass clazz,
                                                       jbyteArray byteArray, jint offset,
                                                       jint length, jint flag) {
    if (isByteArrayNull(env, byteArray))
        return nullptr;
    jbyte *data = env->GetByteArrayElements(byteArray, JNI_FALSE);
    jint realLen = env->GetArrayLength(byteArray);
    if (isOutOfRange(env, realLen, offset, length)) {
        return nullptr;
    }
    EncodeData *encodeData = base64Handler.encode((char *) data, realLen, offset, length, flag);
    realLen = encodeData->length;
    env->ReleaseByteArrayElements(byteArray, data, 0);
    jbyteArray result = env->NewByteArray(realLen);
    env->SetByteArrayRegion(result, 0, realLen, (const jbyte *) encodeData->data);
    free(encodeData->data);
    free(encodeData);
    jclass strClass = env->FindClass("java/lang/String");
    jmethodID strInitId = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring str = env->NewStringUTF("utf-8");
    return (jstring) env->NewObject(strClass, strInitId, result, str);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_androidboot_base64_AndroidBase64_nativeDecode(JNIEnv *env, jclass clazz,
                                                       jbyteArray byteArray, jint offset,
                                                       jint length, jint flag) {
    if (isByteArrayNull(env, byteArray))
        return nullptr;
    jbyte *data = env->GetByteArrayElements(byteArray, JNI_FALSE);
    jint realLen = env->GetArrayLength(byteArray);
    if (isOutOfRange(env, realLen, offset, length)) {
        return nullptr;
    }
    DecodeData *decodeData = base64Handler.decode((const char *) data, realLen, offset, length,
                                                  flag);
    if (decodeData->length == -1) {
        jclass lae = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(lae, "Data can't be decoded!");
        return nullptr;
    }
    realLen = decodeData->length;
    env->ReleaseByteArrayElements(byteArray, data, 0);
    jbyteArray result = env->NewByteArray(realLen);
    env->SetByteArrayRegion(result, 0, realLen, (const jbyte *) decodeData->data);
    free(decodeData->data);
    free(decodeData);
    return result;
}