#include<iostream>

using namespace std;

extern "C" {  //包含C头文件
#include "libavutil/log.h" 
};
#pragma  comment(lib,"avutil.lib") //用到avutil库,需要连接
int main(int argc,char *argv[]) {
	av_log_set_level(AV_LOG_DEBUG); //设置日志级别

	av_log(NULL,AV_LOG_DEBUG,"hellog"); //打印日志

	getchar(); //窗口等待
	return 0;
}