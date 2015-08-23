#include "LevelManager.h"
#include "Common.h"
USING_NS_CC;
using namespace std;

LevelManager* LevelManager::instance = nullptr;

LevelManager::LevelManager()
{
	_data = nullptr;
	_levels = nullptr;
	_curStages = nullptr;
	_curStage = nullptr;
}

LevelManager::~LevelManager()
{

}

LevelManager* LevelManager::getInstance()
{
	if (instance == nullptr)
	{
		instance = LevelManager::create();
		instance->retain();
	}
	return instance;
}

bool LevelManager::init()
{
	if (!Node::init())
	{
		return false;
	}

	//��ȡLevels.plist������·��
	std::string levelsPlist = FileUtils::getInstance()->fullPathForFilename("Levels.plist");
	//��Levels.plist������ת��Ϊ����
	_data = Dictionary::createWithContentsOfFile(levelsPlist.c_str());
	_data->retain();
	CCASSERT(_data!=nullptr, "Couldn't open Levels file");

	_levels = dynamic_cast<Array*>(_data->objectForKey("Levels")); //��ȡ�й�levels����Ϣ
	_levels->retain();
	CCASSERT(_levels!=nullptr, "Couldn't open Levels entry");

	_curLevelIdx = -1;
	_curStageIdx = -1;
	_gameState = GameStateTitle;

	return true;
}

int LevelManager::curLevelIdx()
{
	return _curLevelIdx;
}

bool LevelManager::hasProp(string prop)
{
	std::string result = _curStage->valueForKey(prop)->getCString();
	return result!="";  //���ؿ��ַ�������ʾû�ҵ�
}

string LevelManager::stringForProp(std::string prop)
{
	std::string result = _curStage->valueForKey(prop)->getCString();
	return result; //�Ҳ���prop����""
}

float LevelManager::floatForProp(std::string prop)
{
	std::string retval = _curStage->valueForKey(prop)->getCString();
	CCASSERT(retval!="", "Couldn't find prop");
	return _curStage->valueForKey(prop)->floatValue();
}

bool LevelManager::boolForProp(std::string prop)
{
	std::string retval = _curStage->valueForKey(prop)->getCString();
	if(retval=="")  //û�ҵ��˹ؼ��֣�����false
		return false;
	//CCASSERT(retval!="", "Couldn't find prop");
	return _curStage->valueForKey(prop)->boolValue();
}

void LevelManager::nextLevel()
{
	_curLevelIdx++;
	if (_curLevelIdx >= _levels->count())
	{
		_gameState = GameStateDone;
		return;
	}
	CC_SAFE_RELEASE_NULL(_curStages);
	_curStages = (Array*)(_levels->objectAtIndex(_curLevelIdx)); //��ȡ�Ե�ǰ���±�ָ�������
	_curStages->retain();
	this->nextStage();
}

void LevelManager::nextStage()
{
	//if(_curStageIdx==-1)
	//	this->nextLevel();

	_curStageIdx++;
	if (_curStageIdx >= _curStages->count())  //�����ǰ�׶��Ѿ������һ���׶Σ���ֱ�ӽ�����һ��
	{
		_curStageIdx = -1;
		this->nextLevel();
		return;
	}

	CC_SAFE_RELEASE_NULL(_curStage);
	_gameState = GameStateNormal;
	_curStage = dynamic_cast<Dictionary*>(_curStages->objectAtIndex(_curStageIdx));
	_curStage->retain();

	_stageDuration = this->floatForProp("Duration");
	_stageStart = Common::getMillSecond(); //��ȡ��ǰ�ؿ�ʼ��ʱ���,����ǰ�׶ο�ʼʱ��

	log("Stage ending in: %f", _stageDuration);
}

bool LevelManager::update()
{
	if (_gameState == GameStateTitle )
	//	_gameState == GameStateDone)
	{
		return false;
	}

	if (_gameState == GameStateDone || _gameState == GameStateFail)
	{
		return true;
	}
	double curTime = Common::getMillSecond();
	if (curTime > _stageStart + _stageDuration * 1000) //������׶γ���ʱ���Ѿ����˵�ǰ�ص�ʱ�䣬�ͽ�����һ��
	{
		this->nextStage();
		return true;
	}

	return false;
}