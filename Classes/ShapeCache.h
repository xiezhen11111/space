#ifndef _ShapeCache_
#define _ShapeCache_

#include "Box2D/Box2D.h"
#include "cocos2d.h"

//材质定义
class FixtureDef
{
public:
	FixtureDef();
	~FixtureDef();

	FixtureDef *next;
	b2FixtureDef fixture;
	int callbackData;
};

//物体定义
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
	//增加一个材质到集合中
	void addFixturesToBody(b2Body *body, std::string shapeName, float scale);
	//增加一个材质到集合中,增加了一个读取用户数据的回调函数，std::funtion<void* (int)>表示函数返回void*, 有一个int参数
	void addFixturesToBody(b2Body *body, std::string shapeName, std::function<void*(int)> userdataCallBack, float scale);
	cocos2d::Vec2 anchorPointForShape(std::string shapeName); //返回物体的描点

	void addShapesWithDictionary(cocos2d::Dictionary* dictionary); //由一个字典里创建形状
protected:
	cocos2d::Map<std::string, BodyDef*> _shapeObjects; //存放物体的集合
private:
};
#endif