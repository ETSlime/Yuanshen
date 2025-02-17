#pragma once
//=============================================================================
//
// ƒ‚ƒfƒ‹ˆ— [Player.h]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"

//*****************************************************************************
// \‘¢‘Ì’è‹`
//*****************************************************************************



class Player
{
public:
	Player();
	~Player();
	void Update(void);
	void Draw(void);
	void HandlePlayerMove(void);
	Transform GetTransform(void);

private:

	Attributes playerAttr;
	Sigewinne* sigewinne;
	GameObject<SkinnedMeshModelInstance>* playerGO;
	Renderer& renderer = Renderer::get_instance();
};