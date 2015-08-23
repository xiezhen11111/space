#include "ActionLayer.h"
#include "Common.h"
#include "SpriteArray.h"
#include "GameObject.h"
#include "ShapeCache.h"
#include "ParallaxNodeExtras.h"
#include "SimpleAudioEngine.h"
#include "SimpleContactListener.h"
#include "ParticleSystemArray.h"
#include "LevelManager.h"
#include "BossShip.h"

USING_NS_CC;
using namespace std;
using namespace CocosDenshion;

ActionLayer::ActionLayer()
{

}

ActionLayer::~ActionLayer()
{
	CC_SAFE_RELEASE_NULL(_explositions);
	CC_SAFE_RELEASE_NULL(_asteroidsArray);
	CC_SAFE_RELEASE_NULL(_laserArray);
}

Scene* ActionLayer::scene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = ActionLayer::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void ActionLayer::setupTitle()
{
	Size winSize = Director::getInstance()->getWinSize();

	string fontName  = "fonts/SpaceGameFont.fnt";

	_titleLabel1 = LabelBMFont::create("Space Game", fontName);
	_titleLabel1->setScale(0.f);
	_titleLabel1->setPosition(Vec2(winSize.width/2, winSize.height * 0.8f));
	this->addChild(_titleLabel1, 100);
	//space game�ַ���С��󣬽��е�������
	_titleLabel1->runAction(Sequence::create(DelayTime::create(1.f), 
		CallFunc::create([this]() //��ʱ���󲥷ű�������
	{
		SimpleAudioEngine::getInstance()->playEffect("Sounds/title.caf");
	}),
		EaseOut::create(ScaleTo::create(1.f, 0.5f), 4.f), NULL));

	_titleLabel2 = LabelBMFont::create("Starter Kit", fontName);
	_titleLabel2->setScale(0.f);
	_titleLabel2->setPosition(Vec2(winSize.width/2, winSize.height * 0.6f));
	this->addChild(_titleLabel2, 100);
	_titleLabel2->runAction(Sequence::create(DelayTime::create(2.f), 
		EaseOut::create(ScaleTo::create(1.f, 1.25f), 4.f), NULL));

	LabelBMFont* playLabel = LabelBMFont::create("Play", fontName);
	_playItem = MenuItemLabel::create(playLabel, CC_CALLBACK_1(ActionLayer::playTapped, this));
	_playItem->setScale(0.f);
	_playItem->setPosition(winSize.width/2, winSize.height * 0.3f);

	Menu *menu = Menu::create(_playItem, NULL);
	menu->setPosition(Point::ZERO);
	this->addChild(menu);

	_playItem->runAction(Sequence::create(DelayTime::create(2.f), 
		EaseOut::create(ScaleTo::create(0.5f, 0.5f), 4.f), NULL));
}

void ActionLayer::setupWorld()
{
	b2Vec2 gravity = b2Vec2(0.f, 0.f);
	_world = new b2World(gravity);  //���Box2D world��ʱû������

	_contactListener = new SimpleContactListener(this);
	_world->SetContactListener(_contactListener);  //������ײ����
}

void ActionLayer::setupDebugDraw()
{
	_debugDraw = new GLESDebugDraw(PTM_RATIO);
	_world->SetDebugDraw(_debugDraw);   //������ʾbox2d�ĵ�����Ϣ
	_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit); //��ʾ��״�͹ؽڵĵ�����Ϣ
}

void ActionLayer::setupLevelManager()
{
	_levelManager = LevelManager::getInstance();
}

void ActionLayer::setupBoss()
{
	_boss = BossShip::createWithWorld(_world, this);
	_boss->setVisible(false);
	_batchNode->addChild(_boss);
}

void ActionLayer::testBox2D()
{
	Size winSize = Director::getInstance()->getWinSize();

	b2BodyDef bodyDef;  //����һ�����嶨��
	bodyDef.type = b2_dynamicBody; //�������Ƕ�̬��
	bodyDef.position = b2Vec2(winSize.width/2/PTM_RATIO,
							  winSize.height/2/PTM_RATIO);
	b2Body *body = _world->CreateBody(&bodyDef); //���ݴ����嶨�崴��һ������

	//����һ��Բ��
	b2CircleShape circleShape;
	circleShape.m_radius = 0.25f;
	b2FixtureDef fixtureDef;  //�趨�������
	fixtureDef.shape = &circleShape;  //������״��Բ��
	fixtureDef.density = 1.f;  //�����ܶ�
	body->CreateFixture(&fixtureDef); //�ɴ˲��ʲ���һ������

	body->ApplyAngularImpulse(0.01, false);
}

void ActionLayer::playTapped(cocos2d::Ref* pSender)
{
	SimpleAudioEngine::getInstance()->playEffect("Sounds/powerup.caf");

	//��ÿ�������ⶼ��ʧ�ٴӳ�����ɾ��
	Vector<Node*> nodes;
	nodes.pushBack(_titleLabel1);
	nodes.pushBack(_titleLabel2);
	nodes.pushBack(_playItem);
	for (Node* node : nodes)
	{
		node->runAction(Sequence::create(EaseOut::create(ScaleTo::create(0.5f, 0.f), 4.f),
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::removeNode, this)), NULL));
	}
	//�����ɻ�
	this->spawnShip();

	//��ʼ�¹ؿ�
	//_gameStage = GameStageAsteroids;
	_levelManager->nextLevel();
	this->newStageStarted();

	//�趨��Ϸȡʤʱ��
//	double curTime = Common::getMillSecond();
//	_gameWonTime = curTime + 30 * 1000; //30��֮����Ϸʤ��
}

void ActionLayer::newStageStarted()
{
	if (_levelManager->_gameState == GameStateDone)
	{
		this->endScene(true);
	}
	else if (_levelManager->_gameState == GameStateFail)
	{
		this->endScene(false);
	}
	else if (_levelManager->_gameState == GameStateNormal && _levelManager->hasProp("SpawnBoss"))
	{
		this->spawnBoss();
	}
	else if (_levelManager->boolForProp("SpawnLevelIntro"))  //��ʾ�ؿ�����
	{
		this->doLevelIntro();
	}
}

