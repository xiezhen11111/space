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
	//space game字符从小变大，进行弹性缩放
	_titleLabel1->runAction(Sequence::create(DelayTime::create(1.f), 
		CallFunc::create([this]() //延时过后播放背景音乐
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
	_world = new b2World(gravity);  //这个Box2D world暂时没有重力

	_contactListener = new SimpleContactListener(this);
	_world->SetContactListener(_contactListener);  //设置碰撞监听
}

void ActionLayer::setupDebugDraw()
{
	_debugDraw = new GLESDebugDraw(PTM_RATIO);
	_world->SetDebugDraw(_debugDraw);   //用于显示box2d的调试信息
	_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit); //显示形状和关节的调试信息
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

	b2BodyDef bodyDef;  //创建一个物体定义
	bodyDef.type = b2_dynamicBody; //此物体是动态的
	bodyDef.position = b2Vec2(winSize.width/2/PTM_RATIO,
							  winSize.height/2/PTM_RATIO);
	b2Body *body = _world->CreateBody(&bodyDef); //根据此物体定义创建一个物体

	//定义一个圆形
	b2CircleShape circleShape;
	circleShape.m_radius = 0.25f;
	b2FixtureDef fixtureDef;  //设定物体材质
	fixtureDef.shape = &circleShape;  //物体形状是圆形
	fixtureDef.density = 1.f;  //物体密度
	body->CreateFixture(&fixtureDef); //由此材质产生一个物体

	body->ApplyAngularImpulse(0.01, false);
}

void ActionLayer::playTapped(cocos2d::Ref* pSender)
{
	SimpleAudioEngine::getInstance()->playEffect("Sounds/powerup.caf");

	//让每个结点标题都消失再从场景上删除
	Vector<Node*> nodes;
	nodes.pushBack(_titleLabel1);
	nodes.pushBack(_titleLabel2);
	nodes.pushBack(_playItem);
	for (Node* node : nodes)
	{
		node->runAction(Sequence::create(EaseOut::create(ScaleTo::create(0.5f, 0.f), 4.f),
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::removeNode, this)), NULL));
	}
	//开出飞机
	this->spawnShip();

	//开始新关卡
	//_gameStage = GameStageAsteroids;
	_levelManager->nextLevel();
	this->newStageStarted();

	//设定游戏取胜时间
//	double curTime = Common::getMillSecond();
//	_gameWonTime = curTime + 30 * 1000; //30秒之后游戏胜利
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
	else if (_levelManager->boolForProp("SpawnLevelIntro"))  //显示关卡标题
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
		starsEffect->setPosVar(Vec2(starsEffect->getPosVar().x, (winSize.height/2.f)*1.5f));//发射器纵向变化范围差不多屏幕高度三分之二
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

	//1 创建一个多层结点ParallaxNode
	_backgroundNode = ParallaxNodeExtras::node();
	this->addChild(_backgroundNode, -2);

	//2 创建你打算加到ParallaxNode的Sprite
	_spacedust1 = Sprite::create("Backgrounds/bg_front_spacedust.png");
	_spacedust2 = Sprite::create("Backgrounds/bg_front_spacedust.png");
	_planesunrise = Sprite::create("Backgrounds/bg_planetsunrise.png");
	_galaxy = Sprite::create("Backgrounds/bg_galaxy.png");
	_spacialanomaly = Sprite::create("Backgrounds/bg_spacialanomaly.png");
	_spacialanomaly2 = Sprite::create("Backgrounds/bg_spacialanomaly2.png");

	//3 设定不同物体和背景移动的相对速度
	Vec2 dustSpeed = Vec2(0.1f, 0.1f);
	Vec2 bgSpeed = Vec2(0.05f, 0.05f); //背景移动速度

	//4 把上述精灵加到ParallaxNode
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

	//设定游戏取胜时间
	//double curTime = Common::getMillSecond();
	//_gameWonTime = curTime + 300 * 1000; //30秒之后游戏胜利
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
bool ActionLayer::containsTouchLocation(Touch* touch)							//注册触摸
{
	// return CCRect::CCRectContainsPoint(rect(), convertTouchToNodeSpaceAR(touch));
	if (!_ship)
	{
		log("_ship is null");
		return false;
	}
	// 将世界坐标中触摸点转换为模型坐标。AR表示相对于锚点。
	Vec2 touchLocation = touch->getLocationInView(); //获取触摸点在视图中的屏幕坐标，左上角为原占
	touchLocation = Director::getInstance()->convertToGL(touchLocation);  //转化为OpenGL坐标系，游戏所用坐标系
//	Vec2 localPoint = convertToNodeSpace(touchLocation); //转换为本地坐标系，原点为锚点所在

	Rect shipRect = this->planeRect();  //这个矩形为本地坐标
	return shipRect.containsPoint(touchLocation);
	return true;
}

