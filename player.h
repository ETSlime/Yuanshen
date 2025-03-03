#pragma once
//=============================================================================
//
// ���f������ [Player.h]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"
#include "Klee.h"
#include "Lumine.h"
#include "Hilichurl.h"
#include "Mitachurl.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define ACTION_QUEUE_SIZE		(4)
#define ACTION_QUEUE_CLEAR_WAIT	(120)
#define Action(action)			static_cast<UINT>(action)

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct PlayerAction
{
	ActionEnum	actionType;
	UINT		liveTime;
};

struct PlayerAttributes
{
	UINT			actionQueueClearTime;
	PlayerAction	actionQueue[ACTION_QUEUE_SIZE];
	UINT			actionQueueStart;
	UINT			actionQueueEnd;
};


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

	void UpdateActionQueue(void);

	Attributes attributes;
	PlayerAttributes playerAttr;
	Sigewinne* sigewinne;
	Klee* klee;
	Lumine* lumine;

	Hilichurl* hilichurl;
	Mitachurl* mitachurl;

	GameObject<SkinnedMeshModelInstance>* playerGO;
	Renderer& renderer = Renderer::get_instance();
};