void ActionLayer::spawnBoss()
{
	Size winSize = Director::getInstance()->getWinSize();
	_boss->setPosition(winSize.width * 1.2f, winSize.height * 1.2f);

	_boss->revive();

	this->shakeScreen(30);
	SimpleAudioEngine::getInstance()->playEffect("Sounds/boss.caf");
}

void ActionLayer::removeNode(cocos2d::Node* sender)
{
	sender->removeFromParentAndCleanup(true);
}

void ActionLayer::setupSound()
{
	SimpleAudioEngine::getInstance()->playBackgroundMusic("Sounds/SpaceGame.caf", true);
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/explosion_large.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/explosion_small.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/laser_enemy.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/laser_ship.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/shake.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/powerup.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/boss.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/cannon.caf");
	SimpleAudioEngine::getInstance()->preloadEffect("Sounds/title.caf");
}

void ActionLayer::setupStars()
{
	Size winSize = Director::getInstance()->getWinSize();
	std::vector<string> vec;
	vec.push_back("Particles/Stars1.plist");
	vec.push_back("Particles/Stars2.plist");
	vec.push_back("Particles/Stars3.plist");
	for (vector<string>::iterator it = vec.begin(); it!=vec.end(); it++)
	{
		ParticleSystemQuad *starsEffect = ParticleSystemQuad::create(*it);
		starsEffect->setPosition(Vec2(winSize.width*1.5f, winSize.height/2.f));
		starsEffect->setPosVar(Vec2(starsEffect->getPosVar().x, (winSize.height/2.f)*1.5f));//����������仯��Χ�����Ļ�߶�����֮��
		this->addChild(starsEffect);
	}
}

void ActionLayer::setupBatchNode()
{
	_batchNode = SpriteBatchNode::create("Spritesheets/Sprites.pvr.ccz");
	this->addChild(_batchNode, -1);
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Spritesheets/Sprites.plist");
}

void ActionLayer::setupArrays()
{
	_asteroidsArray = SpriteArray::createWithCapacity(30, "asteroid.png", _batchNode, _world, "asteroid", 1, HealthBarTypeNone);
	_asteroidsArray->retain();
	_laserArray = SpriteArray::createWithCapacity(15, "laserbeam_blue.png", _batchNode, _world, "laserbeam_blue", 1, HealthBarTypeNone);
	_laserArray->retain();
	_alienArray = SpriteArray::createWithCapacity(15, "enemy_spaceship.png", _batchNode, _world, "enemy_spaceship", 1, HealthBarTypeNone);
	_alienArray->retain();

	_explositions = ParticleSystemArray::createWithFile("Particles/Explosion.plist", 3, this);
	_explositions->retain();

	_enemyLasers = SpriteArray::createWithCapacity(15, "laserbeam_red.png", _batchNode, _world, "laserbeam_red", 1, HealthBarTypeNone, 100);
	_enemyLasers->retain();

	_powerups = SpriteArray::createWithCapacity(1, "powerup.png", _batchNode, _world, "powerup", 1, HealthBarTypeNone);
	_powerups->retain();

	_boostEffects = ParticleSystemArray::createWithFile("Particles/Boost.plist", 1, this);
	_boostEffects->retain();

	_cannonBalls = SpriteArray::createWithCapacity(5, "Boss_cannon_ball.png", _batchNode, _world, "Boss_cannon_ball", 1, HealthBarTypeNone,100);
	_cannonBalls->retain();
}

void ActionLayer::setupShapeCache()
{
	ShapeCache::getInstance()->addShapesWithFile("Shapes/Shapes.plist");
}

void ActionLayer::setupBackground()
{
	Size winSize = Director::getInstance()->getWinSize();

	//1 ����һ�������ParallaxNode
	_backgroundNode = ParallaxNodeExtras::node();
	this->addChild(_backgroundNode, -2);

	//2 ���������ӵ�ParallaxNode��Sprite
	_spacedust1 = Sprite::create("Backgrounds/bg_front_spacedust.png");
	_spacedust2 = Sprite::create("Backgrounds/bg_front_spacedust.png");
	_planesunrise = Sprite::create("Backgrounds/bg_planetsunrise.png");
	_galaxy = Sprite::create("Backgrounds/bg_galaxy.png");
	_spacialanomaly = Sprite::create("Backgrounds/bg_spacialanomaly.png");
	_spacialanomaly2 = Sprite::create("Backgrounds/bg_spacialanomaly2.png");

	//3 �趨��ͬ����ͱ����ƶ�������ٶ�
	Vec2 dustSpeed = Vec2(0.1f, 0.1f);
	Vec2 bgSpeed = Vec2(0.05f, 0.05f); //�����ƶ��ٶ�

	//4 ����������ӵ�ParallaxNode
	_backgroundNode->addChild(_spacedust1, 0, dustSpeed, Vec2(0, winSize.height/2));
	_backgroundNode->addChild(_spacedust2, 0, dustSpeed, Vec2(_spacedust1->getContentSize().width * _spacedust1->getScale(), winSize.height/2));
	_backgroundNode->addChild(_galaxy, -1, bgSpeed, Vec2(0, winSize.height*0.7f));
	_backgroundNode->addChild(_planesunrise, -1, bgSpeed, Vec2(600, winSize.height * 0));
	_backgroundNode->addChild(_spacialanomaly, -1, bgSpeed, Vec2(900, winSize.height * 0.3f));
	_backgroundNode->addChild(_spacialanomaly2, -1, bgSpeed, Vec2(1500, winSize.height * 0.9f));
	
}

void ActionLayer::onExit()
{
	CC_SAFE_RELEASE_NULL(_asteroidsArray);
	Layer::onExit();
}

