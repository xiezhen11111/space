/*************
  ����ϵͳģ��
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

	cocos2d::ParticleSystemQuad* nextParticleSystem();  //��ȡ��һ������Ч��

	cocos2d::Vector<cocos2d::ParticleSystemQuad*> getArray() {return _array;}

protected:
	cocos2d::Vector<cocos2d::ParticleSystemQuad*> _array;  //����ϵͳ����
	int _nextItem;  //��һ������Ч������
private:
};
#endif