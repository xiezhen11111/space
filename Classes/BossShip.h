/*******************
 Boss∑…¥¨¿‡
********************/
#ifndef _BossShip_
#define _BossShip_

class ActionLayer;

#include "cocos2d.h"
#include "GameObject.h"

class BossShip : public GameObject
{
public:
	BossShip();
	~BossShip();

	static BossShip* createWithWorld(b2World* world, ActionLayer* layer);
	bool initWithWorld(b2World* world, ActionLayer* layer);

	void updateWithShipPosition(cocos2d::Vec2 shipPosition);

	void randomAction();

	virtual void revive();

public:
	ActionLayer* _layer;
	bool _initialMove;

	cocos2d::Sprite* _shooter1;
	cocos2d::Sprite* _shooter2;
	cocos2d::Sprite* _cannon;
private:
};
#endif