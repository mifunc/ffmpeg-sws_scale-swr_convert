#include<iostream>
using namespace std;


extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/log.h"
#include "libavcodec/avcodec.h"
};

#pragma  comment(lib,"avformat.lib")
#pragma  comment(lib,"avutil.lib")
#pragma  comment(lib,"avcodec.lib")


static double r2d(AVRational r) {
	return r.den == 0 ? 0 : double(r.num) / double(r.den);
}
int main(int argc,char *argv[]){
	av_log_set_level(AV_LOG_DEBUG); //设置日志级别
	av_log(NULL,AV_LOG_DEBUG,"hello");

	AVFormatContext* fmt_ctx = NULL;

	const char* p = "G:/123/123.mp4";

	avformat_open_input(&fmt_ctx,p,NULL,NULL); //打开视频文件

	avformat_find_stream_info(fmt_ctx,NULL);  

	av_dump_format(fmt_ctx,0,p,0); //打印媒体信息
	
	int videoStream = 0;
	int audioStream = 0;

	for (int i = 0;i < fmt_ctx->nb_streams;i++) {
		AVStream* as = fmt_ctx->streams[i];
		cout << "codec_id" <<as->codecpar->codec_id  << endl; //编码格式
		cout << "format" << as->codecpar->format << endl;     //像素格式

		if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) { //视频
			cout << "height:" << as->codecpar->width << endl;
			cout << "width:" << as->codecpar->height << endl;
			cout << "FPS:" << r2d(as->avg_frame_rate) << endl;
		}
		else if (as->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {//音频

			cout << "channels:" << as->codecpar->channels << endl;
			cout << "frame_zise:" << as->codecpar->frame_size << endl;
			cout << "sample_rate:" << as->codecpar->sample_rate << endl;
		}
	}

	videoStream = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);

	AVPacket* pkt = av_packet_alloc();
	for (;;) {
		int re = av_read_frame(fmt_ctx,pkt);
		if (re != 0) {
			//跳到3秒处播放
			int ms = 3000;
			long long pos = (double)ms / (double)1000 * (r2d(fmt_ctx->streams[pkt->stream_index]->time_base));
			av_seek_frame(fmt_ctx,videoStream,pos,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME);
			continue;
		}
		cout << "pts:" << pkt->pts << endl;
		//pts转换成时间戳(秒)
		cout << "pts s:" <<(pkt->pts/1000)*(r2d(fmt_ctx->streams[pkt->stream_index]->time_base)) <<endl;
		
		cout << "dts:" << pkt->dts << endl;
		//dts转换成时间戳(秒)
		cout << "dts s:" <<(pkt->dts/1000)*(r2d(fmt_ctx->streams[pkt->stream_index]->time_base)) <<endl;

		//释放,引用计数减1,为0释放空间
		av_packet_unref(pkt);
	}
	av_packet_free(&pkt);
	avformat_close_input(&fmt_ctx);


	return 0;
}
