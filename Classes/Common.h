#ifndef _Common_
#define _Common_

#include "cocos2d.h"

#define PTM_RATIO 50  //�����ֵ���Գ����ģ�Խ���Χ�о�Խ��ԽС��Χ��ԽС
class Common
{
public:
	//������low��high֮��������
	static float randomValueBetween(float low, float high);
	//��ȡ��ǰϵͳ���뼶ʱ��
	static double getMillSecond();
	//�������������
	static unsigned long getRandSeed();
protected:
private:
};
#endif // !_Common_
