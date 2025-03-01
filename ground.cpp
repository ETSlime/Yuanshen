//=============================================================================
//
// Ground処理 [Ground.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "ground.h"
#include "CollisionManager.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define MAX_TREE			(0)

#define TREE_SIZE			(350.0f)
//#define TOWN_SIZE			(125850.0f)
#define TOWN_SIZE			(550.0f)
#define FIELD_SIZE			(126850.0f)

//=============================================================================
// 初期化処理
//=============================================================================
Ground::Ground()
{

	for (int i = 0; i < MAX_TREE; i++)
	{
		GameObject<SkinnedMeshModelInstance>* treeGO = new GameObject<SkinnedMeshModelInstance>();
		treeGO->Instantiate(MODEL_ENVIRONMENT_PATH, MODEL_TREE_NAME, ModelType::Tree);
		treeGO->GetInstance()->pModel->SetBodyDiffuseTexture("data/MODEL/Environment/Bark.jpg");
		treeGO->SetPosition(XMFLOAT3(110.0f + i * 430.0f, 180.0f, 0.0f));
		treeGO->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
		treeGO->SetScale(XMFLOAT3(TREE_SIZE, TREE_SIZE, TREE_SIZE));
		
		treeGO->GetSkinnedMeshModel()->SetBoundingBoxSize(XMFLOAT3(0.1f, 1.0f, 0.1f));
		treeGO->GetSkinnedMeshModel()->SetBoundingBoxLocationOffset(XMFLOAT3(-0.55f, 0.0f, 0.2f));
		treeGO->SetColliderType(ColliderType::WALL);
		CollisionManager::get_instance().RegisterCollider(&treeGO->GetCollider());

		skinnedMeshGroundGO.push_back(treeGO);
	}

	BOUNDING_BOX worldBB;
	worldBB.maxPoint = WORLD_MAX;
	worldBB.minPoint = WORLD_MIN;
	CollisionManager::get_instance().InitOctree(worldBB);

	//town = new Town();

	GameObject<SkinnedMeshModelInstance>* fieldGO = new GameObject<SkinnedMeshModelInstance>();
	fieldGO->Instantiate(MODEL_ENVIRONMENT_PATH, MODEL_FIELD_NAME, ModelType::Field);
	fieldGO->SetScale(XMFLOAT3(FIELD_SIZE, FIELD_SIZE, FIELD_SIZE));
	fieldGO->SetRotation(XMFLOAT3(XM_PI / 2, XM_PI, 0.0f));
	fieldGO->SetPosition(XMFLOAT3(0.0f, -650.0f, 0.0f));
	fieldGO->Update();
	XMMATRIX worldMatrix = fieldGO->GetWorldMatrix();
	fieldGO->GetSkinnedMeshModel()->BuildTrianglesByWorldMatrix(worldMatrix);
	fieldGO->GetSkinnedMeshModel()->BuildOctree();
	fieldGO->SetRenderShadow(false);
	fieldGO->GetSkinnedMeshModel()->SetDrawBoundingBox(false);
	skinnedMeshGroundGO.push_back(fieldGO);
	

	//grass = new Grass();
	//grass->GenerateGrassInstances(fieldGO->GetSkinnedMeshModel()->GetTriangles(), fieldGO->GetSkinnedMeshModel()->GetBoundingBox(), 75);
}

//=============================================================================
// 終了処理
//=============================================================================
Ground::~Ground()
{
	int size = groundGO.getSize();
	for (int i = 0; i < size; i++)
	{
		if (groundGO[i])
			delete groundGO[i];
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void Ground::Update(void)
{
	int size = groundGO.getSize();
	for (int i = 0; i < size; i++)
	{
		if (groundGO[i]->GetUse() == false) continue;
		groundGO[i]->Update();
	}

	size = skinnedMeshGroundGO.getSize();
	for (int i = 0; i < size; i++)
	{
		if (skinnedMeshGroundGO[i]->GetUse() == false) continue;
		skinnedMeshGroundGO[i]->Update();
	}

	if (town)
		town->Update();
}

//=============================================================================
// 描画処理
//=============================================================================
void Ground::Draw(void)
{
	renderer.SetLightModeBuffer(1);

	if (town)
		town->Draw();

	if (renderer.GetRenderMode() == RenderMode::INSTANCE)
	{
		if (grass)
			grass->Draw();
	}
	else if (renderer.GetRenderMode() == RenderMode::OBJ)
	{
		int size = groundGO.getSize();
		for (int i = 0; i < size; i++)
		{
			if (groundGO[i]->GetUse() == false) continue;

			groundGO[i]->Draw();
		}
	}
	else if (renderer.GetRenderMode() == RenderMode::SKINNED_MESH)
	{
		int size = skinnedMeshGroundGO.getSize();
		for (int i = 0; i < size; i++)
		{
			if (skinnedMeshGroundGO[i]->GetUse() == false) continue;

			skinnedMeshGroundGO[i]->Draw();
		}
	}

	renderer.SetLightModeBuffer(0);
}
