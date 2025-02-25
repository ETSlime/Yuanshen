#pragma once
//=============================================================================
//
// ƒ‚ƒfƒ‹ˆ— [Player.h]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"
#include "Klee.h"
#include "Lumine.h"
#include "Hilichurl.h"
#include "Mitachurl.h"
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
	void HandlePlayerMove(Transform& transform);
	Transform GetTransform(void);

private:

	Attributes playerAttr;
	Sigewinne* sigewinne;
	Klee* klee;
	Lumine* lumine;

	Hilichurl* hilichurl;
	Mitachurl* mitachurl;

	GameObject<SkinnedMeshModelInstance>* playerGO;
	Renderer& renderer = Renderer::get_instance();
};