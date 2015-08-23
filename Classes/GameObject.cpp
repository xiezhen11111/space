#include "GameObject.h"
#include "ShapeCache.h"
USING_NS_CC;
using namespace std;

GameObject::GameObject()
{
	_world = nullptr;
	_body = nullptr;
}

GameObject::~GameObject()
{

}

GameObject* GameObject::createWithSpriteFrameName(std::string spriteFrameName, b2World* world, std::string shapeName, float maxHp, HealthBarType healthBarType)
{
	GameObject *obj = new (std::nothrow) GameObject();
	if (obj  && obj->initWithSpriteFrameName(spriteFrameName, world, shapeName, maxHp, healthBarType))
	{
		obj->autorelease();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}

bool GameObject::initWithSpriteFrameName(std::string spriteFrameName, b2World* world, std::string shapeName, float maxHp, HealthBarType healthBarType)
{
	if(!Sprite::initWithSpriteFrameName(spriteFrameName))
		return false;

	_hp = maxHp;
	_maxHp = maxHp;
	_world = world;
	_shapeName = shapeName;
	_healthBarType = healthBarType;
	_healthBarBg = nullptr;
	_healthBarProgress = nullptr;
	_healthBarProgressFrame = nullptr;

	this->setupHealthBar();
	this->scheduleUpdate();

	return true;
}

void GameObject::destroyBody()
{
	if (_body!=NULL)
	{
		_world->DestroyBody(_body);
		_body = NULL;
	}
}

void GameObject::createBody()
{
	this->destroyBody();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody; //�����Ƕ�̬����
	bodyDef.position.Set(this->getPositionX()/PTM_RATIO,
						 this->getPositionY()/PTM_RATIO);
	bodyDef.userData = this; //������û����������������
	_body = _world->CreateBody(&bodyDef);  //��������

	ShapeCache::getInstance()->addFixturesToBody(_body, _shapeName, this->getScale());  //�����ṩ����״�����������������״�Ͳ���
	Vec2 anchorP = ShapeCache::getInstance()->anchorPointForShape(_shapeName); //��ȡê��
	this->setAnchorPoint(anchorP);

}

void GameObject::setNodeInvisible(Node* sender)
{
	sender->setPosition(Point::ZERO);
	sender->setVisible(false);
	this->destroyBody();
}

void GameObject::revive()
{
	_hp = _maxHp;
	_displayedWidth = _fullWidth;
	if(_healthBarBg)
		_healthBarBg->setVisible(false);
	this->stopAllActions();
	this->setVisible(true);
	this->setOpacity(255);
	this->createBody();
}

bool GameObject::dead()
{
	return (_hp<=0);
}

void GameObject::takeHit()
{
	if (_hp > 0)
	{
		_hp--;
	}
	if (_hp == 0)
	{
		this->destroy();
	}
}

void GameObject::destroy()
{
	_hp = 0;
	this->stopAllActions();
	//���߻�ʱ�ȵ�������ʧ
	this->runAction(Sequence::create(FadeOut::create(0.1f), 
		CallFuncN::create(CC_CALLBACK_1(GameObject::setNodeInvisible, this)),
		NULL));
}

void GameObject::setupHealthBar()
{
	if(_healthBarType == HealthBarTypeNone) return;

	_healthBarBg = Sprite::createWithSpriteFrameName("healthbar_bg.png");
	_healthBarBg->setPosition(this->getPosition() + Vec2(this->getContentSize().width/2, -_healthBarBg->getContentSize().height));
	this->addChild(_healthBarBg);

	std::string progressSpriteName = "";
	if (_healthBarType == HealthBarTypeGreen)
	{
		progressSpriteName = "healthbar_green.png";
	}
	else
	{
		progressSpriteName = "healthbar_red.png";
	}
	_healthBarProgressFrame = SpriteFrameCache::getInstance()->spriteFrameByName(progressSpriteName);
	_healthBarProgressFrame->retain();
	_healthBarProgress = Sprite::createWithSpriteFrameName(progressSpriteName);
	_healthBarProgress->setPosition(Vec2(_healthBarProgress->getContentSize().width/2,
										 _healthBarProgress->getContentSize().height/2));
	_fullWidth = _healthBarProgress->getTextureRect().size.width;
	_healthBarBg->addChild(_healthBarProgress);
}

void GameObject::update(float dt)
{
	if(_healthBarType == HealthBarTypeNone)  return;

	float POINTS_PER_SEC = 50;

	float percentage = _hp / _maxHp;  //�����ǰѪ������Ѫ������
	percentage = MIN(percentage, 1.0);
	percentage = MAX(percentage, 0);
	float desiredWidth = _fullWidth * percentage; //������Ѫ�����

	if (desiredWidth < _displayedWidth)//�����ʾ��Ѫ����������Ѫ����
	{
		_displayedWidth = MAX(desiredWidth, _displayedWidth - POINTS_PER_SEC*dt); //��ʾ��Ѫ��ÿ��ѭ����ȥPOINTS_PER_SEC��ֱ����������Ѫ�����
	}
	else
	{
		_displayedWidth = MIN(desiredWidth, _displayedWidth + POINTS_PER_SEC*dt);
	}

	Rect oldTextureRect = _healthBarProgressFrame->getRect();
	Rect newTextureRect = Rect(oldTextureRect.origin.x, oldTextureRect.origin.y,
							   _displayedWidth, oldTextureRect.size.height);

	_healthBarProgress->setTextureRect(newTextureRect, _healthBarProgressFrame->isRotated(), _healthBarProgressFrame->getOriginalSize());

	_healthBarProgress->setPosition(Vec2(_displayedWidth/2, _healthBarProgress->getContentSize().height/2));

	//ֻ��Ѫ���仯ʱ����Ѫ������
	if (desiredWidth!=_displayedWidth)
	{
		_healthBarBg->setVisible(true);
		_healthBarBg->stopAllActions();
		_healthBarBg->runAction(     //��Ѫ���ȳ�������ʧ
			Sequence::create(
				FadeTo::create(0.25f, 255), DelayTime::create(2.f),
				FadeTo::create(0.25, 0),
				CallFunc::create(CC_CALLBACK_0(GameObject::fadeOutDone, this)), NULL
			)
			);
		//_healthBarBg�ĺ��Ӳ�����Ÿ���һ���������Ժ���Ҫ��������͸����
		_healthBarProgress->stopAllActions();
		_healthBarProgress->runAction(     //��Ѫ���ȳ�������ʧ
			Sequence::create(
			FadeTo::create(0.25f, 255), DelayTime::create(2.f),
			FadeTo::create(0.25, 0),
			 NULL
			)
			);
	}
}

void GameObject::fadeOutDone()
{
	_healthBarBg->setVisible(false);
}