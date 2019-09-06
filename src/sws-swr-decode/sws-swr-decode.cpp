#include<iostream>

using namespace std;

extern "C" {
#include "libavutil/log.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
};
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"swresample.lib")

int main(int argc,char *argv[]) {
	av_log_set_level(AV_LOG_DEBUG);
	av_log(NULL,AV_LOG_DEBUG,"hello");
	const char* p = "G:/123/123.mp4";
	AVFormatContext* fmt_ctx = NULL;
	avformat_open_input(&fmt_ctx,p,NULL,NULL);
	avformat_find_stream_info(fmt_ctx,NULL);
	av_dump_format(fmt_ctx,0,p,0);
	int videoStream = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
	int audioStream = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);

	//视频
	AVCodec* vcode = avcodec_find_decoder(fmt_ctx->streams[videoStream]->codecpar->codec_id);
	AVCodecContext* vc = avcodec_alloc_context3(vcode);

	avcodec_parameters_to_context(vc,fmt_ctx->streams[videoStream]->codecpar);

	vc->thread_count = 8;

	//音频
	AVCodec* acode = avcodec_find_decoder(fmt_ctx->streams[audioStream]->codecpar->codec_id);

	AVCodecContext* ac = avcodec_alloc_context3(acode);

	avcodec_parameters_to_context(ac,fmt_ctx->streams[audioStream]->codecpar);

	ac->thread_count = 8;

	AVPacket* pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	//像素格式和尺寸转换上下文
	SwsContext *sws = NULL;

	unsigned char* rgb = NULL;

	unsigned char* pcm = NULL;
	//音频重采样 上下文初始化
	SwrContext* swr = swr_alloc();
	swr = swr_alloc_set_opts(
		swr,
		av_get_default_channel_layout(2),
		AV_SAMPLE_FMT_S16,
		ac->sample_rate,
		av_get_default_channel_layout(ac->channels),
		ac->sample_fmt,
		ac->sample_rate,
		0,
		0
	);
	//初始化音频上下文
	swr_init(swr);
	//打开解码器
	avcodec_open2(vc,0,0);
	avcodec_open2(ac,0,0);
	for (;;) {
		int re = av_read_frame(fmt_ctx,pkt);
		if (re != 0) {
			break;
		}
		AVCodecContext* cc = 0;
		if (pkt->stream_index == videoStream) {
			//视频
			cc = vc;
		}
		if (pkt->stream_index == audioStream) {
			//音频
			cc = ac;
		}
		//发送到解码线程
		avcodec_send_packet(cc,pkt);
		//释放,引用数减1，为0后自动释放空间
		av_packet_unref(pkt);
		//解码有缓冲,所以要循环接收
		for (;;) {
			re=avcodec_receive_frame(cc,frame);
			if (re != 0) break;
			if (cc==vc) {
				sws = sws_getCachedContext(
				    sws,
					frame->width,frame->height,(AVPixelFormat)frame->format,
					frame->width,frame->height,AV_PIX_FMT_RGBA,
					SWS_BILINEAR,
					0,0,0);
				if (sws) {
					if (!rgb) rgb = new unsigned char[frame->width * frame->height * 4];
					uint8_t* data[2] = {0};
					data[0] = rgb;
					int lines[2] = {0};
					lines[0] = frame->width * 4;
					re = sws_scale(sws,frame->data,frame->linesize,0,frame->height,data,lines);
					//高度
					cout << "sws_scale:----------------" << re << endl;
				
				}
			}
			else {

				if (!pcm) pcm = new unsigned char[frame->nb_samples * 2*2];
				uint8_t* data[2] = {0};
				data[0] = pcm;

				re = swr_convert(swr,data,frame->nb_samples,(const uint8_t **)frame->data,frame->nb_samples);
				//单通道采样数量
				cout << "swr_convert=============:" << re << endl;
			
			}
		}

	
	}
	
	av_frame_free(&frame);
	avformat_close_input(&fmt_ctx);

	return 0;
}