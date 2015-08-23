#ifndef _SpriteArray_
#define _SpriteArray_

#include "cocos2d.h"
#include "GameObject.h"

class SpriteArray : public cocos2d::Node
{
public:
	SpriteArray();
	~SpriteArray();

	virtual void onExit();
	
	//不带zorder的版本
	static SpriteArray* createWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode, b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType);
	bool initWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode, b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType);

	//带zOrder的版本
	static SpriteArray* createWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode, b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType,int zOrder);
	bool initWithCapacity(int capacity, std::string spriteFrameName, cocos2d::SpriteBatchNode* batchNode, b2World *world, std::string shapeName, int maxHp, HealthBarType healthBarType, int zOrder);
	GameObject* nextSprite();

	cocos2d::Array* getArray() {return _array;}
protected:
	cocos2d::Array* _array;
	int _nextItem;
private:
};
#endif // !_SpriteArray_
