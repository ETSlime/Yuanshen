#pragma once
//=============================================================================
//
// モデル処理 [Player.h]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"

// states
enum PlayerState
{
	IDLE,
	WALK,
	RUN,
	DASH,
	ATTACK,
	JUMP,
	FALL,
	HARD_LANDING,
	HIT,
	KNOCKDOWN,
	REBOUND,
	DEFEND,
	CAST,
	DIE,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PlayerAttribute
{
	XMFLOAT4X4		mtxWorld;	// ワールドマトリックス

	bool			load;

	float			spd;		// 移動スピード
	float			dir;		// 向き
	float			size;		// 当たり判定の大きさ
	bool			use;

	int				state;

	float			targetDir;

	BOOL			stopRun;
};

class Player
{
public:
	Player();
	~Player();
	void Update(void);
	void Draw(void);
	void HandlePlayerMove(void);
	Transform GetTransform(void);
	BOOL CheckIdle(void);

private:

	PlayerAttribute playerAttribute;

	Sigewinne* sigewinne;
	Renderer& renderer = Renderer::get_instance();
};