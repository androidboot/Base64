#include "Base64.h"

/**
* 获取选项值
*/
Flags *getFlags(int flag) {
    Flags *flags = (Flags *) malloc(sizeof(Flags));
    flags->isPadding = !(flag & NO_PADDING);
    flags->isWrap = !(flag & NO_WRAP);
    flags->isCRLF = (flag & CRLF);
    flags->isUrlSafe = (flag & URL_SAFE);
    return flags;
}

DecodeData *getErrorDecodeData(DecodeData *data) {
    data->length = DECODE_ERROR;
    return data;
}

/**
* 计算、获取编码长度
*/
int getEncodeLength(int realLength, int length, Flags *flags) {
//编码后的长度计算：3×8=4×6->4×8
    int tempLength = realLength;
    if (length != LEN_DEFAULT)
        tempLength = length;
    int encodeLength = 0;
    if (realLength > 0) {
        int remainder = tempLength % 3;
        encodeLength = (remainder == 0 ? ((tempLength / 3) << 2) : (((tempLength / 3) + 1) << 2));
        if (flags->isWrap) {
            if (!flags->isCRLF) //编码后每76个字符插入'\n'换行
                encodeLength = encodeLength + 1 + encodeLength / LINE_CHARS;
            else //编码后每76个字符插入'\r\n'换行
                encodeLength = encodeLength + (1 + encodeLength / LINE_CHARS) * 2;
        }
//没有padding，不需要在后面补'='
        if (!flags->isPadding && remainder > 0)
            encodeLength -= (3 - remainder);
    }
    return encodeLength;
}

EncodeData * androidboot::Base64::encode(const char *data, int realLength, int offset, int length, int flag) {
    EncodeData *encodeData = (EncodeData *) malloc(sizeof(EncodeData));
    Flags *flags = getFlags(flag);//根据选项值获取对应选项
    const char *table = flags->isUrlSafe ? ENCODE_WEB_TABLE : ENCODE_TABLE;
    char *p;//活动指针，用于循环取数据
    char temp1, temp2, temp3;//辅助取数
    int encodeLength = getEncodeLength(realLength, length, flags);
    encodeData->data = p = (char *) malloc(encodeLength * sizeof(char));
    int i;
    int count = LINE_GROUPS;
    int len = realLength;
    if (length != LEN_DEFAULT)
        len = length;
    len += offset;
    for (i = offset; i < len; i += 3) {
        bool isSecondCharEmpty = (i + 1 >= len);
        bool isThirdCharEmpty = (i + 2 >= len);
//每次增长3个，若能进入循环，第一个字节肯定存在
        temp1 = data[i];
        temp2 = isSecondCharEmpty ? 0 : data[i + 1];
        temp3 = isThirdCharEmpty ? 0 : data[i + 2];
//第一个转码取第一个字节的前6位，再通过0x3F(0011 1111)提取出
        *(p++) = table[(temp1 >> 2) & 0x3F];
//第二个转码取第一个字节的7、8位(0x3F)和第二个字节的前4位(0x0F)
        *(p++) = table[((temp1 << 4) & 0x3F) + ((temp2 >> 4) & 0x0F)];
//第三个转码取第二个字节的5、6、7、8位(0x3C)和第三个字节的前2位(0x03)
//假设只有一个字节，那么必定可以转换出第一、二个转码，所以只需要判断第二个字节是否为空
//若第二字节为空，则用‘=’（64）号代替
        if (isSecondCharEmpty && flags->isPadding)
            *(p++) = table[64];
        else if (!isSecondCharEmpty)
            *(p++) = table[(((temp2 << 2) & 0x3C) + ((temp3 >> 6) & 0x03))];
//第四个转码直接取第三个字节的后6位即可
        if (isThirdCharEmpty && flags->isPadding)
            *(p++) = table[64];
        else if (!isThirdCharEmpty)
            *(p++) = table[(temp3 & 0x3F)];
        if (flags->isWrap && (--count) == 0) {
            if (flags->isCRLF) *(p++) = '\r';
            *(p++) = '\n';
            count = LINE_GROUPS;
        }
    }
    if (flags->isWrap) {
        if (flags->isCRLF) *(p++) = '\r';
        *p = '\n';//最后要加结束符号
    }
    free(flags);
    encodeData->length = encodeLength;
    return encodeData;
}

