#include "SpriteArray.h"
USING_NS_CC;
using namespace std;

SpriteArray::SpriteArray()
{
	_array = nullptr;
}

SpriteArray::~SpriteArray()
{

}

SpriteArray* SpriteArray::createWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode,b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType)
{
	SpriteArray* array = new (std::nothrow) SpriteArray();
	if (array && array->initWithCapacity(capacity, spriteFrameName, batchNode, world, shapeName, maxHp, healthBarType))
	{
		array->autorelease();
		return array;
	}
	CC_SAFE_DELETE(array);
	return nullptr;
}

bool SpriteArray::initWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode,b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType)
{
	if (!Node::init())
	{
		return false;
	}
	_nextItem = 0;
	_array = Array::createWithCapacity(capacity);
	for (int i=0; i<capacity; i++)
	{
		GameObject* sprite = GameObject::createWithSpriteFrameName(spriteFrameName, world, shapeName, maxHp, healthBarType);
		sprite->setVisible(false);
		batchNode->addChild(sprite);
		_array->addObject(sprite);
	}
	_array->retain();
	return true;
}

SpriteArray* SpriteArray::createWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode,b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType, int zOrder)
{
	SpriteArray* array = new (std::nothrow) SpriteArray();
	if (array && array->initWithCapacity(capacity, spriteFrameName, batchNode, world, shapeName, maxHp, healthBarType, zOrder))
	{
		array->autorelease();
		return array;
	}
	CC_SAFE_DELETE(array);
	return nullptr;
}

bool SpriteArray::initWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode,b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType, int zOrder)
{
	if (!Node::init())
	{
		return false;
	}
	_nextItem = 0;
	_array = Array::createWithCapacity(capacity);
	for (int i=0; i<capacity; i++)
	{
		GameObject* sprite = GameObject::createWithSpriteFrameName(spriteFrameName, world, shapeName, maxHp, healthBarType);
		sprite->setVisible(false);
		batchNode->addChild(sprite, zOrder);
		_array->addObject(sprite);
	}
	_array->retain();
	return true;
}

void SpriteArray::onExit()
{
	CC_SAFE_RELEASE_NULL(_array);
	Node::onExit();
}
GameObject* SpriteArray::nextSprite()
{
	//取出_nextItem指向的Sprite
	GameObject* sprite = dynamic_cast<GameObject*>(_array->objectAtIndex(_nextItem));
	_nextItem++;
	if (_nextItem >= _array->count())
	{
		_nextItem = 0;
	}
	return sprite;
}