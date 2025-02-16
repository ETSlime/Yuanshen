#pragma once
//=============================================================================
//
// ���f������ [Player.h]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"

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
	GameObject<SkinnedMeshModelInstance>* playerGO;
	Renderer& renderer = Renderer::get_instance();
};