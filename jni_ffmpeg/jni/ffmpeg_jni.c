#include <stdio.h>
#include "ffmpeg_jni.h"
#include <android/log.h>



//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//重采样
#include "libswresample/swresample.h"


#define LOG_TAG "wangjf"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

char* Jstring2CStr(JNIEnv* env, jstring jstr) {
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env, "java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env, "GB2312");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes",
			"(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray)(*env)->CallObjectMethod(env, jstr, mid,
			strencode); // String .getByte("GB2312");
	jsize alen = (*env)->GetArrayLength(env, barr);
	jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char*) malloc(alen + 1); //"\0"
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	(*env)->ReleaseByteArrayElements(env, barr, ba, 0); //
	return rtn;
}

JNIEXPORT void JNICALL Java_com_research_amr2mp3_FFmpegUtil_jniRun
  (JNIEnv * env, jclass cls, 
  jstring jinput, jstring joutput){
	char* input = Jstring2CStr(env,jinput) ;
	char* output = Jstring2CStr(env,joutput);

	av_register_all();
        AVFormatContext *pFormatCtx = avformat_alloc_context();
        //打开音频文件
		int resultint = avformat_open_input(&pFormatCtx, input, NULL, NULL);
        if (resultint != 0) {
            LOGI("%s", "open avformat fail");
			LOGE(" resultint  %d", resultint);
            return;
        }
        //获取输入文件信息
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
            LOGI("%s", "open stream info fail");
            return;
        }
        //获取音频流索引位置
        int i = 0, audio_stream_idx = -1;
        for (; i < pFormatCtx->nb_streams; i++) {
            if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                audio_stream_idx = i;
                break;
            }
        }
        //获取解码器
        AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
        AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
        //打开解码器
        if (avcodec_open2(codecCtx, codec, NULL) < 0) {
            LOGI("%s", "open avcodec fial");
            return;
        }
        //压缩数据
        AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
        //解压缩数据
        AVFrame *frame = av_frame_alloc();
        //frame->16bit 44100 PCM 统一音频采样格式与采样率
        SwrContext *swrContext = swr_alloc();
        //音频格式  重采样设置参数
        const enum AVSampleFormat in_sample = codecCtx->sample_fmt;//原音频的采样位数
        //输出采样格式
        const enum AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;//16位
        int in_sample_rate = codecCtx->sample_rate;// 输入采样率
        int out_sample_rate = 16000;//输出采样
    
        //输入声道布局
        uint64_t in_ch_layout = codecCtx->channel_layout;
        //输出声道布局
        uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;//2通道 立体声 AV_CH_LAYOUT_STEREO  AV_CH_LAYOUT_MONO
    
        /**
         * struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
          int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
          int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
          int log_offset, void *log_ctx);
         */
        swr_alloc_set_opts(swrContext, out_ch_layout, out_sample, out_sample_rate, in_ch_layout, in_sample,
                           in_sample_rate, 0, NULL);
        swr_init(swrContext);
        int got_frame = 0;
        int ret;
        int out_channerl_nb = av_get_channel_layout_nb_channels(out_ch_layout);
        LOGE("out_channerl_nb %d ", out_channerl_nb);
        int count = 0;
        //设置音频缓冲区间 16bit   44100  PCM数据
        uint8_t *out_buffer = (uint8_t *) av_malloc(2 * 44100);
        FILE *fp_pcm = fopen(output, "wb");//输出到文件
		int aaa = 1;
        while (av_read_frame(pFormatCtx, packet) >= 0) {
    
            ret = avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);
            //LOGE("decode ing %d", count++);
            if (ret < 0) {
                LOGE("decode finish");
            }
            //解码一帧
            if (got_frame > 0) {
                /**
                 * int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
                                    const uint8_t **in , int in_count);
                 */
                swr_convert(swrContext, &out_buffer, 2 * 44100,
                            (const uint8_t **) frame->data, frame->nb_samples);
                /**
                 * int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                                   enum AVSampleFormat sample_fmt, int align);
                 */
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channerl_nb, frame->nb_samples,
                                                                 out_sample, 1);
				
				//LOGE("out_buffer_size %hu\n", out_buffer_size);												 
                fwrite(out_buffer, 1, out_buffer_size, fp_pcm);//输出到文件
				
            }
        }
        fclose(fp_pcm);
        av_frame_free(&frame);
        av_free(out_buffer);
        swr_free(&swrContext);
        avcodec_close(codecCtx);
        avformat_close_input(&pFormatCtx);
}