DecodeData * androidboot::Base64::decode(const char *data, int realLength, int offset, int length, int flag) {
    DecodeData *decodeData = (DecodeData *) malloc(sizeof(DecodeData));
    const int *table;
    if (flag & URL_SAFE)
        table = DECODE_WEB_TABLE;
    else
        table = DECODE_TABLE;
    int value;
    char c;
    int index = offset;
    int len = realLength;
    if (length != LEN_DEFAULT)
        len = length;
    len += offset;
    decodeData->data = (char *) malloc(sizeof(char) * len);
    int decodeLength = 0;
    int state = 0;//记录状态
//最后还是用了android源码里的解决方法
//因为解码数据中可能存在多种被无效数据（如头文件里解码表中的-1、-2）污染的情况
//自己写过2种方法，效果不理想
    while (index < len) {
        if (state == 0) {
            while (index + 4 <= len && (value = (table[data[index]] << 18) |
                                                (table[data[index + 1]] << 12) |
                                                (table[data[index + 2]] << 6) |
                                                (table[data[index + 3]])) >= 0) {
                decodeData->data[decodeLength++] = (value >> 16);
                decodeData->data[decodeLength++] = (value >> 8);
                decodeData->data[decodeLength++] = value;
// //第一个解码取第一个字节的第2~8位，第二个字节的第3~4位
// decodeData->data[decodeLength++] = (temp1 << 2) + (temp2 >> 4);
// //第二个解码取第二个字节的后4位，第三个字节的第2~6位
// decodeData->data[decodeLength++] = (temp2 << 4) + (temp3 >> 2);
// //第三个解码取第三个字节的后2位，第四个字节的第0~6位
// decodeData->data[decodeLength++] = (temp3 << 6) + temp4;
                index += 4;
            }
            if (index >= len)
                break;
        }
        c = table[data[index++]];
        switch (state) {
            case 0:
                if (c < DECODE_EQUALS) {
                    value = c;
                    ++state;
                } else if (c != DECODE_SKIP)
                    return getErrorDecodeData(decodeData);
                break;
            case 1:
                if (c < DECODE_EQUALS) {
                    value = (value << 6) | c;
                    ++state;
                } else if (c != DECODE_SKIP)
                    return getErrorDecodeData(decodeData);
                break;
            case 2:
                if (c < DECODE_EQUALS) {
                    value = (value << 6) | c;
                    ++state;
                } else if (c == DECODE_EQUALS) {
                    decodeData->data[decodeLength++] = (value >> 4);
// //第一个解码取第一个字节的第2~8位，第二个字节的第3~4位
// decodeData->data[decodeLength++] = (temp1 << 2) + (temp2 >> 4);
                    state = 4;
                } else if (c != DECODE_SKIP)
                    return getErrorDecodeData(decodeData);
                break;
            case 3:
                if (c < DECODE_EQUALS) {
                    value = (value << 6) | c;
                    decodeData->data[decodeLength++] = (value >> 16);
                    decodeData->data[decodeLength++] = (value >> 8);
                    decodeData->data[decodeLength++] = value;
                    state = 0;
                } else if (c == DECODE_EQUALS) {
                    decodeData->data[decodeLength++] = (value >> 10);
                    decodeData->data[decodeLength++] = (value >> 2);
                    state = 5;
                } else if (c != DECODE_SKIP)
                    return getErrorDecodeData(decodeData);
                break;
            case 4:
                if (c == DECODE_EQUALS) {
                    ++state;
                } else if (c != DECODE_SKIP)
                    return getErrorDecodeData(decodeData);
                break;
            case 5:
                if (c != DECODE_SKIP)
                    return getErrorDecodeData(decodeData);
                break;
        }
    }
//跳出while循环后
    switch (state) {
        case 0:
            break;
        case 1:
            return getErrorDecodeData(decodeData);
        case 2:
            decodeData->data[decodeLength++] = (value >> 4);
            break;
        case 3:
            decodeData->data[decodeLength++] = (value >> 10);
            decodeData->data[decodeLength++] = (value >> 2);
            break;
        case 4:
            return getErrorDecodeData(decodeData);
        case 5:
            break;
    }
    decodeData->length = decodeLength;
    return decodeData;
}