bool ActionLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}
	_canControl = false;
	_canEmitLaser = false;
	_gameOver = false;
	_wantNextStage = false;
	_ship = nullptr;
	_backgroundNode = nullptr;
	_nextAsteroidSpawn = Common::getMillSecond();
	_nextLaserSpawn = Common::getMillSecond();
	_nextAlienSpawn = Common::getMillSecond();
	_nextShootChance = Common::getMillSecond();
	_nextPowerupSpawn = Common::getMillSecond();

	_numAlienSpawns = 0;

	_gameStage = GameStageTitle;

	_laserArray = nullptr;
	_asteroidsArray = nullptr;
	_alienArray = nullptr;
	_enemyLasers = nullptr;

	_world = nullptr;
	_debugDraw = nullptr;

	_contactListener = nullptr;
	_explositions = nullptr;
	_boss = nullptr;

	_invincible = false;
	_boostEffects = nullptr;
	_cannonBalls = nullptr;

	this->setupWorld();
	this->setupDebugDraw();
	//this->testBox2D();
	this->setupShapeCache();

	this->setupSound();
	this->setupTitle();
	this->setupStars();
	this->setupBatchNode();
	this->setupArrays();
	this->setupBackground();
	this->setupBoss();

	//�趨��Ϸȡʤʱ��
	//double curTime = Common::getMillSecond();
	//_gameWonTime = curTime + 300 * 1000; //30��֮����Ϸʤ��
	this->setupLevelManager();

	this->scheduleUpdate();
	return true;
}

Rect ActionLayer::planeRect()
{
	if (!_ship)
	{
		log("_ship is null");
		return Rect(0,0,0,0);
	}
	Size size = Size(78, 53);
	return Rect(-size.width/2+_ship->getPositionX(), -size.height/2+_ship->getPositionY(), size.width, size.height);
}
bool ActionLayer::containsTouchLocation(Touch* touch)							//ע�ᴥ��
{
	// return CCRect::CCRectContainsPoint(rect(), convertTouchToNodeSpaceAR(touch));
	if (!_ship)
	{
		log("_ship is null");
		return false;
	}
	// �����������д�����ת��Ϊģ�����ꡣAR��ʾ�����ê�㡣
	Vec2 touchLocation = touch->getLocationInView(); //��ȡ����������ͼ�е���Ļ���꣬���Ͻ�Ϊԭռ
	touchLocation = Director::getInstance()->convertToGL(touchLocation);  //ת��ΪOpenGL����ϵ����Ϸ��������ϵ
//	Vec2 localPoint = convertToNodeSpace(touchLocation); //ת��Ϊ��������ϵ��ԭ��Ϊê������

	Rect shipRect = this->planeRect();  //�������Ϊ��������
	return shipRect.containsPoint(touchLocation);
	return true;
}

void ActionLayer::spawnShip()
{
	Size winSize = Director::getInstance()->getWinSize();

	//����֡������һ���ɴ�ģ��
	_ship = GameObject::createWithSpriteFrameName("SpaceFlier_sm_1.png", _world, "SpaceFlier_sm_1", 10, HealthBarTypeGreen);//Sprite::createWithSpriteFrameName("SpaceFlier_sm_1.png");
	_ship->setPosition(Vec2(-_ship->getContentSize().width/2, winSize.height*0.5f));
	_ship->revive(); //revive���д�������
	_batchNode->addChild(_ship, 1);

	//ע��ԭ�̳�����������Ӧ�����ݷɴ���λ�õģ�������֪������Ӧ�ĳ������������ֲ��������ֲ�ϰ�ߣ�����PC�ϵ��Բ��㣬���ﱾ��ŭ������������Ӧ�����ô�ͳ�Ĵ����¼������λ������
	Director *pDirector = Director::getInstance();
	//�������㴥���¼�
	auto listener = EventListenerTouchOneByOne::create();
	////��������Ϊtrue�����ô������´���  
	listener->setSwallowTouches(true);//�����´��ݴ��� 
	listener->onTouchBegan = CC_CALLBACK_2(ActionLayer::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(ActionLayer::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(ActionLayer::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this); //��listener��this�󶨣������¼�ί����

	//�ɴ�����ǰ��������֮һ��Ļ���ٻ�����
	_ship->runAction(
		Sequence::create(EaseOut::create(MoveBy::create(0.5f, Vec2(_ship->getContentSize().width/2+winSize.width * 0.3f, 0)),4.f),
		EaseInOut::create(MoveBy::create(0.5f, Vec2(-winSize.width*0.2f, 0)), 4.f),
		CallFunc::create([this]() //�ɻ�ȥ֮����Ա�����
	{
		this->_canControl = true; 
	}),
		NULL)
		);

	//���ض���
	SpriteFrameCache* cache = SpriteFrameCache::getInstance();

	Animation *animation = Animation::create();
	SpriteFrame* aniFrame = cache->spriteFrameByName("SpaceFlier_sm_1.png"); //��һ֡
	SpriteFrame* aniFrame2 = cache->spriteFrameByName("SpaceFlier_sm_2.png"); //��2֡
	animation->addSpriteFrame(aniFrame);
	animation->addSpriteFrame(aniFrame2);
	animation->setDelayPerUnit(0.2f); //ÿ֡����0.2��

	_ship->runAction(RepeatForever::create(Animate::create(animation))); //�ɴ���ͣ���ж���
}

bool ActionLayer::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	if (!_ship)
	{
		log("_ship is null");
		return false;
	}
	if (_ship->dead())
	{
		log("ship is dead");
		return false;
	}
	//��Ϊ�����Ͻ�����Ӧ��ֻ�зɴ�һ�������Ϸ�Χ�ж���������Ϸ������
	/*if (!containsTouchLocation(touch))
	{
		log("!containsTouchLocation(touch)");
		return false;
	}*/

	if (!_canControl)
	{
		log("!_canControl");
		return false;
	}

	Point touchPoint = touch->getLocationInView(); // �Ӵ������ȡ������Ļ����ϵ�е�����
	touchPoint = Director::getInstance()->convertToGL(touchPoint);
	_lastTouch = touchPoint;
	//_offset.x = touchPoint.x - _ship->getPositionX();
	//_offset.y = touchPoint.y - _ship->getPositionY();
	_canEmitLaser = true;
	log("canEmitLaser = true");
	
	return true;
}

void ActionLayer::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
	if (!_ship)
	{
		log("_ship is null");
		return;
	}

	if (_canControl)
	{
		Point touchPoint = touch->getLocationInView(); // �Ӵ������ȡ������Ļ����ϵ�е�����
		touchPoint = Director::getInstance()->convertToGL(touchPoint);

		_offset.x = touchPoint.x - _lastTouch.x;  //�������δ���֮���ƫ��
		_offset.y = touchPoint.y - _lastTouch.y;

		
		float x = _ship->getPositionX() + _offset.x; //��ǰ�ɴ���λ��+��ָ������ƫ�������Ƿɴ����ڵ�λ��
		float y = _ship->getPositionY() + _offset.y;

		_lastTouch = touchPoint;

		//�޶��߽�
		Size winSize = Director::getInstance()->getWinSize();

		float maxY = winSize.height - _ship->getContentSize().height/4;
		float minY = _ship->getContentSize().height/4;
		y = std::min(y, maxY);
		y = std::max(minY, y); //��֤���������Сֵ֮��

		float maxX = winSize.width - _ship->getContentSize().width/4;
		float minX = _ship->getContentSize().width/4;
		x = std::min(x, maxX);
		x = std::max(minX, x);

		_ship->setPosition(x, y);

		_canEmitLaser = true;
		log("canEmitLaser = true");
	}
}

