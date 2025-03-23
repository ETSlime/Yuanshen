#pragma once
//=============================================================================
//
// モデル処理 [Player.h]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"
#include "Klee.h"
#include "Lumine.h"
#include "Hilichurl.h"
#include "Mitachurl.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ACTION_QUEUE_SIZE		(4)
#define ACTION_QUEUE_CLEAR_WAIT	(40)
#define PLAYER_HIT_WINDOW		(90)
#define Action(action)			static_cast<UINT>(action)

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PlayerAction
{
	ActionEnum	actionType;
	float		liveTime;
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
	void DrawEffect(void);
	Transform GetTransform(void) const;

private:

	void UpdateActionQueue(void);
	void HandlePlayerMove(Transform& transform);
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