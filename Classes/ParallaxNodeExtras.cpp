#include "ParallaxNodeExtras.h"
USING_NS_CC;

/*
class PointObject  : Object {
	CC_SYNTHESIZE(Vec2, m_tRatio, Ratio);
	CC_SYNTHESIZE(Vec2, m_tOffset, Offset);
	CC_SYNTHESIZE(Node *,m_pChild, Child);	// weak ref
};
*/

ParallaxNodeExtras::ParallaxNodeExtras()
{
	//ParallaxNode::ParallaxNode();
}

ParallaxNodeExtras* ParallaxNodeExtras::node()
{
	return new ParallaxNodeExtras();
}

void ParallaxNodeExtras::incrementOffset(cocos2d::Vec2 offset, cocos2d::Node* node)
{
	for (unsigned int i=0; i<_parallaxArray->num; i++)
	{
		//PointObject *point = dynamic_cast<PointObject *>(_parallaxArray->arr[i]);//(PointObject *)_parallaxArray->arr[i];
		PointObject *point = (PointObject*)_parallaxArray->arr[i];
		if(!point)
			continue;
		Node* curNode = point->getChild();
		if(!curNode)
			continue;;
		if (curNode == node)
		{
			point->setOffset(point->getOffset() +  offset );
			//curNode->setPosition(ccpAdd(curNode->getPosition(),offset));
			break;
		}
	}
}

