#pragma once
#include "GameObject.h"

class Town
{
public:
	Town();

	void Update();
	void Draw();

	SimpleArray< GameObject<SkinnedMeshModelInstance>*> models;
	// skyBox;
	//GameObject<SkinnedMeshModelInstance>* plot02;
	//GameObject<SkinnedMeshModelInstance>* plot04;
	//GameObject<SkinnedMeshModelInstance>* plot05;
	//GameObject<SkinnedMeshModelInstance>* plot09;
};