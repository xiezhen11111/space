#include "ParticleSystemArray.h"

USING_NS_CC;
using namespace std;

ParticleSystemArray::ParticleSystemArray()
{

}

ParticleSystemArray::~ParticleSystemArray()
{

}

ParticleSystemArray* ParticleSystemArray::createWithFile(std::string file, int capacity, cocos2d::Node* parent)
{
	ParticleSystemArray *obj = new (std::nothrow) ParticleSystemArray();
	if (obj && obj->initWithFile(file, capacity, parent))
	{
		obj->autorelease();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}

bool ParticleSystemArray::initWithFile(std::string file, int capacity, cocos2d::Node* parent)
{
	if (!Node::init())
	{
		return false;
	}

	_nextItem = 0;
	_array.reserve(capacity); //���輯������

	for (int i=0; i<capacity; ++i)
	{
		ParticleSystemQuad *particleSystem = ParticleSystemQuad::create(file); //�����ļ�����һ������Ч��
		particleSystem->stopSystem();
		parent->addChild(particleSystem, 10); //������ϼ����������Ч��
		_array.pushBack(particleSystem);
	}

	return true;
}

ParticleSystemQuad* ParticleSystemArray::nextParticleSystem()
{
	ParticleSystemQuad* retval = _array.at(_nextItem);
	_nextItem++;

	if (_nextItem >= _array.size())
	{
		_nextItem = 0;
	}

	return retval;
}