#ifndef _SimpleContactListener_
#define _SimpleContactListener_

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "ActionLayer.h"

class SimpleContactListener : public b2ContactListener
{
public:
	ActionLayer* _layer;

	SimpleContactListener(ActionLayer *layer) : _layer(layer) { 
	}

	void BeginContact(b2Contact* contact)
	{
		_layer->beginContact(contact);  //开始接触
	}

	void EndContact(b2Contact* contact) { 
		_layer->endContact(contact);  //结束接触
	}
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) { 
	}

	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {  
	}
};
#endif