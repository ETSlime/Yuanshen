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
#define FIELD_SIZE			(56850.0f)
#define BONFIRE_SIZE		(80.0f)

#define PLAYER_INIT_POS		XMFLOAT3(12582.0f, -2424.0f, -19485.0f)

//=============================================================================
// 初期化処理
//=============================================================================
Ground::Ground()
{
	Transform trans;
	trans.pos = XMFLOAT3(12937.0f, -2384.0f, -19485.0f);

	for (int i = 0; i < MAX_TREE; i++)
	{
		float radius = 20000.f;
		float r = GetRandFloat(2000.0f, radius);
		float angle = GetRandFloat(0.0f, 2.0f * 3.14159265359f);
		float x = trans.pos.x + r * cos(angle);
		float z = trans.pos.z + r * sin(angle);
		float size = GetRandFloat(TREE_SIZE * .8f, TREE_SIZE * 2.5f);
		GameObject<SkinnedMeshModelInstance>* treeGO = new GameObject<SkinnedMeshModelInstance>();
		treeGO->Instantiate(MODEL_ENVIRONMENT_PATH, MODEL_TREE_NAME, ModelType::Tree);
		treeGO->GetInstance()->pModel->SetBodyDiffuseTexture("data/MODEL/Environment/Bark.jpg");
		treeGO->SetPosition(XMFLOAT3(x, trans.pos.y, z));
		treeGO->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
		treeGO->SetScale(XMFLOAT3(size, size, size));
		
		treeGO->GetSkinnedMeshModel()->SetBoundingBoxSize(XMFLOAT3(0.1f, 1.0f, 0.1f));
		treeGO->GetSkinnedMeshModel()->SetBoundingBoxLocationOffset(XMFLOAT3(-0.55f, 0.0f, 0.2f));
		treeGO->SetColliderOwner(treeGO);
		treeGO->SetColliderEnable(true);
		treeGO->SetColliderType(ColliderType::TREE);
		CollisionManager::get_instance().RegisterDynamicCollider(&treeGO->GetCollider());

		skinnedMeshGroundGO.push_back(treeGO);
	}

	BOUNDING_BOX worldBB;
	worldBB.maxPoint = WORLD_MAX;
	worldBB.minPoint = WORLD_MIN;
	CollisionManager::get_instance().InitOctree(worldBB);

	town = new Town();

	XMMATRIX worldMatrix;

	GameObject<SkinnedMeshModelInstance>* fieldGO = new GameObject<SkinnedMeshModelInstance>();
	fieldGO->Instantiate(MODEL_ENVIRONMENT_PATH, MODEL_FIELD_NAME, ModelType::Field);
	fieldGO->SetScale(XMFLOAT3(FIELD_SIZE, FIELD_SIZE, FIELD_SIZE));
	fieldGO->SetRotation(XMFLOAT3(XM_PI / 2, XM_PI, 0.0f));
	fieldGO->SetPosition(XMFLOAT3(0.0f, -650.0f, 0.0f));
	fieldGO->Update();
	worldMatrix = fieldGO->GetWorldMatrix();
	fieldGO->GetSkinnedMeshModel()->BuildTrianglesByWorldMatrix(worldMatrix, true);
	fieldGO->GetSkinnedMeshModel()->BuildOctree();
	fieldGO->GetSkinnedMeshModel()->SetDrawBoundingBox(false);
	//fieldGO->SetCastShadow(false);
	skinnedMeshGroundGO.push_back(fieldGO);

	//GameObject<SkinnedMeshModelInstance>* bonfireGO = new GameObject<SkinnedMeshModelInstance>();
	//bonfireGO->Instantiate(MODEL_ENVIRONMENT_PATH, MODEL_BONFIRE_NAME);
	//bonfireGO->SetScale(XMFLOAT3(BONFIRE_SIZE, BONFIRE_SIZE, BONFIRE_SIZE));
	//bonfireGO->SetPosition(XMFLOAT3(12582.0f, -2374.0f, -19485.0f));
	//bonfireGO->Update();
	//bonfireGO->SetCastShadow(true);
	//worldMatrix = bonfireGO->GetWorldMatrix();
	//BOUNDING_BOX boundingBox = bonfireGO->GetCollider().bbox;
	//bonfireGO->GetSkinnedMeshModel()->BuildTrianglesByBoundingBox(boundingBox);
	//bonfireGO->GetSkinnedMeshModel()->BuildOctree();
	//bonfireGO->GetSkinnedMeshModel()->SetDrawBoundingBox(false);
	//skinnedMeshGroundGO.push_back(bonfireGO);

	//GameObject<ModelInstance>* banyanGO = new GameObject<ModelInstance>();
	//banyanGO->Instantiate(MODEL_BANYAN_PATH);
	//banyanGO->SetPosition(XMFLOAT3(-12280.0f, -2254.0f, -4650.0f));
	//banyanGO->SetScale(XMFLOAT3(50.0f, 50.0f, 50.0f));
	//groundGO.push_back(banyanGO);

	//GameObject<ModelInstance>* banyanGO = new GameObject<ModelInstance>();
	//banyanGO->Instantiate("data/MODEL/Environment/Bush/Bush_2.obj");
	//banyanGO->SetPosition(XMFLOAT3(12582.0f, -2374.0f, -19485.0f));
	//banyanGO->SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
	//groundGO.push_back(banyanGO);

	InstanceParams params;
	params.type = EnvironmentObjectType::Bush_2;
	params.transformArray.push_back(InstanceTransformInfo(
		12582.0f,
		-19485.0f,
		0.0f,
		1.0f
	));

	//environment = new Environment();
	//environment->GenerateInstanceByParams(params, fieldGO->GetSkinnedMeshModel());
	//environment->GenerateRandomInstances(EnvironmentObjectType::Shrubbery_1, fieldGO->GetSkinnedMeshModel(), 15);
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

		//if (skinnedMeshGroundGO[i]->GetCollider().type == ColliderType::TREE)
		//{
		//	if (skinnedMeshGroundGO[i]->GetAttributes().isGrounded == false)
		//	{
		//		Transform transform = skinnedMeshGroundGO[i]->GetTransform();
		//		transform.pos.y -= 25;
		//		skinnedMeshGroundGO[i]->SetTransform(transform);
		//	}
		//}
	}

	if (town)
		town->Update();

	if (environment)
		environment->Update();
}