void ActionLayer::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
	_lastTouch = _ship->getPosition();
	_canEmitLaser = false;
}

void ActionLayer::updateAsteroids(float dt)
{
	//if(_gameStage != GameStageAsteroids) return;  
	if(_levelManager->_gameState != GameStateNormal) return; //������Ϸ״̬ʱֱ�ӷ���
	if(!_levelManager->boolForProp("SpawnAsteroids")) return; //��ǰ�׶β�������С���Ǵ��׶Σ�Ҳ����,�еĹؿ���bossս���е���С�ɻ�

	Size winSize = Director::getInstance()->getWinSize();
	//��ʱ�䴴��С������
	double curTime = Common::getMillSecond();
	if (curTime >_nextAsteroidSpawn) //�����ǰʱ���Ѿ�����Ӧ�ô���С���ǵ�ʱ�䣬�ʹ���С����
	{
		//�������һʱ������С����ʱ��
		float spawnSecsLow = _levelManager->floatForProp("ASpawnSecsLow");  //С���ǳ��ֵ����ʱ���
		float spawnSecsHigh = _levelManager->floatForProp("ASpawnSecsHigh"); //С���ǳ��ֵ����ʱ���
		float randSecs = Common::randomValueBetween(spawnSecsLow, spawnSecsHigh); //�೤ʱ���Ժ󴴽�����λ��s
		//log("randSecs = %f", randSecs);
		_nextAsteroidSpawn = randSecs * 1000 + curTime; //curTime��λ�Ǻ���

		//�����С���Ǹ߶ȵ����ֵ
		float randY = Common::randomValueBetween(0.f, winSize.height);

		//С���Ǵ��ҵ����ƶ�ʱ��
		float moveDurationLow = _levelManager->floatForProp("AMoveDurationLow");  //����ʱ����̵�ʱ��
		float moveDurationHigh = _levelManager->floatForProp("AMoveDurationHigh"); //����ʱ�����ʱ��
		float randDuration = Common::randomValueBetween(moveDurationLow, moveDurationHigh);
		//����һ��С����
		GameObject *asteroid = _asteroidsArray->nextSprite();
		asteroid->stopAllActions();
		asteroid->setVisible(true);

		asteroid->setPosition(winSize.width+asteroid->getContentSize().width/2, randY);

		//С���Ǵ�С
		int randNum = (int)(Common::randomValueBetween(0, 3));
		if (randNum == 0)
		{
			asteroid->setScale(0.25f);
			asteroid->setMaxHp(2);
		}
		else if (randNum == 1)
		{
			asteroid->setScale(0.5f);
			asteroid->setMaxHp(4);
		}
		else
		{
			asteroid->setScale(1.f);
			asteroid->setMaxHp(6);
		}
		asteroid->revive();
		//��С���Ǵ���Ļ�ұ��Ƶ��󣬵�ͷʱɾ��
		Sequence* seq = Sequence::create(MoveBy::create(randDuration, Vec2(-winSize.width - asteroid->getContentSize().width, 0)),
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::invisNode, this)), NULL);
		asteroid->runAction(seq);
	}
}

void ActionLayer::updateLasers(float dt)
{
	if (!_canEmitLaser)
	{
		log("_canEmit = false,can't emit");
		return;
	}
	//��ʱ�䴴��С������
	double curTime = Common::getMillSecond();
	log("curTime = %f", curTime);
	log("_nextLaserSpawn = %f", _nextLaserSpawn);
	if (curTime > _nextLaserSpawn) //��ǰʱ�����Ҫ���伤��ʱ�䣬�ͷ���
	{
		log("curTime > _nextLaserSpawn");
		//�������һʱ������С����ʱ��
		//float randSecs = Common::randomValueBetween(0.1f, 0.2f); //�೤ʱ���Ժ󴴽�����λ��s
		float randSecs = Common::randomValueBetween(0.1f, 0.2f);
		log("randSecs = %f", randSecs);
		_nextLaserSpawn = randSecs * 1000 + curTime; //curTime��λ�Ǻ���

		//�趨���伤��
		CCSize winSize = Director::getInstance()->getWinSize();
		SimpleAudioEngine::getInstance()->playEffect("Sounds/laser_ship.caf",false, 1.f, 0.f,0.25f); //���伤������

		GameObject *shipLaser = _laserArray->nextSprite();
		shipLaser->stopAllActions();
		shipLaser->revive(); //�������
		//�趨����λ��,�ڷɴ�ͷ��
		shipLaser->setPosition(_ship->getPosition() + Vec2(shipLaser->getContentSize().width/2, 0));
		Sequence* seq = Sequence::create(MoveBy::create(0.5f, Vec2(winSize.width, 0)), //����ɳ�һ����Ļ���
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::invisNode, this)), NULL); //������ü�����ʧ
		shipLaser->runAction(seq);
	}
	
}

