#ifndef _LevelManager_
#define _LevelManager_

#include "cocos2d.h"

typedef enum
{
	GameStateTitle = 0, //标题状态
	GameStateNormal,   //普通战斗状态
	GameStateDone,     //游戏正常胜利结束
	GameStateFail      //游戏失败结束
} GameState;

class LevelManager : public cocos2d::Node
{
public:
	LevelManager();
	~LevelManager();

	bool init();
	CREATE_FUNC(LevelManager);

	static LevelManager* instance;
	static LevelManager* getInstance();

	GameState _gameState;

	int curLevelIdx();
	void nextStage();
	void nextLevel();
	bool update();
	float floatForProp(std::string prop);
	std::string stringForProp(std::string prop);
	bool boolForProp(std::string prop);
	bool hasProp(std::string prop);

protected:
	double _stageStart;  //当前阶段开始时间
	double _stageDuration;  //本关阶段持续时间

	cocos2d::Dictionary* _data; //plist中数据，被存成一个字典形式
	cocos2d::Array* _levels;  //字典里关键字Levels对应的项,是一个数组
	int _curLevelIdx;  //当前关卡
	cocos2d::Array* _curStages; //当前关所处的阶段
	int _curStageIdx;
	cocos2d::Dictionary* _curStage;
private:
};
#endif