#include "ShapeCache.h"
USING_NS_CC;
using namespace std;

#define PTM_RATIO 100

FixtureDef::FixtureDef() : next(nullptr)
{

}

FixtureDef::~FixtureDef()
{
	if(next)
	{
		delete next;
		next = nullptr;
	}
	if (fixture.shape)
	{
		delete fixture.shape;
		fixture.shape = nullptr;
	}
	
}

BodyDef::BodyDef()
{

}

BodyDef::~BodyDef()
{
	CC_SAFE_DELETE(fixtures);
}

bool BodyDef::init()
{
	if (!Node::init())
	{
		return false;
	}

	fixtures = nullptr;
	return true;
}
ShapeCache* ShapeCache::instance = nullptr;

ShapeCache::ShapeCache()
{

}

ShapeCache::~ShapeCache()
{
	_shapeObjects.clear();
}

ShapeCache* ShapeCache::getInstance()
{
	if (instance == nullptr)
	{
		instance = ShapeCache::create();
		instance->retain();
	}
	return instance;
}

bool ShapeCache::init()
{
	if (!Node::init())
	{
		return false;
	}
	_shapeObjects.clear();

	return true;
}

void ShapeCache::addFixturesToBody(b2Body *body, std::string shapeName,std::function<void*(int)> userdataCallBack, float scale)
{
	BodyDef* so = (BodyDef*)_shapeObjects.at(shapeName);
	CCASSERT(so!=nullptr, "BodyDef is nullptr");

	FixtureDef* fix = so->fixtures;
	while (fix)
	{
		//Make a copy of the shape and scale it 复制一份形状
		b2PolygonShape* shape = (b2PolygonShape *)fix->fixture.shape;
		b2PolygonShape shapeCopy;
		shapeCopy.Set(shape->m_vertices, shape->m_count); //复制顶点
		for (int i=0; i<shapeCopy.m_count; ++i)
		{
			shapeCopy.m_vertices[i] *= scale; //对每个顶点进行scale放大小操作
		}
		b2FixtureDef fdCopy = fix->fixture;
		fdCopy.shape = &shapeCopy;

		b2Fixture *f = body->CreateFixture(&fdCopy);
		//如果有个人数据，也用回调函数复制一份
	    if (fix->callbackData && userdataCallBack)
		{
			f->SetUserData(userdataCallBack(fix->callbackData)); //函数回调，函数userdataCallBack必须返回值为void*,带个参数int
		}

		fix = fix->next;
	}
}

void ShapeCache::addFixturesToBody(b2Body *body, std::string shapeName, float scale)
{
	this->addFixturesToBody(body, shapeName, nullptr, scale);
	//this->addFixturesToBody(body, shapeName, scale);
}

Vec2 ShapeCache::anchorPointForShape(std::string shapeName)
{
	BodyDef* bd = _shapeObjects.at(shapeName);
	CCASSERT(bd!=NULL, "BodyDef is null");
	return bd->getAnchorPoint();
}

void ShapeCache::addShapesWithDictionary(Dictionary* dictionary)
{
	Dictionary *bodyDict = (Dictionary *)dictionary->objectForKey("bodies");

	b2Vec2 vertices[b2_maxPolygonVertices];

	DictElement* dicEle = NULL;
	Dictionary* dicShape = NULL;
	CCDICT_FOREACH(bodyDict, dicEle)
	{
		const char* bodyName = dicEle->getStrKey();  //获取body名字
		Dictionary *bodyData = dynamic_cast<Dictionary*>(bodyDict->objectForKey(bodyName)); //再取到下一级字典
		CCASSERT(bodyData!=nullptr, "bodyData is nullptr"); //注意bodyData!=nullptr这个条件为假才会触发错误断言，别搞错了！

		//创建一个body object
		BodyDef *bodyDef = BodyDef::create();
		string strAnchorPoint = (bodyData->valueForKey("anchorpoint"))->getCString(); //从表里找到anchorpoint字段，
		bodyDef->setAnchorPoint(PointFromString(strAnchorPoint));

		//迭代每个fixture
		Array *fixtureList = dynamic_cast<CCArray*>(bodyData->objectForKey("fixtures"));
		FixtureDef **nextFixtureDef = &(bodyDef->fixtures);

		CCObject* obj = NULL;
		CCArray* layerArray = NULL;
		CCARRAY_FOREACH(fixtureList, obj)
		{
			Dictionary* fixtureData = dynamic_cast<Dictionary*>(obj);

			b2FixtureDef basicData;

			basicData.filter.categoryBits = (fixtureData->valueForKey("filter_categoryBits"))->intValue();
			basicData.filter.maskBits = (fixtureData->valueForKey("filter_maskBits"))->intValue();
			basicData.filter.groupIndex = (fixtureData->valueForKey("filter_groupIndex"))->intValue();
			basicData.friction = (fixtureData->valueForKey("friction"))->floatValue();
			basicData.density = (fixtureData->valueForKey("density"))->floatValue();
			basicData.restitution = (fixtureData->valueForKey("restitution"))->floatValue();
			basicData.isSensor = (fixtureData->valueForKey("isSensor"))->boolValue();
			int callbackData = (fixtureData->valueForKey("userdataCbValue"))->intValue();

			string fixtureType = (fixtureData->valueForKey("fixture_type"))->getCString();
			//读取多边形
			if (fixtureType == "POLYGON")
			{
				Array *polygonsArray = dynamic_cast<Array *>(fixtureData->objectForKey("polygons"));
				Object* ary = nullptr;
				CCARRAY_FOREACH(polygonsArray, ary)
				{
					Array* polygonArray = dynamic_cast<Array*>(ary);

					FixtureDef *fix = new FixtureDef();
					fix->fixture = basicData;
					fix->callbackData = callbackData;

					b2PolygonShape *polyshape = new b2PolygonShape();
					int vindex = 0;
					CCASSERT(polygonArray->count()<=b2_maxPolygonVertices, "verties too much");
					CCARRAY_FOREACH(polygonArray, obj)
					{
						string strOffset = dynamic_cast<String*>(obj)->getCString();
						Vec2 offset = PointFromString(strOffset);
						vertices[vindex].x = (offset.x / PTM_RATIO) / 2.0 ;  // sprites in editor are highres, points in cocos are lowres
						vertices[vindex].y = (offset.y / PTM_RATIO) / 2.0 ;  // sprites in editor are highres, points in cocos are lowres
						vindex++;
					}

					polyshape->Set(vertices, vindex);
					fix->fixture.shape = polyshape;

					// create a list
					*nextFixtureDef = fix;
					nextFixtureDef = &(fix->next);
				}
			}
			else //除了多边形其他一律报错
			{
				CCASSERT(true,"ERROR!!!!");
			}
		}

		//add the body element to the hash 添加一个body到子集里
		_shapeObjects.insert(bodyName, bodyDef);
	}
}

void ShapeCache::addShapesWithFile(std::string plist)
{
	string path = FileUtils::getInstance()->fullPathForFilename(plist);
	Dictionary *dict = Dictionary::createWithContentsOfFile(path.c_str());

	this->addShapesWithDictionary(dict);

}