void ActionLayer::updateAlienSwarm(float dt)
{
	if(_levelManager->_gameState != GameStateNormal) return;
	if(!_levelManager->hasProp("SpawnAlienSwarm")) return;  //�����ǰ�׶β��ǲ����л�Ⱥ�Ľ׶Σ����˳�

	Size winSize = Director::getInstance()->getWinSize();

	double curTime = Common::getMillSecond();
	if (curTime > _nextAlienSpawn) //�����ǰʱ���Ѿ�����Ҫ���ֵл���ʱ��
	{
		if (_numAlienSpawns == 0)  //�����ǰ�л���Ϊ0, ��Ҫ���ɵл�
		{
			Vec2 pos1 = Vec2(winSize.width * 1.3f, Common::randomValueBetween(0, winSize.height * 0.1f)); //���������ߵ�1
			Vec2 cp1 = Vec2(Common::randomValueBetween(winSize.width*0.1f, winSize.width*0.6f),           //�������������ߵ�1
						    Common::randomValueBetween(0, winSize.height*0.3f));

			Vec2 pos2 = Vec2(winSize.width * 1.3f, Common::randomValueBetween(winSize.height*0.9f, winSize.height * 1.f)); //���������ߵ�2
			Vec2 cp2 = Vec2(Common::randomValueBetween(winSize.width*0.1f, winSize.width*0.6f),							   //�������������ߵ�2
				Common::randomValueBetween(winSize.height*0.7f, winSize.height*1.f));

			_numAlienSpawns = Common::randomValueBetween(1, 20);   //������ɵĵл���

			float randResult = (int)Common::randomValueBetween(0, 1); //����һ��0-1�����������
			if (randResult <0.5f) //���ֵС��0.5�� ���ñ���������
			{
				_alienSpawnStart = pos1;
				_bezierConfig.controlPoint_1 = cp1;
				_bezierConfig.controlPoint_2 = cp2;
				_bezierConfig.endPosition = pos2;
			}
			else
			{
				_alienSpawnStart = pos2;
				_bezierConfig.controlPoint_1 = cp2;
				_bezierConfig.controlPoint_2 = cp1;
				_bezierConfig.endPosition = pos1;
			}

			_nextAlienSpawn = curTime+ 1000.f;  //һ�����Ժ����ɵл�С�ɻ�
		}
		else
		{
			_nextAlienSpawn = curTime + 0.3f * 1000; //0.3�������ɵл�С�ɻ�
			
			_numAlienSpawns -= 1;   //�л�����1

			GameObject* alien = _alienArray->nextSprite();
			alien->setPosition(_alienSpawnStart);
			alien->revive();

			alien->runAction(BezierTo::create(3.f, _bezierConfig));  //�õл��ر�����������
		}
	}

	curTime = Common::getMillSecond();
	//�����ǰʱ�䵽�˸÷ŵл������ʱ��
	if (curTime > _nextShootChance)
	{
		_nextShootChance = curTime + 0.1f * 1000;

		Object* obj = nullptr;
		CCARRAY_FOREACH(_alienArray->getArray(), obj)
		{
			GameObject* alien = dynamic_cast<GameObject *>(obj);
			if (alien->isVisible())
			{
				int randRes = (int)Common::randomValueBetween(0, 1000000);
				if (randRes % 40 == 0) //���ʺܵ͵�,�ŷ���һ������
				{
					this->shootEnemyLaserFromPosition(alien->getPosition());
				}
			}
		}
	}
}

void ActionLayer::updatePowerups(float dt)
{
	if(_levelManager->_gameState != GameStateNormal) return;
	if(!_levelManager->hasProp("SpawnPowerups")) return;  //�����ǰ�׶β��ǲ�������Ʒ�Ľ׶Σ����˳�

	Size winSize = Director::getInstance()->getWinSize();

	double curTime = Common::getMillSecond();
	if (curTime > _nextPowerupSpawn) //���˲�������Ʒ��ʱ��
	{
		_nextPowerupSpawn = curTime + _levelManager->floatForProp("PSpawnSecs")*1000.f;

		GameObject* powerup = _powerups->nextSprite();
		powerup->setPosition(Vec2(winSize.width, Common::randomValueBetween(0, winSize.height)));
		powerup->revive();
		powerup->runAction(
			Sequence::create(MoveBy::create(5.f, Vec2(-winSize.width*1.5f, 0)),
				CallFuncN::create(CC_CALLBACK_1(ActionLayer::invisNode, this)), NULL)
			);
	}
}

void ActionLayer::updateBoostEffects(float dt)
{
	if(!_ship) return;
	if(!_boostEffects) return;
	for(auto particleSystem : _boostEffects->getArray())
	{
		particleSystem->setPosition(_ship->getPosition());
	}
}

void ActionLayer::updateBoss(float dt)
{
	if(_levelManager->_gameState != GameStateNormal)  return;
	//if(!_levelManager->boolForProp("SpawnBoss")) return;

	if(_boss->isVisible())
	{
		_boss->updateWithShipPosition(_ship->getPosition());
	}
}
void ActionLayer::shootEnemyLaserFromPosition(cocos2d::Vec2 pos)
{
	Size winSize = Director::getInstance()->getWinSize();
	GameObject *shipLaser = _enemyLasers->nextSprite();

	SimpleAudioEngine::getInstance()->playEffect("sounds/laser_enemy.caf", false, 1.f, 0.f, 0.25f);
	shipLaser->setPosition(pos);
	shipLaser->revive();
	
	shipLaser->stopAllActions();
	//�ü�������������Ļ��Ȼ������ʧ
	shipLaser->runAction(
		Sequence::create(
			MoveBy::create(2.f, Vec2(-winSize.width, 0)),
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::invisNode, this)), NULL
		)
	);
}

