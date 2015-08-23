#ifndef Cocos2DxFirstIosSample_CCParallaxNodeExtras_h
#define Cocos2DxFirstIosSample_CCParallaxNodeExtras_h

#include "cocos2d.h"

class PointObject : public cocos2d::Ref
{
public:
	static PointObject * create(cocos2d::Vec2 ratio, cocos2d::Vec2 offset)
	{
		PointObject *ret = new (std::nothrow) PointObject();
		ret->initWithPoint(ratio, offset);
		ret->autorelease();
		return ret;
	}

	bool initWithPoint(cocos2d::Vec2 ratio, cocos2d::Vec2 offset)
	{
		_ratio = ratio;
		_offset = offset;
		_child = nullptr;
		return true;
	}

	inline const cocos2d::Vec2& getRatio() const { return _ratio; };
	inline void setRatio(const cocos2d::Vec2& ratio) { _ratio = ratio; };

	inline const cocos2d::Vec2& getOffset() const { return _offset; };
	inline void setOffset(const cocos2d::Vec2& offset) { _offset = offset; };

	inline cocos2d::Node* getChild() const { return _child; };
	inline void setChild(cocos2d::Node* child) { _child = child; };

private:
	cocos2d::Vec2 _ratio;
	cocos2d::Vec2 _offset;
	cocos2d::Node *_child; // weak ref
};

class ParallaxNodeExtras : public cocos2d::ParallaxNode
{
public:
	ParallaxNodeExtras();

	static ParallaxNodeExtras* node();

	void incrementOffset(cocos2d::Vec2 offset, cocos2d::Node* node);
	// virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags) override;
};
#endif