#ifndef _LevelManager_
#define _LevelManager_

#include "cocos2d.h"

typedef enum
{
	GameStateTitle = 0, //����״̬
	GameStateNormal,   //��ͨս��״̬
	GameStateDone,     //��Ϸ����ʤ������
	GameStateFail      //��Ϸʧ�ܽ���
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
	double _stageStart;  //��ǰ�׶ο�ʼʱ��
	double _stageDuration;  //���ؽ׶γ���ʱ��

	cocos2d::Dictionary* _data; //plist�����ݣ������һ���ֵ���ʽ
	cocos2d::Array* _levels;  //�ֵ���ؼ���Levels��Ӧ����,��һ������
	int _curLevelIdx;  //��ǰ�ؿ�
	cocos2d::Array* _curStages; //��ǰ�������Ľ׶�
	int _curStageIdx;
	cocos2d::Dictionary* _curStage;
private:
};
#endif