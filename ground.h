//=============================================================================
//
// �n�`����ъ��I�u�W�F�N�g�Ǘ�[Ground.h]
// Author : 
// �t�B�[���h�E���E���I�u�W�F�N�g�̕`��ƍX�V���Ǘ�����
// 
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "Environment.h"
#include "Town.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
// �ǂݍ��ރ��f����
#define MODEL_BANYAN_PATH			"data/MODEL/Environment/Banyan/Big_Banyan_121.obj"
#define	MODEL_ENVIRONMENT_PATH		"data/MODEL/Environment/"
#define	MODEL_FIELD_PATH			"data/MODEL/Environment/Land.obj"
#define	MODEL_TOWN_PATH				"data/MODEL/Environment/Knight"
#define	MODEL_TREE_NAME				"Tree.fbx"
#define	MODEL_FIELD_NAME			"Land.fbx"
#define MODEL_BONFIRE_PATH			"data/MODEL/Environment/bonfire.obj"

#define WORLD_MAX			(XMFLOAT3(50000.0f, 50000.0f, 50000.0f))
#define WORLD_MIN			(XMFLOAT3(-50000.0f, -50000.0f, -50000.0f))
//*****************************************************************************
// �\���̒�`
//*****************************************************************************

class Ground
{
public:
	Ground();
	~Ground();
	void Update(void);
	void Draw(void);
	void CollectStaticShadowMeshes(SimpleArray<StaticRenderData>& outMeshes);
private:
	SimpleArray<GameObject<SkinnedMeshModelInstance>*> skinnedMeshGroundGO;
	SimpleArray<GameObject<ModelInstance>*>	groundGO;
	Town* town = nullptr;
	Environment* environment = nullptr;
	Renderer& renderer = Renderer::get_instance();
};