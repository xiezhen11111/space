#ifndef _Common_
#define _Common_

#include "cocos2d.h"

#define PTM_RATIO 50  //这个数值是试出来的，越大包围盒就越大，越小包围盒越小
class Common
{
public:
	//返回在low和high之间的随机数
	static float randomValueBetween(float low, float high);
	//获取当前系统毫秒级时间
	static double getMillSecond();
	//产生随机数种子
	static unsigned long getRandSeed();
protected:
private:
};
#endif // !_Common_