void ActionLayer::spawnShip()
{
	Size winSize = Director::getInstance()->getWinSize();

	//根据帧名创建一个飞船模型
	_ship = GameObject::createWithSpriteFrameName("SpaceFlier_sm_1.png", _world, "SpaceFlier_sm_1", 10, HealthBarTypeGreen);//Sprite::createWithSpriteFrameName("SpaceFlier_sm_1.png");
	_ship->setPosition(Vec2(-_ship->getContentSize().width/2, winSize.height*0.5f));
	_ship->revive(); //revive里有创建物体
	_batchNode->addChild(_ship, 1);

	//注：原教程是用重力感应来操纵飞船的位置的，众所周知重力感应的臭名昭著，种种不好用种种不习惯，加上PC上调试不便，这里本人怒而不用重力感应，改用传统的触摸事件来解决位置问题
	Director *pDirector = Director::getInstance();
	//创建单点触摸事件
	auto listener = EventListenerTouchOneByOne::create();
	////设置吞噬为true，不让触摸往下传递  
	listener->setSwallowTouches(true);//不向下传递触摸 
	listener->onTouchBegan = CC_CALLBACK_2(ActionLayer::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(ActionLayer::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(ActionLayer::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this); //将listener和this绑定，放入事件委托中

	//飞船先向前滑行三分之一屏幕，再滑回来
	_ship->runAction(
		Sequence::create(EaseOut::create(MoveBy::create(0.5f, Vec2(_ship->getContentSize().width/2+winSize.width * 0.3f, 0)),4.f),
		EaseInOut::create(MoveBy::create(0.5f, Vec2(-winSize.width*0.2f, 0)), 4.f),
		CallFunc::create([this]() //飞回去之后可以被控制
	{
		this->_canControl = true; 
	}),
		NULL)
		);

	//加载动画
	SpriteFrameCache* cache = SpriteFrameCache::getInstance();

	Animation *animation = Animation::create();
	SpriteFrame* aniFrame = cache->spriteFrameByName("SpaceFlier_sm_1.png"); //第一帧
	SpriteFrame* aniFrame2 = cache->spriteFrameByName("SpaceFlier_sm_2.png"); //第2帧
	animation->addSpriteFrame(aniFrame);
	animation->addSpriteFrame(aniFrame2);
	animation->setDelayPerUnit(0.2f); //每帧持续0.2秒

	_ship->runAction(RepeatForever::create(Animate::create(animation))); //飞船不停运行动画
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
	//因为场景上接受响应的只有飞船一个，加上范围判定反而令游戏体验变差
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

	Point touchPoint = touch->getLocationInView(); // 从触摸点获取到在屏幕坐标系中的坐标
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
		Point touchPoint = touch->getLocationInView(); // 从触摸点获取到在屏幕坐标系中的坐标
		touchPoint = Director::getInstance()->convertToGL(touchPoint);

		_offset.x = touchPoint.x - _lastTouch.x;  //计算两次触摸之间的偏移
		_offset.y = touchPoint.y - _lastTouch.y;

		
		float x = _ship->getPositionX() + _offset.x; //当前飞船的位置+手指划过的偏移量就是飞船现在的位置
		float y = _ship->getPositionY() + _offset.y;

		_lastTouch = touchPoint;

		//限定边界
		Size winSize = Director::getInstance()->getWinSize();

		float maxY = winSize.height - _ship->getContentSize().height/4;
		float minY = _ship->getContentSize().height/4;
		y = std::min(y, maxY);
		y = std::max(minY, y); //保证在最大与最小值之间

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
	if(_levelManager->_gameState != GameStateNormal) return; //不在游戏状态时直接返回
	if(!_levelManager->boolForProp("SpawnAsteroids")) return; //当前阶段不是生成小行星带阶段，也返回,有的关卡是boss战，有的是小飞机

	Size winSize = Director::getInstance()->getWinSize();
	//是时间创建小行星吗？
	double curTime = Common::getMillSecond();
	if (curTime >_nextAsteroidSpawn) //如果当前时间已经过了应该创建小行星的时间，就创建小行星
	{
		//计算出下一时刻生成小行星时间
		float spawnSecsLow = _levelManager->floatForProp("ASpawnSecsLow");  //小行星出现的最低时间点
		float spawnSecsHigh = _levelManager->floatForProp("ASpawnSecsHigh"); //小行星出现的最高时间点
		float randSecs = Common::randomValueBetween(spawnSecsLow, spawnSecsHigh); //多长时间以后创建，单位是s
		//log("randSecs = %f", randSecs);
		_nextAsteroidSpawn = randSecs * 1000 + curTime; //curTime单位是毫秒

		//计算出小行星高度的随机值
		float randY = Common::randomValueBetween(0.f, winSize.height);

		//小行星从右到左移动时间
		float moveDurationLow = _levelManager->floatForProp("AMoveDurationLow");  //飞行时间最短的时间
		float moveDurationHigh = _levelManager->floatForProp("AMoveDurationHigh"); //飞行时间最长的时间
		float randDuration = Common::randomValueBetween(moveDurationLow, moveDurationHigh);
		//产生一个小行星
		GameObject *asteroid = _asteroidsArray->nextSprite();
		asteroid->stopAllActions();
		asteroid->setVisible(true);

		asteroid->setPosition(winSize.width+asteroid->getContentSize().width/2, randY);

		//小行星大小
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
		//让小行星从屏幕右边移到左，到头时删除
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
	//是时间创建小行星吗？
	double curTime = Common::getMillSecond();
	log("curTime = %f", curTime);
	log("_nextLaserSpawn = %f", _nextLaserSpawn);
	if (curTime > _nextLaserSpawn) //当前时间过了要发射激光时间，就发射
	{
		log("curTime > _nextLaserSpawn");
		//计算出下一时刻生成小行星时间
		//float randSecs = Common::randomValueBetween(0.1f, 0.2f); //多长时间以后创建，单位是s
		float randSecs = Common::randomValueBetween(0.1f, 0.2f);
		log("randSecs = %f", randSecs);
		_nextLaserSpawn = randSecs * 1000 + curTime; //curTime单位是毫秒

		//设定发射激光
		CCSize winSize = Director::getInstance()->getWinSize();
		SimpleAudioEngine::getInstance()->playEffect("Sounds/laser_ship.caf",false, 1.f, 0.f,0.25f); //发射激光声音

		GameObject *shipLaser = _laserArray->nextSprite();
		shipLaser->stopAllActions();
		shipLaser->revive(); //激光出现
		//设定激光位置,在飞船头部
		shipLaser->setPosition(_ship->getPosition() + Vec2(shipLaser->getContentSize().width/2, 0));
		Sequence* seq = Sequence::create(MoveBy::create(0.5f, Vec2(winSize.width, 0)), //激光飞出一个屏幕宽度
			CallFuncN::create(CC_CALLBACK_1(ActionLayer::invisNode, this)), NULL); //飞完后让激光消失
		shipLaser->runAction(seq);
	}
	
}

void ActionLayer::updateAlienSwarm(float dt)
{
	if(_levelManager->_gameState != GameStateNormal) return;
	if(!_levelManager->hasProp("SpawnAlienSwarm")) return;  //如果当前阶段不是产生敌机群的阶段，就退出

	Size winSize = Director::getInstance()->getWinSize();

	double curTime = Common::getMillSecond();
	if (curTime > _nextAlienSpawn) //如果当前时间已经到了要出现敌机的时间
	{
		if (_numAlienSpawns == 0)  //如果当前敌机数为0, 就要生成敌机
		{
			Vec2 pos1 = Vec2(winSize.width * 1.3f, Common::randomValueBetween(0, winSize.height * 0.1f)); //贝塞尔曲线点1
			Vec2 cp1 = Vec2(Common::randomValueBetween(winSize.width*0.1f, winSize.width*0.6f),           //贝塞尔曲线切线点1
						    Common::randomValueBetween(0, winSize.height*0.3f));

			Vec2 pos2 = Vec2(winSize.width * 1.3f, Common::randomValueBetween(winSize.height*0.9f, winSize.height * 1.f)); //贝塞尔曲线点2
			Vec2 cp2 = Vec2(Common::randomValueBetween(winSize.width*0.1f, winSize.width*0.6f),							   //贝塞尔曲线切线点2
				Common::randomValueBetween(winSize.height*0.7f, winSize.height*1.f));

			_numAlienSpawns = Common::randomValueBetween(1, 20);   //随机生成的敌机数

			float randResult = (int)Common::randomValueBetween(0, 1); //生成一个0-1的随机浮点数
			if (randResult <0.5f) //随机值小于0.5， 设置贝塞尔曲线
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

			_nextAlienSpawn = curTime+ 1000.f;  //一秒钟以后生成敌机小飞机
		}
		else
		{
			_nextAlienSpawn = curTime + 0.3f * 1000; //0.3秒后后生成敌机小飞机
			
			_numAlienSpawns -= 1;   //敌机数减1

			GameObject* alien = _alienArray->nextSprite();
			alien->setPosition(_alienSpawnStart);
			alien->revive();

			alien->runAction(BezierTo::create(3.f, _bezierConfig));  //让敌机沿贝塞尔曲线走
		}
	}

	curTime = Common::getMillSecond();
	//如果当前时间到了该放敌机激光的时间
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
				if (randRes % 40 == 0) //机率很低的,才发射一个激光
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
	if(!_levelManager->hasProp("SpawnPowerups")) return;  //如果当前阶段不是产生补给品的阶段，就退出

	Size winSize = Director::getInstance()->getWinSize();

	double curTime = Common::getMillSecond();
	if (curTime > _nextPowerupSpawn) //到了产生补给品的时间
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
	//让激光向左发射半个屏幕，然后再消失
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

	Vec2 shootVector = ccpNormalize(_ship->getPosition() - pos);  //向量化我机与敌BOSS之间向量,得到的向量即为炮弹与我机的方向
	Vec2 shootTarget = shootVector * (winSize.width * 2);  //向量几何的知识，目标设为向量乘以飞行长度，因为炮弹目标是飞向我机的，但目标不能定为我机，要定为方向是我机位置，但飞行距离要远远大于敌机与我机的距离，不然会出现炮弹飞到我机位置(我机跑掉了)就停止的现象

	//shootTarget = _ship->getPosition();

	cannonBall->setPosition(pos);
	cannonBall->revive();
	//加农炮飞向我机目标
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
		if(!laser->isVisible())  //若激光不可见，跳过
			continue;

		Sprite *asteroid = nullptr;
		Object *obj = nullptr;
		//针对每一个小行星，检查是否与激光相撞
		CCARRAY_FOREACH(_asteroidsArray->getArray(), obj)
		{
			asteroid = dynamic_cast<Sprite*>(obj);
			if(!asteroid)
				continue;
			if(!asteroid->isVisible())
				continue;

			if(asteroid->getBoundingBox().intersectsRect(laser->getBoundingBox())) //如果激光和小行星的碰撞盒相撞
			{
				SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_large.caf", false,1.f, 0.f, 0.25f);
				asteroid->setVisible(false);  //激光与小行星都要消失
				laser->setVisible(false);
				break;
			}
		}
	}
}

void ActionLayer::updateBackground(float dt)
{
	Vec2 backgroundScrollVel = Vec2(-1000, 0);
	_backgroundNode->setPosition(_backgroundNode->getPosition() + dt * backgroundScrollVel); //ParallaxNode动，才能看出其他背景元素在动

	Vector<Sprite*> spaceDusts;
	spaceDusts.pushBack(_spacedust1);
	spaceDusts.pushBack(_spacedust2);
	//检查背景上的元素是否飞出了屏幕，如果是则调整它的偏移，将它移回到屏幕最右边，重新飞
	for (Sprite* spaceDust : spaceDusts)
	{
		Vec2 pos = _backgroundNode->convertToWorldSpace(spaceDust->getPosition());
		float screenWidth = spaceDust->getContentSize().width/2 * spaceDust->getScale(); //屏幕宽度
		if (pos.x < -screenWidth)
		{
			_backgroundNode->incrementOffset(Vec2(2*spaceDust->getContentSize().width * spaceDust->getScale(), 0), spaceDust);
		}
	}
	//下面是一些星云，光晕背景，处理同上
	Vector<Sprite*> backgrounds;
	backgrounds.pushBack(_planesunrise);
	backgrounds.pushBack(_galaxy);
	backgrounds.pushBack(_spacialanomaly);
	backgrounds.pushBack(_spacialanomaly2);
	for (Sprite* background : backgrounds)
	{
		Vec2 pos = _backgroundNode->convertToWorldSpace(background->getPosition());
		float screenWidth = background->getContentSize().width/2 * background->getScale(); //屏幕宽度
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
		if(b->GetUserData()!=NULL)  //针对系统内每个Box2D物体，来看下他们的userdata是不是一个GameObject，这个GameObject是cocos2d-x的东西
		{
			GameObject *sprite = (GameObject *)(b->GetUserData()); //如果是GameObject，就把它们的Box2D的Position和角度设成与cocos2d-x设置的位置和角度一致
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
	if (newStage) //如果是新阶段,就开始新阶段
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
	this->updateAsteroids(dt);  //更新小行星位置
	this->updateLasers(dt);  //更新飞船发射的激光
	//this->updateCollisions(dt);  //更新激光与小行星的碰撞处理
	this->updateBackground(dt); //更新多层背景滚动 
	this->updateBox2D(dt);

	//过30秒后游戏胜利
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
	//如果是敌机产生阶段
	if (_levelManager->_gameState == GameStateNormal && _levelManager->boolForProp("SpawnAlienSwarm"))
	{
		//绘制贝塞尔曲线
		ccDrawCubicBezier(_alienSpawnStart,
						  _bezierConfig.controlPoint_1,
						  _bezierConfig.controlPoint_2,
						  _bezierConfig.endPosition, 16);
		//绘制贝塞尔曲线起始切线
		ccDrawLine(_alienSpawnStart, 
				   _bezierConfig.controlPoint_1);
		//绘制贝塞尔曲线终止切线
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
	b2Fixture *fixtureA = contact->GetFixtureA();  //碰撞的A物体
	b2Fixture *fixtureB = contact->GetFixtureB();  //碰撞的B物体
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();
	GameObject *spriteA = (GameObject*)bodyA->GetUserData();
	GameObject *spriteB = (GameObject*)bodyB->GetUserData();

	if(!spriteA->isVisible() || !spriteB->isVisible())
		return;

	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);
	b2Vec2 b2ContactPoint = manifold.points[0];
	Vec2 contactPoint = Vec2(b2ContactPoint.x * PTM_RATIO, b2ContactPoint.y * PTM_RATIO); //碰撞接触点

	Size winSize = Director::getInstance()->getWinSize();

	//如果是激光类与敌人类相撞
	if ((fixtureA->GetFilterData().categoryBits &  
		kCategoryShipLaser &&
		fixtureB->GetFilterData().categoryBits &
		kCategoryEnemy) ||
		(fixtureB->GetFilterData().categoryBits & 
		kCategoryShipLaser &&
		fixtureA->GetFilterData().categoryBits &
		kCategoryEnemy))
	{
		//先区分出哪一个是激光，哪一个是敌人
		GameObject *enemyShip = (GameObject*)spriteA;
		GameObject *laser = (GameObject*)spriteB;
		if (fixtureB->GetFilterData().categoryBits & kCategoryEnemy)
		{
			enemyShip = (GameObject *)spriteB;
			laser = (GameObject *)spriteA;
		}

		//确认敌机和激光都没死
		if (!enemyShip->dead() && !laser->dead())
		{
			enemyShip->takeHit();  //两者都受伤
			laser->takeHit();
		}
		if (enemyShip->dead())  //如果敌机死了
		{
			SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_large.caf",false, 1.f, 0.f,0.25f);
			//播放粒子效果
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

			if (enemyShip == _boss) //如果BOSS死了，就进入下一关
			{
				_wantNextStage = true;
			}
		}
		else  //没死,放个小特效
		{
			SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_small.caf",false, 1.f, 0.f,0.25f);

			ParticleSystemQuad* explosion = _explositions->nextParticleSystem();
			explosion->setScale(0.125f);
			explosion->setPosition(contactPoint);
			explosion->resetSystem();
		}
	}

	//我机与敌人相撞
	if ((fixtureA->GetFilterData().categoryBits & kCategoryShip && fixtureB->GetFilterData().categoryBits & kCategoryEnemy) ||
		(fixtureB->GetFilterData().categoryBits & kCategoryShip && fixtureA->GetFilterData().categoryBits & kCategoryEnemy))
	{
		//查一下哪一个是敌机
		GameObject *enemyShip = (GameObject *)spriteA;
		if (fixtureB->GetFilterData().categoryBits & kCategoryEnemy)
		{
			enemyShip = spriteB;
		}

		if (!enemyShip->dead()) //如果敌机没死
		{
			SimpleAudioEngine::getInstance()->playEffect("Sounds/explosion_large.caf",false, 1.f, 0.f,0.25f);
			this->shakeScreen(1);
			ParticleSystemQuad *explosion = _explositions->nextParticleSystem();
			explosion->setScale(0.5f);
			explosion->setPosition(contactPoint);
			explosion->resetSystem();

			enemyShip->destroy();  //我机与敌机撞，敌机直接死
			if(!_invincible) //如果不在无敌状态中，才挨打
			  _ship->takeHit();

			if (_ship->dead())  //我机死了就Game Over
			{
				//this->endScene(false);
				_levelManager->_gameState = GameStateFail;
			}
		}
	}

	//我机与补给品类相撞
	if ((fixtureA->GetFilterData().categoryBits & kCategoryShip && fixtureB->GetFilterData().categoryBits & kCategoryPowerup) ||
		(fixtureB->GetFilterData().categoryBits & kCategoryShip && fixtureA->GetFilterData().categoryBits & kCategoryPowerup))
	{
		//查一下fixtureA和B哪一个是补给品
		GameObject *powerUp = (GameObject *)spriteA;  //先假定A为补给品
		if (fixtureB->GetFilterData().categoryBits & kCategoryPowerup)  //如果B为补给品
		{
			powerUp = spriteB;  //设B为补给品
		}

		if (!powerUp->dead())
		{
			SimpleAudioEngine::getInstance()->playEffect("sounds/powerup.caf", false, 1.f, 0.f, 1.f);
			powerUp->destroy();  //碰过之后，补给品消失
			//下面是补给品对我机加成的代码
			float scaleDuration = 1.f;
			float waitDuration = 3.f;
			_invincible = true;
			ParticleSystemQuad *boostEffect = _boostEffects->nextParticleSystem();
			boostEffect->setScale(0.6f);
			//_ship->addChild(boostEffect); //特效挂载到飞船上
			boostEffect->resetSystem(); //出现爆发特效

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
		boostEffect->stopSystem(); //停止爆发
	}
}
void ActionLayer::shakeScreen(int times)
{
	auto shakeLow = MoveBy::create(0.025f, Vec2(0, -5)); //下跳
	auto shakeLowBack = shakeLow->reverse();  //下跳回来
	auto shakeHigh = MoveBy::create(0.025f, Vec2(0, 5)); //上跳
	auto shakeHighBack = shakeHigh->reverse();
	auto shake = Sequence::create(shakeLow, shakeLowBack, shakeHigh, shakeHighBack, NULL);  //下跳，回来，再上跳，再回来
	Repeat* shakeAction = Repeat::create(shake, times); //重复执行times次

	this->runAction(shakeAction); //让本游戏层执行这个震动
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
