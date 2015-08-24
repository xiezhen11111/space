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

#define kCategoryShip      0x1  //�ɴ���
#define kCategoryShipLaser 0x2  //�ɴ�����ļ�����
#define kCategoryEnemy     0x4  //������
#define kCategoryPowerup   0x8  //���õĲ���Ʒ��

enum GameStage {
	GameStageTitle = 0,		 //����Ϸ����ʱ�ĳ���
	GameStageAsteroids = 1,  //��С���Ǵ�ʱ�ĳ���
	GameStageDone			 //��Ϸ��������
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

	void beginContact(b2Contact* contact);  //��ײ����ʱ�Ĵ���
	void endContact(b2Contact* contact);  //��ײ����ʱ�Ĵ���

	void shootEnemyLaserFromPosition(cocos2d::Vec2 pos); //�л���ĳ��λ�÷��䷢�伤��
	void shootCannonBallAtShipFromPosition(cocos2d::Vec2 pos);  //�л���ĳ��λ�÷����ڵ�
protected:
	void setupTitle(); //���ñ���
	void setupSound(); //��������
	void setupStars(); //������������
	void setupBatchNode(); //����ͼƬ��������
	void setupArrays();   //���þ��黺�����
	void setupBackground();  //�����±���
	void setupWorld(); //������������
	void setupDebugDraw(); //����������Ϣ
	void setupShapeCache(); //��ȡ������Ϣ
	void setupLevelManager();  //����ؿ�
	void setupBoss();		   //����Boss

	void testBox2D();   //����Box2D����

	void playTapped(cocos2d::Ref* pSender);//��ʼ��Ϸ

	void removeNode(cocos2d::Node* sender);
	void invisNode(cocos2d::Node* sender); //ʹ�ڵ�����
	void boostDone(cocos2d::Node* sender); //������ʧ

	void spawnShip();  //����һ���ɻ�
	void spawnBoss();  //����һ��BOSS
	void doLevelIntro();  //��ʾ�ؿ�����

	cocos2d::Rect planeRect();  //��ȡ�ɴ��ľ��δ�С
	bool containsTouchLocation(cocos2d::Touch* touch);

	bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event);

	void update(float dt);
	void updateAsteroids(float dt); //����С����
	void updateLasers(float dt); //���¼���
	void updateCollisions(float dt);  //������ײ
	void updateBackground(float dt); //���±�������
	void updateBox2D(float dt); //����Box2D���磨���������ʹ��cocos2d������box2d�������꣬����box2d������Ҫʱ��ȥ������ײ��⣩
	void updateLevel(float dt); //���¹ؿ�
	void updateAlienSwarm(float dt); //�������ǵл�Ⱥ
	void updatePowerups(float dt);  //���²�Ʒ
	void updateBoostEffects(float dt);  //���±���Ч��
	void updateBoss(float dt);
	
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;

	void shakeScreen(int times); //��

	void endScene(bool win); //��Ϸ����ʱ����

	void restartTapped(cocos2d::Ref* sender);  //������Ϸ

	void newStageStarted();  //һ���ؿ����½׶ο�ʼ

protected:
	//void onDraw();
protected:
	cocos2d::LabelBMFont* _titleLabel1; //����1
	cocos2d::LabelBMFont* _titleLabel2; //����2
	cocos2d::LabelBMFont* _levelIntroLabel1; //�ؿ�����1
	cocos2d::LabelBMFont* _levelIntroLabel2; //�ؿ�����2

	cocos2d::MenuItemLabel* _playItem;  //��ʼ��Ϸ��ť

	cocos2d::SpriteBatchNode* _batchNode; //��������
	GameObject* _ship; //�ҷ��ɴ�

	bool _canControl; //�ҷ��ɴ��Ƿ��ܱ�����
	cocos2d::Vec2 _offset; //�ɴ��ƶ�ƫ��
	cocos2d::Vec2 _lastTouch; //�ϴε��

	double _nextAsteroidSpawn;  //�´�Ҫ����С���ǵ�ʱ��
	double _nextLaserSpawn;  //�´μ������ʱ��

	bool _canEmitLaser; //�ܷ��伤�� ����ʱ���Է���

	SpriteArray* _asteroidsArray; //С���Ƕ���
	SpriteArray* _laserArray;  //�������
	SpriteArray* _alienArray;  //�����˵л�����
	SpriteArray* _powerups;    //�ӳɵĲ�Ʒ
	SpriteArray* _cannonBalls; //��ũ�ڵ��ڵ�

	double _nextPowerupSpawn;  //�´β�Ʒ����ʱ��
	double _nextAlienSpawn;   //�´εл�Ⱥ���ֵ�ʱ��
	int _numAlienSpawns;   //�л�������Ŀ
	cocos2d::Vec2 _alienSpawnStart; //�л����ֵص�
	cocos2d::ccBezierConfig _bezierConfig;  //�л����е�����

	double _nextShootChance;   //�´εл����伤��ʱ��
	SpriteArray* _enemyLasers; //���˷���ļ���

	bool _invincible;  //�Ƿ��޵�
	ParticleSystemArray* _boostEffects; //����ʱ��Ч��

protected:
	ParallaxNodeExtras *_backgroundNode;  //˫�㱳�����
	cocos2d::Sprite* _spacedust1;  //̫������
	cocos2d::Sprite* _spacedust2;
	cocos2d::Sprite* _planesunrise; //�Ź�ĺ���
	cocos2d::Sprite* _galaxy; //����״����
	cocos2d::Sprite* _spacialanomaly; //̫���в���������
	cocos2d::Sprite* _spacialanomaly2;

	GameStage _gameStage;  //��ǰ��Ϸ����
	bool _gameOver;

	//double _gameWonTime; //��Ϸȡʤ��ʱ�ޣ���ֹ����ʱ����Ϸ��Ӯ��
	LevelManager* _levelManager;

	BossShip *_boss;
	bool _wantNextStage;

protected:
	b2World* _world; //��������
	GLESDebugDraw* _debugDraw; //�����û���
	b2ContactListener* _contactListener; //��ײ���������Ǹ����࣬Ҫʵ����

	ParticleSystemArray* _explositions; //����Ч����
private:
};
#endif // !_ActionLayer_
