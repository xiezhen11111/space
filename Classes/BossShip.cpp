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

	//������BOSSͷ����ũ�ڵ�λ��
	Vec2 cannonHeadWorld = this->convertToWorldSpace(Vec2(_cannon->getPositionX() - _cannon->getContentSize().width/2, _cannon->getPositionY()));
	//�����¼�ũ�ڵķ���Ƕ�
	Vec2 shootVector = cannonHeadWorld - shipPosition;
	//�����¼�ũ�ڵ���ת�Ƕȣ���ũ������ָ�����ǵ�С�ɻ������㷨�ã�
	float cannoAngle = -1 * ccpToAngle(shootVector);
	float rotate = CC_RADIANS_TO_DEGREES(cannoAngle);
	//log("rotate = %f", rotate);
	_cannon->setRotation(rotate);
}

void BossShip::revive()
{
	GameObject::revive();
	_initialMove = false;
	this->randomAction();  //boss��ʼ����˶�
}

void BossShip::randomAction()
{
	Size winSize = Director::getInstance()->getWinSize();
	int randomAction = (int)(Common::randomValueBetween(0, 5)); //����ж�ģʽ����0��5���ѡһ��

	FiniteTimeAction *action = nullptr;
	if (randomAction ==0 || !_initialMove)  //0���˶�ģʽ, BOSS����ƶ�
	{
		_initialMove = true;

		float randWidth = winSize.width * Common::randomValueBetween(0.6f, 1.0f); //����ƶ��Ŀ�Ⱥ͸߶�
		float randV = Common::randomValueBetween(0.4f, 0.9f);
		log("randV = %f", randV);
		float randHeight = winSize.height * randV;

		Vec2 randDest = Vec2(randWidth, randHeight);  //����ƶ�����Ŀ�ĵ�

		float randVel = Common::randomValueBetween(winSize.height/4.f, winSize.height/2.f); //����ƶ����ٶ�
		float randLength = ccpLength(this->getPosition() - randDest);  //����ƶ��ľ���  ��ǰλ��-Ŀ��λ��
		float randDurtion = randLength / randVel;  //����������ƶ���ʱ��
		randDurtion = MAX(randDurtion, 0.2f);

		action = MoveTo::create(randDurtion, Vec2(randWidth, randHeight));
	}
	else if (randomAction == 1) //1����Ϊģʽ�������Ƿ���0.2��
	{
		action = DelayTime::create(0.2f);
	}
	else if (randomAction >=2 && randomAction<4)  //2-3����Ϊģʽ��BOSS���伤��
	{
		//Boss����������ǹ���伤��
		_layer->shootEnemyLaserFromPosition(this->convertToWorldSpace(Vec2(_shooter1->getPositionX() - _shooter1->getContentSize().width/2, _shooter1->getPositionY()))); //��Ϊshooter1��Boss�ĺ��ӣ������㲻��ֱ��������λ�ã���Ҫת������������
		_layer->shootEnemyLaserFromPosition(this->convertToWorldSpace(Vec2(_shooter2->getPositionX() - _shooter2->getContentSize().width/2, _shooter2->getPositionY())));

		action = DelayTime::create(0.2f);
	}
	else if (randomAction == 4)  //4����Ϊģʽ,BOSS�����ũ����  
	{
		//��ȡ��ũ��ͷ����������λ��,��ũ��ê����ͼƬ���ģ�������Ҫȡ���ڿ�λ����Ȼ��ͼƬ��ǰ�ߣ�����Ҫ��ȥgetContentSize().width/2
		Vec2 cannonHeadWorld = this->convertToWorldSpace(Vec2( _cannon->getPositionX() - _cannon->getContentSize().width/2, 
										_cannon->getPositionY())); 
		_layer->shootCannonBallAtShipFromPosition(cannonHeadWorld); //actionLayer���ڿ�λ�÷����ũ��

		action = DelayTime::create(0.2f);
	}

	this->runAction(Sequence::create(action, CallFunc::create(CC_CALLBACK_0(BossShip::randomAction, this)), NULL)); //boss����������˶�
}