void ActionLayer::shootCannonBallAtShipFromPosition(Vec2 pos)
{
	//pos = _boss->_cannon->getPosition();
	Size winSize = Director::getInstance()->getWinSize();
	GameObject *cannonBall = _cannonBalls->nextSprite();

	SimpleAudioEngine::getInstance()->playEffect("Sounds/cannon.caf", false, 1.f, 0.f, 0.25f);

	Vec2 shootVector = ccpNormalize(_ship->getPosition() - pos);  //�������һ����BOSS֮������,�õ���������Ϊ�ڵ����һ��ķ���
	Vec2 shootTarget = shootVector * (winSize.width * 2);  //�������ε�֪ʶ��Ŀ����Ϊ�������Է��г��ȣ���Ϊ�ڵ�Ŀ���Ƿ����һ��ģ���Ŀ�겻�ܶ�Ϊ�һ���Ҫ��Ϊ�������һ�λ�ã������о���ҪԶԶ���ڵл����һ��ľ��룬��Ȼ������ڵ��ɵ��һ�λ��(�һ��ܵ���)��ֹͣ������

	//shootTarget = _ship->getPosition();

	cannonBall->setPosition(pos);
	cannonBall->revive();
	//��ũ�ڷ����һ�Ŀ��
	cannonBall->runAction(Sequence::create(MoveBy::create(5.f, shootTarget), 
		CallFuncN::create(CC_CALLBACK_1(ActionLayer::invisNode, this))
		,NULL));
}

void ActionLayer::updateCollisions(float dt)
{
	Sprite *laser = nullptr;
	Object *obj = nullptr;
	CCARRAY_FOREACH(_laserArray->getArray(), obj)
	{
		laser = dynamic_cast<Sprite*>(obj);
		if(!laser)
			continue;
		if(!laser->isVisible())  //�����ⲻ�ɼ�������
			continue;

		Sprite *asteroid = nullptr;
		Object *obj = nullptr;
		//���ÿһ��С���ǣ�����Ƿ��뼤����ײ
		CCARRAY_FOREACH(_asteroidsArray->getArray(), obj)
		{
			asteroid = dynamic_cast<Sprite*>(obj);
			if(!asteroid)
				continue;
			if(!asteroid->isVisible())
				continue;

			if(asteroid->getBoundingBox().intersectsRect(laser->getBoundingBox())) //��������С���ǵ���ײ����ײ
			{
				SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_large.caf", false,1.f, 0.f, 0.25f);
				asteroid->setVisible(false);  //������С���Ƕ�Ҫ��ʧ
				laser->setVisible(false);
				break;
			}
		}
	}
}

void ActionLayer::updateBackground(float dt)
{
	Vec2 backgroundScrollVel = Vec2(-1000, 0);
	_backgroundNode->setPosition(_backgroundNode->getPosition() + dt * backgroundScrollVel); //ParallaxNode�������ܿ�����������Ԫ���ڶ�

	Vector<Sprite*> spaceDusts;
	spaceDusts.pushBack(_spacedust1);
	spaceDusts.pushBack(_spacedust2);
	//��鱳���ϵ�Ԫ���Ƿ�ɳ�����Ļ����������������ƫ�ƣ������ƻص���Ļ���ұߣ����·�
	for (Sprite* spaceDust : spaceDusts)
	{
		Vec2 pos = _backgroundNode->convertToWorldSpace(spaceDust->getPosition());
		float screenWidth = spaceDust->getContentSize().width/2 * spaceDust->getScale(); //��Ļ���
		if (pos.x < -screenWidth)
		{
			_backgroundNode->incrementOffset(Vec2(2*spaceDust->getContentSize().width * spaceDust->getScale(), 0), spaceDust);
		}
	}
	//������һЩ���ƣ����α���������ͬ��
	Vector<Sprite*> backgrounds;
	backgrounds.pushBack(_planesunrise);
	backgrounds.pushBack(_galaxy);
	backgrounds.pushBack(_spacialanomaly);
	backgrounds.pushBack(_spacialanomaly2);
	for (Sprite* background : backgrounds)
	{
		Vec2 pos = _backgroundNode->convertToWorldSpace(background->getPosition());
		float screenWidth = background->getContentSize().width/2 * background->getScale(); //��Ļ���
		if (pos.x < -screenWidth)
		{
			_backgroundNode->incrementOffset(Vec2(2000, 0), background);
		}
	}
}

void ActionLayer::updateBox2D(float dt)
{
	if(!_world)
		return;
	_world->Step(dt, 1, 1);

	for (b2Body *b = _world->GetBodyList(); b; b = b->GetNext())
	{
		if(b->GetUserData()!=NULL)  //���ϵͳ��ÿ��Box2D���壬���������ǵ�userdata�ǲ���һ��GameObject�����GameObject��cocos2d-x�Ķ���
		{
			GameObject *sprite = (GameObject *)(b->GetUserData()); //�����GameObject���Ͱ����ǵ�Box2D��Position�ͽǶ������cocos2d-x���õ�λ�úͽǶ�һ��
			if(!sprite)
				continue;

			b2Vec2 b2Position = b2Vec2(sprite->getPositionX()/PTM_RATIO,
									   sprite->getPositionY()/PTM_RATIO);
			float32 b2Angle = -1 * CC_DEGREES_TO_RADIANS(sprite->getRotation());

			b->SetTransform(b2Position, b2Angle);
		}
	}
}

void ActionLayer::invisNode(Node* sender)
{
	GameObject *gobect = dynamic_cast<GameObject *>(sender);
	if(!gobect)
		return;
	gobect->destroy();
}

void ActionLayer::updateLevel(float dt)
{
	bool newStage = _levelManager->update();
	if (newStage) //������½׶�,�Ϳ�ʼ�½׶�
	{
		this->newStageStarted();
	}
	if (_wantNextStage)
	{
		_wantNextStage = false;
		_levelManager->nextStage();
		this->newStageStarted();
	}
}

