#pragma once
//=============================================================================
//
// Townèàóù [Town.h]
// Author : 
//
//=============================================================================
#include "GameObject.h"

class Town
{
public:
	Town();

	void Update();
	void Draw();

	SimpleArray< GameObject<SkinnedMeshModelInstance>*> models;

	Renderer& renderer = Renderer::get_instance();
};