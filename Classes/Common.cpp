#include "Common.h"

USING_NS_CC;

const unsigned long RAND_SCALE = 100007;

unsigned long Common::getRandSeed()
{
	auto director = cocos2d::Director::getInstance();
	auto ps = director->getSecondsPerFrame();
	auto tf = director->getTotalFrames();
	auto ret = (unsigned long)(ps*tf*RAND_SCALE);  
	return ret;
}

float Common::randomValueBetween(float low, float high)
{
	/*struct timeval now;
	gettimeofday(&now, NULL); //计算时间种子
	unsigned rand_seed = (unsigned)(now.tv_sec*1000 + now.tv_usec/1000);*/
	unsigned long rand_seed = getRandSeed();
	srand(rand_seed);

	float result = CCRANDOM_0_1() * high + low;
	return result;

	
}

double Common::getMillSecond()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);  //获取当前系统时间

	//log("CurrentTime MillSecond: %f", (double)tv.tv_sec * 1000 + (double)tv.tv_usec/1000);

	return (double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000;
}