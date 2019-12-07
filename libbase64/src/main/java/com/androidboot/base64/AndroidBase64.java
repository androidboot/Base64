package com.androidboot.base64;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import java.io.ByteArrayOutputStream;

public class AndroidBase64 {

    /**
     * 默认（76字换行、结尾换行、结尾字符不足补'='、换行符不带\r，标准编码库）
     */
    public static final int DEFAULT = 0;
    /**
     * 结尾不带'='，默认带
     */
    public static final int NO_PADDING = 1;
    /**
     * 没有换行，默认有
     */
    public static final int NO_WRAP = 2;
    /**
     * 换行风格是CRLF还是LF，默认LF
     */
    public static final int CRLF = 4;
    /**
     * 使用url可使用的格式，即'_'换成'-'，'+'、'/'换成'_'
     */
    public static final int URL_SAFE = 8;

    private static final int LEN_DEFAULT = Integer.MIN_VALUE;

    static {
        System.loadLibrary("base64");
    }

    /**
     * base64字符串转图片
     *
     * @param base64 base64字符串
     * @return 图片
     */
    public static Bitmap base64ToBitmap(String base64) {
        if (base64 == null) {
            return null;
        }
        byte[] bitmapData = nativeDecode(base64.getBytes(), 0, LEN_DEFAULT, DEFAULT);
        return BitmapFactory.decodeByteArray(bitmapData, 0, bitmapData.length);
    }

    /**
     * 图片转base64字符串
     *
     * @param bitmap 图片bitmap
     * @return base64字符串
     */
    public static String bitmapToBase64(Bitmap bitmap) {
        return bitmapToBase64(bitmap, Bitmap.CompressFormat.PNG);
    }

    /**
     * 图片转base64字符串
     *
     * @param bitmap         图片bitmap
     * @param compressFormat 图片格式
     * @return base64字符串
     */
    public static String bitmapToBase64(Bitmap bitmap, Bitmap.CompressFormat compressFormat) {
        if (bitmap == null) {
            return null;
        }
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        bitmap.compress(compressFormat, 100, byteArrayOutputStream);
        return nativeEncode(byteArrayOutputStream.toByteArray(), 0, LEN_DEFAULT, DEFAULT);
    }

    /**
     * 编码
     *
     * @param byteArray 需要编码字节数组
     * @param offset    要编码的第一个 byte 的索引
     * @param length    要编码的 byte 数 的长度
     * @param flag      选项
     * @return 编码结果
     */
    public static native String nativeEncode(byte[] byteArray, int offset, int length, int flag);


    /**
     * 解码
     *
     * @param data   需要解码字节数组
     * @param offset 要解码的第一个 byte 的索引
     * @param length 要解码的 byte 数 的长度
     * @param flag   选项
     * @return 解码结果
     */
    public static native byte[] nativeDecode(byte[] data, int offset, int length, int flag);
}
