//=============================================================================
//
// �G�l�~�[���f������ [Ground.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "Grass.h"
#include "Town.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
// �ǂݍ��ރ��f����
#define	MODEL_ENVIRONMENT_PATH		"data/MODEL/Environment/"
#define	MODEL_TOWN_PATH				"data/MODEL/Environment/Knight"
#define	MODEL_FIELD					"data/MODEL/Environment/Land.obj"
#define	MODEL_TREE_NAME				"Tree.fbx"
#define	MODEL_FIELD_NAME			"Land.fbx"
#define	MODEL_BONFIRE_NAME			"bonfire.fbx"
#define	MODEL_TOWN_NAME				"kt.fbx"

#define WORLD_MAX			(XMFLOAT3(200000.0f, 200000.0f, 200000.0f))
#define WORLD_MIN			(XMFLOAT3(-200000.0f, -200000.0f, -200000.0f))
//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct GROUND
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	int					state;
	bool				use;
	bool				load;
	Model*				pModel;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

};



class Ground
{
public:
	Ground();
	~Ground();
	void Update(void);
	void Draw(void);
private:
	SimpleArray<GameObject<SkinnedMeshModelInstance>*> skinnedMeshGroundGO;
	SimpleArray<GameObject<ModelInstance>*>	groundGO;
	Town* town;
	Grass* grass;
	Renderer& renderer = Renderer::get_instance();
};