void ActionLayer::update(float dt)
{
	this->updateAsteroids(dt);  //����С����λ��
	this->updateLasers(dt);  //���·ɴ�����ļ���
	//this->updateCollisions(dt);  //���¼�����С���ǵ���ײ����
	this->updateBackground(dt); //���¶�㱳������ 
	this->updateBox2D(dt);

	//��30�����Ϸʤ��
	/*double curTime = Common::getMillSecond();
	if (_gameStage == GameStageAsteroids)
	{
		if (curTime > _gameWonTime)
		{
			this->endScene(true);
		}
		
	}*/
	this->updateLevel(dt);

	this->updateAlienSwarm(dt);

	this->updatePowerups(dt);

	this->updateBoostEffects(dt);

	this->updateBoss(dt);
}

void ActionLayer::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Layer::draw(renderer, transform, flags);

	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position);
	kmGLPushMatrix();
	_world->DrawDebugData();
	kmGLPopMatrix();
	//����ǵл������׶�
	if (_levelManager->_gameState == GameStateNormal && _levelManager->boolForProp("SpawnAlienSwarm"))
	{
		//���Ʊ���������
		ccDrawCubicBezier(_alienSpawnStart,
						  _bezierConfig.controlPoint_1,
						  _bezierConfig.controlPoint_2,
						  _bezierConfig.endPosition, 16);
		//���Ʊ�����������ʼ����
		ccDrawLine(_alienSpawnStart, 
				   _bezierConfig.controlPoint_1);
		//���Ʊ�����������ֹ����
		ccDrawLine(_bezierConfig.endPosition, _bezierConfig.controlPoint_2);
	}
	/*
#if CC_ENABLE_BOX2D_INTEGRATION
	GL::enableVertexAttribs( cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION );
	Director* director = Director::getInstance();
	CCASSERT(nullptr != director, "Director is null when seting matrix stack");
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

	_modelViewMV = director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

	_customCommand.init(_globalZOrder);
	_customCommand.func = CC_CALLBACK_0(Box2DTestLayer::onDraw, this);
	renderer->addCommand(&_customCommand);

	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
#endif
	*/
}

void ActionLayer::beginContact(b2Contact* contact)
{
	b2Fixture *fixtureA = contact->GetFixtureA();  //��ײ��A����
	b2Fixture *fixtureB = contact->GetFixtureB();  //��ײ��B����
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();
	GameObject *spriteA = (GameObject*)bodyA->GetUserData();
	GameObject *spriteB = (GameObject*)bodyB->GetUserData();

	if(!spriteA->isVisible() || !spriteB->isVisible())
		return;

	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);
	b2Vec2 b2ContactPoint = manifold.points[0];
	Vec2 contactPoint = Vec2(b2ContactPoint.x * PTM_RATIO, b2ContactPoint.y * PTM_RATIO); //��ײ�Ӵ���

	Size winSize = Director::getInstance()->getWinSize();

	//����Ǽ��������������ײ
	if ((fixtureA->GetFilterData().categoryBits &  
		kCategoryShipLaser &&
		fixtureB->GetFilterData().categoryBits &
		kCategoryEnemy) ||
		(fixtureB->GetFilterData().categoryBits & 
		kCategoryShipLaser &&
		fixtureA->GetFilterData().categoryBits &
		kCategoryEnemy))
	{
		//�����ֳ���һ���Ǽ��⣬��һ���ǵ���
		GameObject *enemyShip = (GameObject*)spriteA;
		GameObject *laser = (GameObject*)spriteB;
		if (fixtureB->GetFilterData().categoryBits & kCategoryEnemy)
		{
			enemyShip = (GameObject *)spriteB;
			laser = (GameObject *)spriteA;
		}

		//ȷ�ϵл��ͼ��ⶼû��
		if (!enemyShip->dead() && !laser->dead())
		{
			enemyShip->takeHit();  //���߶�����
			laser->takeHit();
		}
		if (enemyShip->dead())  //����л�����
		{
			SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_large.caf",false, 1.f, 0.f,0.25f);
			//��������Ч��
			ParticleSystemQuad *explosion = _explositions->nextParticleSystem();
			if (enemyShip->getMaxHp()>3)
			{
				this->shakeScreen(6);
				explosion->setScale(0.5f);
			}
			else if (enemyShip->getMaxHp()>1)
			{
				this->shakeScreen(3);
				explosion->setScale(0.3f);
			}
			else
			{
				this->shakeScreen(1);
				explosion->setScale(0.125f);
			}
			explosion->setPosition(contactPoint);
			
			explosion->resetSystem();

			if (enemyShip == _boss) //���BOSS���ˣ��ͽ�����һ��
			{
				_wantNextStage = true;
			}
		}
		else  //û��,�Ÿ�С��Ч
		{
			SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_small.caf",false, 1.f, 0.f,0.25f);

			ParticleSystemQuad* explosion = _explositions->nextParticleSystem();
			explosion->setScale(0.125f);
			explosion->setPosition(contactPoint);
			explosion->resetSystem();
		}
	}

	//�һ��������ײ
	if ((fixtureA->GetFilterData().categoryBits & kCategoryShip && fixtureB->GetFilterData().categoryBits & kCategoryEnemy) ||
		(fixtureB->GetFilterData().categoryBits & kCategoryShip && fixtureA->GetFilterData().categoryBits & kCategoryEnemy))
	{
		//��һ����һ���ǵл�
		GameObject *enemyShip = (GameObject *)spriteA;
		if (fixtureB->GetFilterData().categoryBits & kCategoryEnemy)
		{
			enemyShip = spriteB;
		}

		if (!enemyShip->dead()) //����л�û��
		{
			SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_large.caf",false, 1.f, 0.f,0.25f);
			this->shakeScreen(1);
			ParticleSystemQuad *explosion = _explositions->nextParticleSystem();
			explosion->setScale(0.5f);
			explosion->setPosition(contactPoint);
			explosion->resetSystem();

			enemyShip->destroy();  //�һ���л�ײ���л�ֱ����
			if(!_invincible) //��������޵�״̬�У��Ű���
			  _ship->takeHit();

			if (_ship->dead())  //�һ����˾�Game Over
			{
				//this->endScene(false);
				_levelManager->_gameState = GameStateFail;
			}
		}
	}

	//�һ��벹��Ʒ����ײ
	if ((fixtureA->GetFilterData().categoryBits & kCategoryShip && fixtureB->GetFilterData().categoryBits & kCategoryPowerup) ||
		(fixtureB->GetFilterData().categoryBits & kCategoryShip && fixtureA->GetFilterData().categoryBits & kCategoryPowerup))
	{
		//��һ��fixtureA��B��һ���ǲ���Ʒ
		GameObject *powerUp = (GameObject *)spriteA;  //�ȼٶ�AΪ����Ʒ
		if (fixtureB->GetFilterData().categoryBits & kCategoryPowerup)  //���BΪ����Ʒ
		{
			powerUp = spriteB;  //��BΪ����Ʒ
		}

		if (!powerUp->dead())
		{
			SimpleAudioEngine::getInstance()->playEffect("sounds/powerup.caf", false, 1.f, 0.f, 1.f);
			powerUp->destroy();  //����֮�󣬲���Ʒ��ʧ
			//�����ǲ���Ʒ���һ��ӳɵĴ���
			float scaleDuration = 1.f;
			float waitDuration = 3.f;
			_invincible = true;
			ParticleSystemQuad *boostEffect = _boostEffects->nextParticleSystem();
			boostEffect->setScale(0.6f);
			//_ship->addChild(boostEffect); //��Ч���ص��ɴ���
			boostEffect->resetSystem(); //���ֱ�����Ч

			_ship->runAction(
				Sequence::create(MoveBy::create(scaleDuration, Vec2(winSize.width * 0.6f, 0)),
				DelayTime::create(waitDuration),
				MoveBy::create(scaleDuration, Vec2(-winSize.width*0.6, 0))
				,NULL)
				);

			this->runAction(
				Sequence::create(ScaleTo::create(scaleDuration, 0.75f),
				DelayTime::create(waitDuration),
				ScaleTo::create(scaleDuration, 1.f),
				CallFuncN::create(CC_CALLBACK_1(ActionLayer::boostDone, this)), NULL)
				);
		}
	}
}

