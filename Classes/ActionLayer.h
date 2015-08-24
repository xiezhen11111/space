#ifndef _ActionLayer_
#define _ActionLayer_

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "GLES-Render.h"

class BossShip;

class SpriteArray;
class ParallaxNodeExtras;
class GameObject;
class ParticleSystemArray;
class LevelManager;

#define kCategoryShip      0x1  //飞船类
#define kCategoryShipLaser 0x2  //飞船发射的激光类
#define kCategoryEnemy     0x4  //敌人类
#define kCategoryPowerup   0x8  //有用的补给品类

enum GameStage {
	GameStageTitle = 0,		 //在游戏标题时的场景
	GameStageAsteroids = 1,  //在小行星带时的场景
	GameStageDone			 //游戏结束场景
};
class ActionLayer : public cocos2d::Layer
{
public:
	ActionLayer();
	~ActionLayer();
	bool init();
	CREATE_FUNC(ActionLayer);
	static cocos2d::Scene* scene();

	virtual void onExit();

	void beginContact(b2Contact* contact);  //碰撞发生时的处理
	void endContact(b2Contact* contact);  //碰撞结束时的处理

	void shootEnemyLaserFromPosition(cocos2d::Vec2 pos); //敌机从某个位置发射发射激光
	void shootCannonBallAtShipFromPosition(cocos2d::Vec2 pos);  //敌机从某个位置发射炮弹
protected:
	void setupTitle(); //设置标题
	void setupSound(); //设置声音
	void setupStars(); //设置星星粒子
	void setupBatchNode(); //设置图片批处理结点
	void setupArrays();   //设置精灵缓存队列
	void setupBackground();  //设置下背景
	void setupWorld(); //创建物理世界
	void setupDebugDraw(); //创建调试信息
	void setupShapeCache(); //读取物体信息
	void setupLevelManager();  //管理关卡
	void setupBoss();		   //设置Boss

	void testBox2D();   //测试Box2D环境

	void playTapped(cocos2d::Ref* pSender);//开始游戏

	void removeNode(cocos2d::Node* sender);
	void invisNode(cocos2d::Node* sender); //使节点隐藏
	void boostDone(cocos2d::Node* sender); //爆发消失

	void spawnShip();  //产生一个飞机
	void spawnBoss();  //产生一个BOSS
	void doLevelIntro();  //显示关卡介绍

	cocos2d::Rect planeRect();  //获取飞船的矩形大小
	bool containsTouchLocation(cocos2d::Touch* touch);

	bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event);

	void update(float dt);
	void updateAsteroids(float dt); //更新小行星
	void updateLasers(float dt); //更新激光
	void updateCollisions(float dt);  //更新碰撞
	void updateBackground(float dt); //更新背景滚动
	void updateBox2D(float dt); //更新Box2D世界（尽管你可以使用cocos2d来更新box2d对象坐标，但是box2d还是需要时间去处理碰撞检测）
	void updateLevel(float dt); //更新关卡
	void updateAlienSwarm(float dt); //更新外星敌机群
	void updatePowerups(float dt);  //更新补品
	void updateBoostEffects(float dt);  //更新爆发效果
	void updateBoss(float dt);
	
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;

	void shakeScreen(int times); //震动

	void endScene(bool win); //游戏结束时出现

	void restartTapped(cocos2d::Ref* sender);  //重新游戏

	void newStageStarted();  //一个关卡的新阶段开始

protected:
	//void onDraw();
protected:
	cocos2d::LabelBMFont* _titleLabel1; //标题1
	cocos2d::LabelBMFont* _titleLabel2; //标题2
	cocos2d::LabelBMFont* _levelIntroLabel1; //关卡介绍1
	cocos2d::LabelBMFont* _levelIntroLabel2; //关卡介绍2

	cocos2d::MenuItemLabel* _playItem;  //开始游戏按钮

	cocos2d::SpriteBatchNode* _batchNode; //批处理结点
	GameObject* _ship; //我方飞船

	bool _canControl; //我方飞船是否能被控制
	cocos2d::Vec2 _offset; //飞船移动偏移
	cocos2d::Vec2 _lastTouch; //上次点击

	double _nextAsteroidSpawn;  //下次要产生小行星的时间
	double _nextLaserSpawn;  //下次激光产生时间

	bool _canEmitLaser; //能否发射激光 触屏时可以发射

	SpriteArray* _asteroidsArray; //小行星队列
	SpriteArray* _laserArray;  //激光队列
	SpriteArray* _alienArray;  //外星人敌机队列
	SpriteArray* _powerups;    //加成的补品
	SpriteArray* _cannonBalls; //加农炮的炮弹

	double _nextPowerupSpawn;  //下次补品出现时间
	double _nextAlienSpawn;   //下次敌机群出现的时间
	int _numAlienSpawns;   //敌机出现数目
	cocos2d::Vec2 _alienSpawnStart; //敌机出现地点
	cocos2d::ccBezierConfig _bezierConfig;  //敌机飞行的曲线

	double _nextShootChance;   //下次敌机发射激光时间
	SpriteArray* _enemyLasers; //敌人发射的激光

	bool _invincible;  //是否无敌
	ParticleSystemArray* _boostEffects; //爆发时的效果

protected:
	ParallaxNodeExtras *_backgroundNode;  //双层背景结点
	cocos2d::Sprite* _spacedust1;  //太空垃圾
	cocos2d::Sprite* _spacedust2;
	cocos2d::Sprite* _planesunrise; //放光的恒星
	cocos2d::Sprite* _galaxy; //星云状物体
	cocos2d::Sprite* _spacialanomaly; //太空中不规则物体
	cocos2d::Sprite* _spacialanomaly2;

	GameStage _gameStage;  //当前游戏场景
	bool _gameOver;

	//double _gameWonTime; //游戏取胜的时限，坚持过这段时间游戏就赢了
	LevelManager* _levelManager;

	BossShip *_boss;
	bool _wantNextStage;

protected:
	b2World* _world; //物理世界
	GLESDebugDraw* _debugDraw; //调试用画笔
	b2ContactListener* _contactListener; //碰撞监听，这是个虚类，要实现它

	ParticleSystemArray* _explositions; //粒子效果集
private:
};
#endif // !_ActionLayer_