//=============================================================================
// 描画処理
//=============================================================================
void Ground::Draw(void)
{
	renderer.SetLightModeBuffer(1);

	//if (town)
	//	town->Draw();

	if (renderer.GetRenderMode() == RenderMode::INSTANCE
		|| renderer.GetRenderMode() == RenderMode::INSTANCE_SHADOW)
	{
		if (environment)
			environment->Draw();
	}
	else if (renderer.GetRenderMode() == RenderMode::OBJ)
	{
		UINT size = groundGO.getSize();
		for (UINT i = 0; i < size; i++)
		{
			if (groundGO[i]->GetUse() == false) continue;

			groundGO[i]->Draw();
		}
	}
	else if (renderer.GetRenderMode() == RenderMode::OBJ_SHADOW)
	{
		UINT size = groundGO.getSize();
		for (UINT i = 0; i < size; i++)
		{
			if (groundGO[i]->GetUse() == false || groundGO[i]->GetCastShadow() == false) continue;

			groundGO[i]->Draw();
		}
	}
	else if (renderer.GetRenderMode() == RenderMode::SKINNED_MESH)
	{
		UINT size = skinnedMeshGroundGO.getSize();
		for (UINT i = 0; i < size; i++)
		{
			if (skinnedMeshGroundGO[i]->GetUse() == false) 
				continue;

			skinnedMeshGroundGO[i]->Draw();
		}
	}
	else if (renderer.GetRenderMode() == RenderMode::SKINNED_MESH_SHADOW)
	{
		UINT size = skinnedMeshGroundGO.getSize();
		for (UINT i = 0; i < size; i++)
		{
			if (skinnedMeshGroundGO[i]->GetUse() == false || skinnedMeshGroundGO[i]->GetCastShadow() == false) 
				continue;

			skinnedMeshGroundGO[i]->Draw();
		}
	}

	renderer.SetLightModeBuffer(0);
}

void Ground::CollectStaticShadowMeshes(SimpleArray<StaticRenderData>& outMeshes)
{
	//UINT size = groundGO.getSize();
	//for (UINT i = 0; i < size; ++i)
	//{
	//	auto* go = groundGO[i];
	//	if (!go->GetUse() || !go->GetCastShadow()) continue;

	//	go->CollectShadowMeshes(outMeshes);
	//}
}

