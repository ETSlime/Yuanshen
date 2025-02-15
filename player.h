#pragma once
//=============================================================================
//
// ���f������ [Player.h]
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
// �\���̒�`
//*****************************************************************************

struct PlayerAttribute
{
	XMFLOAT4X4		mtxWorld;	// ���[���h�}�g���b�N�X

	bool			load;

	float			spd;		// �ړ��X�s�[�h
	float			dir;		// ����
	float			size;		// �����蔻��̑傫��
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