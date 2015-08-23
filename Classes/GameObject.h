#ifndef _GameObject_
#define _GameObject_

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common.h"

typedef enum
{
	HealthBarTypeNone = 0,
	HealthBarTypeGreen, //��Ѫ��
	HealthBarTypeRed  //��Ѫ��
} HealthBarType;  //Ѫ������

class GameObject : public cocos2d::Sprite
{
public:
	GameObject();
	~GameObject();
	CC_SYNTHESIZE(float, _maxHp, MaxHp); //���Ѫ��

	static GameObject* createWithSpriteFrameName(std::string spriteFrameName, b2World* world, std::string shapeName, float maxHp, HealthBarType healthBarType);
	bool initWithSpriteFrameName(std::string spriteFrameName, b2World* world, std::string shapeName, float maxHp, HealthBarType healthBarType);

	bool dead();  //�Ƿ�����?
	void destroy(); //����
	void revive(); //����
	void takeHit(); //����

	void createBody(); //����һ��Box2D��������GameObject��
	void destroyBody();

	void setNodeInvisible(cocos2d::Node* sender);  //ʹ��㲻�ɼ�

	void setupHealthBar(); //����Ѫ��

	void update(float dt);
	void fadeOutDone();
protected:

	float _hp; //��ǰѪ��
	b2World* _world; //��������������
	b2Body* _body;  //�Լ�������
	std::string _shapeName;  //��״����

	HealthBarType _healthBarType;  //Ѫ������
	cocos2d::Sprite* _healthBarBg; //Ѫ������
	cocos2d::Sprite* _healthBarProgress;  //Ѫ��
	cocos2d::SpriteFrame* _healthBarProgressFrame;
	float _fullWidth;  //�ܿ��
	float _displayedWidth; //��ǰ��ʾ�Ŀ��
private:
};
#endif // !_GameObject_