void ActionLayer::boostDone(cocos2d::Node* sender)
{
	_invincible = false;
	for (auto boostEffect : _boostEffects->getArray())
	{
		boostEffect->stopSystem(); //ֹͣ����
	}
}
void ActionLayer::shakeScreen(int times)
{
	auto shakeLow = MoveBy::create(0.025f, Vec2(0, -5)); //����
	auto shakeLowBack = shakeLow->reverse();  //��������
	auto shakeHigh = MoveBy::create(0.025f, Vec2(0, 5)); //����
	auto shakeHighBack = shakeHigh->reverse();
	auto shake = Sequence::create(shakeLow, shakeLowBack, shakeHigh, shakeHighBack, NULL);  //���������������������ٻ���
	Repeat* shakeAction = Repeat::create(shake, times); //�ظ�ִ��times��

	this->runAction(shakeAction); //�ñ���Ϸ��ִ�������
}

void ActionLayer::endContact(b2Contact* contact)
{

}

void ActionLayer::endScene(bool win)
{
	if(_gameOver) return;
	_gameOver = true;
	//_gameStage = GameStageDone;

	Size winSize = Director::getInstance()->getWinSize();

	string message = "";
	if (win)
	{
		message = "You win!";
	}
	else
		message = "You lose!";

	LabelBMFont *label = LabelBMFont::create(message, "fonts/SpaceGameFont.fnt");
	label->setScale(0.1f);
	label->setPosition(Vec2(winSize.width/2, winSize.height * 0.6f));
	this->addChild(label);

	LabelBMFont *restartLabel = LabelBMFont::create("Restart", "fonts/SpaceGameFont.fnt");

	MenuItemLabel *restartItem = MenuItemLabel::create(restartLabel, CC_CALLBACK_1(ActionLayer::restartTapped, this));
	restartItem->setScale(0.1f);
	restartItem->setPosition(Vec2(winSize.width/2, winSize.height * 0.4f));

	Menu *menu = Menu::create(restartItem, NULL);
	menu->setPosition(Point::ZERO);
	this->addChild(menu);

	restartItem->runAction(ScaleTo::create(0.5f, 0.5f));
	label->runAction(ScaleTo::create(0.5f, 0.5f));
}

void ActionLayer::restartTapped(cocos2d::Ref* sender)
{
	Scene *scene = ActionLayer::scene();
	Director::getInstance()->replaceScene(TransitionZoomFlipX::create(0.5f, scene));
}

void ActionLayer::doLevelIntro()
{
	Size winSize = Director::getInstance()->getWinSize();

	string message1 = String::createWithFormat("Level %d", _levelManager->curLevelIdx()+1)->getCString();
	string message2 = _levelManager->stringForProp("LText");

	_levelIntroLabel1 = LabelBMFont::create(message1, "fonts/SpaceGameFont.fnt");
	_levelIntroLabel1->setScale(0.f);
	_levelIntroLabel1->setPosition(Vec2(winSize.width/2, winSize.height * 0.6f));
	this->addChild(_levelIntroLabel1, 100);

	_levelIntroLabel1->runAction(
		Sequence::create(
			EaseOut::create(ScaleTo::create(0.5f, 0.5f), 4.f),
			DelayTime::create(3.f),
			EaseOut::create( ScaleTo::create(0.5f, 0.f), 4.f),
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::removeNode, this)), NULL)
		);

	_levelIntroLabel2 = LabelBMFont::create(message2, "fonts/SpaceGameFont.fnt");
	_levelIntroLabel2->setScale(0.f);
	_levelIntroLabel2->setPosition(Vec2(winSize.width/2, winSize.height * 0.4f));
	this->addChild(_levelIntroLabel2, 100);

	_levelIntroLabel2->runAction(
		Sequence::create(
		EaseOut::create(ScaleTo::create(0.5f, 0.5f), 4.f),
		DelayTime::create(3.f),
		EaseOut::create( ScaleTo::create(0.5f, 0.f), 4.f),
		CallFuncN::create(CC_CALLBACK_1(ActionLayer::removeNode, this)), NULL)
		);
}
