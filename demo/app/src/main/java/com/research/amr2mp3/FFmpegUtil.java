package com.research.amr2mp3;

/**
 * 2018/5/22
 *
 * @author wangjf
 */

public class FFmpegUtil {

    public static int run(String wavPath,String mp3Path){
        return jniRun(wavPath,mp3Path);
    }
    static native int jniRun(String wavPath,String mp3Path);
    static{
        System.loadLibrary("avcodec");
        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("ffmpeg");
        System.loadLibrary("swresample");
    }
}
