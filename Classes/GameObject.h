#ifndef _GameObject_
#define _GameObject_

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common.h"

typedef enum
{
	HealthBarTypeNone = 0,
	HealthBarTypeGreen, //绿血条
	HealthBarTypeRed  //红血条
} HealthBarType;  //血条种类

class GameObject : public cocos2d::Sprite
{
public:
	GameObject();
	~GameObject();
	CC_SYNTHESIZE(float, _maxHp, MaxHp); //最大血量

	static GameObject* createWithSpriteFrameName(std::string spriteFrameName, b2World* world, std::string shapeName, float maxHp, HealthBarType healthBarType);
	bool initWithSpriteFrameName(std::string spriteFrameName, b2World* world, std::string shapeName, float maxHp, HealthBarType healthBarType);

	bool dead();  //是否死亡?
	void destroy(); //毁灭
	void revive(); //复活
	void takeHit(); //受伤

	void createBody(); //创建一个Box2D的物体在GameObject上
	void destroyBody();

	void setNodeInvisible(cocos2d::Node* sender);  //使结点不可见

	void setupHealthBar(); //设置血条

	void update(float dt);
	void fadeOutDone();
protected:

	float _hp; //当前血量
	b2World* _world; //所处的物理世界
	b2Body* _body;  //自己的身体
	std::string _shapeName;  //形状名字

	HealthBarType _healthBarType;  //血条类型
	cocos2d::Sprite* _healthBarBg; //血条背景
	cocos2d::Sprite* _healthBarProgress;  //血条
	cocos2d::SpriteFrame* _healthBarProgressFrame;
	float _fullWidth;  //总宽度
	float _displayedWidth; //当前显示的宽度
private:
};
#endif // !_GameObject_
