/*************
  粒子系统模拟
**************/
#ifndef _ParticleSystemArray_
#define _ParticleSystemArray_

#include "cocos2d.h"

class ParticleSystemArray : public cocos2d::Node
{
public:
	ParticleSystemArray();
	~ParticleSystemArray();
	
	static ParticleSystemArray* createWithFile(std::string file, int capacity, cocos2d::Node* parent);
	bool initWithFile(std::string file, int capacity, cocos2d::Node* parent);

	cocos2d::ParticleSystemQuad* nextParticleSystem();  //获取下一个粒子效果

	cocos2d::Vector<cocos2d::ParticleSystemQuad*> getArray() {return _array;}

protected:
	cocos2d::Vector<cocos2d::ParticleSystemQuad*> _array;  //粒子系统集合
	int _nextItem;  //下一个粒子效果索引
private:
};
#endif