#ifndef _ShapeCache_
#define _ShapeCache_

#include "Box2D/Box2D.h"
#include "cocos2d.h"

//���ʶ���
class FixtureDef
{
public:
	FixtureDef();
	~FixtureDef();

	FixtureDef *next;
	b2FixtureDef fixture;
	int callbackData;
};

//���嶨��
class BodyDef : public cocos2d::Node
{
public:
	BodyDef();
	~BodyDef();
	bool init();
	CREATE_FUNC(BodyDef);
	//virtual void onExit();

public:
	FixtureDef *fixtures;
	cocos2d::Vec2 anchorPoint;
};

class ShapeCache : public cocos2d::Node
{
public:
	ShapeCache();
	~ShapeCache();
	bool init();
	CREATE_FUNC(ShapeCache);

	static ShapeCache* instance;
	static ShapeCache* getInstance();

	void addShapesWithFile(std::string plist);
	//����һ�����ʵ�������
	void addFixturesToBody(b2Body *body, std::string shapeName, float scale);
	//����һ�����ʵ�������,������һ����ȡ�û����ݵĻص�������std::funtion<void* (int)>��ʾ��������void*, ��һ��int����
	void addFixturesToBody(b2Body *body, std::string shapeName, std::function<void*(int)> userdataCallBack, float scale);
	cocos2d::Vec2 anchorPointForShape(std::string shapeName); //������������

	void addShapesWithDictionary(cocos2d::Dictionary* dictionary); //��һ���ֵ��ﴴ����״
protected:
	cocos2d::Map<std::string, BodyDef*> _shapeObjects; //�������ļ���
private:
};
#endif