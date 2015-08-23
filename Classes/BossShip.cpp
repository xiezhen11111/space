#include "BossShip.h"
#include "ActionLayer.h"

USING_NS_CC;
using namespace std;

BossShip::BossShip()
{
	_shooter1 = nullptr;
	_shooter2 = nullptr;
	_cannon = nullptr;
}

BossShip::~BossShip()
{

}

BossShip* BossShip::createWithWorld(b2World* world, ActionLayer* layer)
{
	BossShip *obj = new (std::nothrow) BossShip();
	if (obj && obj->initWithWorld(world, layer))
	{
		obj->autorelease();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}

bool BossShip::initWithWorld(b2World* world, ActionLayer* layer)
{
	if (!GameObject::initWithSpriteFrameName("Boss_ship.png", world, "Boss_ship", 50, HealthBarTypeRed))
	{
		return false;
	}

	_layer = layer;

	_initialMove = false;

	_shooter1 = Sprite::createWithSpriteFrameName("Boss_shooter.png");
	_shooter1->setPosition(Vec2(this->getContentSize().width * 0.65f, this->getContentSize().height * 0.5f));
	this->addChild(_shooter1);

	_shooter2 = Sprite::createWithSpriteFrameName("Boss_shooter.png");
	_shooter2->setPosition(Vec2(this->getContentSize().width * 0.55f, this->getContentSize().height * 0.1f));
	this->addChild(_shooter2);

	_cannon = Sprite::createWithSpriteFrameName("Boss_cannon.png");
	_cannon->setPosition(Vec2(this->getContentSize().width * 0.5f, this->getContentSize().height * 0.95f));
	this->addChild(_cannon, -1);
	//_cannon->setVisible(false);

	return true;
}

void BossShip::updateWithShipPosition(cocos2d::Vec2 shipPosition)
{
	Size winSize = Director::getInstance()->getWinSize();

	if (!_initialMove)
	{
		_initialMove = true;
		Vec2 midScreen = Vec2(winSize.width/2, winSize.height/2);

		this->runAction(MoveTo::create(4.f, midScreen));
	}

	//计算下BOSS头顶加农炮的位置
	Vec2 cannonHeadWorld = this->convertToWorldSpace(Vec2(_cannon->getPositionX() - _cannon->getContentSize().width/2, _cannon->getPositionY()));
	//计算下加农炮的发射角度
	Vec2 shootVector = cannonHeadWorld - shipPosition;
	//计算下加农炮的旋转角度，加农炮总是指向我们的小飞机，这算法好！
	float cannoAngle = -1 * ccpToAngle(shootVector);
	float rotate = CC_RADIANS_TO_DEGREES(cannoAngle);
	//log("rotate = %f", rotate);
	_cannon->setRotation(rotate);
}

void BossShip::revive()
{
	GameObject::revive();
	_initialMove = false;
	this->randomAction();  //boss开始随机运动
}

void BossShip::randomAction()
{
	Size winSize = Director::getInstance()->getWinSize();
	int randomAction = (int)(Common::randomValueBetween(0, 5)); //随机行动模式，从0到5随机选一个

	FiniteTimeAction *action = nullptr;
	if (randomAction ==0 || !_initialMove)  //0号运动模式, BOSS随机移动
	{
		_initialMove = true;

		float randWidth = winSize.width * Common::randomValueBetween(0.6f, 1.0f); //随机移动的宽度和高度
		float randV = Common::randomValueBetween(0.4f, 0.9f);
		log("randV = %f", randV);
		float randHeight = winSize.height * randV;

		Vec2 randDest = Vec2(randWidth, randHeight);  //随机移动到的目的地

		float randVel = Common::randomValueBetween(winSize.height/4.f, winSize.height/2.f); //随机移动的速度
		float randLength = ccpLength(this->getPosition() - randDest);  //随机移动的距离  当前位置-目标位置
		float randDurtion = randLength / randVel;  //计算下随机移动的时间
		randDurtion = MAX(randDurtion, 0.2f);

		action = MoveTo::create(randDurtion, Vec2(randWidth, randHeight));
	}
	else if (randomAction == 1) //1号行为模式，仅仅是发呆0.2秒
	{
		action = DelayTime::create(0.2f);
	}
	else if (randomAction >=2 && randomAction<4)  //2-3号行为模式，BOSS发射激光
	{
		//Boss的两个激光枪发射激光
		_layer->shootEnemyLaserFromPosition(this->convertToWorldSpace(Vec2(_shooter1->getPositionX() - _shooter1->getContentSize().width/2, _shooter1->getPositionY()))); //因为shooter1是Boss的孩子，所以你不能直接用它的位置，需要转换成世界坐标
		_layer->shootEnemyLaserFromPosition(this->convertToWorldSpace(Vec2(_shooter2->getPositionX() - _shooter2->getContentSize().width/2, _shooter2->getPositionY())));

		action = DelayTime::create(0.2f);
	}
	else if (randomAction == 4)  //4号行为模式,BOSS发射加农大炮  
	{
		//获取加农炮头的世界坐标位置,加农炮锚点在图片中心，但我们要取的炮口位置显然是图片最前边，所以要减去getContentSize().width/2
		Vec2 cannonHeadWorld = this->convertToWorldSpace(Vec2( _cannon->getPositionX() - _cannon->getContentSize().width/2, 
										_cannon->getPositionY())); 
		_layer->shootCannonBallAtShipFromPosition(cannonHeadWorld); //actionLayer从炮口位置发射加农炮

		action = DelayTime::create(0.2f);
	}

	this->runAction(Sequence::create(action, CallFunc::create(CC_CALLBACK_0(BossShip::randomAction, this)), NULL)); //boss无限做随机运动
}