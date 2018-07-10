package com.research.amr2mp3;

/**
 * 2018/5/22
 *
 * @author wangjf
 */

public class LameUtil {
    public static int run(String wav,String mp3){
        return jniConvertmp3(wav, mp3, 16000,-1,2,-1,5,1);

    }

    /**
     * @param wavPath wav路径
     * @param mp3Path MP3 路径
     * @param inSamplerate 采样率 不设置传-1
     * @param outSamplerate 采样率 不设置传-1
     * @param numChannels 文件的声道数 不设置传-1
     * @param brate 比特率 不设置传-1
     * @param quality 0-9  2=high  5 = medium  7=low
     * @param vbrModel  0 = vbr_default  1 = vbr_off  2 = vbr_abr  3 = vbr_mtrh
     *
     * 可参考 https://blog.csdn.net/xjwangliang/article/details/7065985
     * @return
     */
    static native int jniConvertmp3(String wavPath,String mp3Path,int inSamplerate, int outSamplerate, int numChannels, int brate, int quality, int vbrModel);
    static{
        System.loadLibrary("lame");
    }
}
