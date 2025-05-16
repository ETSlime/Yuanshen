//=============================================================================
//
// Ground���� [Ground.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "Renderer.h"
#include "Model.h"
#include "InputManager.h"
#include "Debugproc.h"
#include "Ground.h"
#include "CollisionManager.h"
#include "EffectSystem.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************

#define MAX_TREE			(0)

#define TREE_SIZE			(350.0f)
#define FIELD_SIZE			(46850.0f)
#define BONFIRE_SIZE		(12.0f)

#define PLAYER_INIT_POS		XMFLOAT3(12582.0f, -2111.0f, -19485.0f)

//=============================================================================
// ����������
//=============================================================================
Ground::Ground()
{
	Transform trans;
	trans.pos = XMFLOAT3(12937.0f, -2384.0f, -19485.0f);

	//for (int i = 0; i < MAX_TREE; i++)
	//{
	//	float radius = 20000.f;
	//	float r = GetRandFloat(2000.0f, radius);
	//	float angle = GetRandFloat(0.0f, 2.0f * 3.14159265359f);
	//	float x = trans.pos.x + r * cos(angle);
	//	float z = trans.pos.z + r * sin(angle);
	//	float size = GetRandFloat(TREE_SIZE * .8f, TREE_SIZE * 2.5f);
	//	GameObject<SkinnedMeshModelInstance>* treeGO = new GameObject<SkinnedMeshModelInstance>();
	//	treeGO->Instantiate(MODEL_ENVIRONMENT_PATH, MODEL_TREE_NAME, ModelType::Tree);
	//	treeGO->GetInstance()->pModel->SetBodyDiffuseTexture("data/MODEL/Environment/Bark.jpg");
	//	treeGO->SetPosition(XMFLOAT3(x, trans.pos.y, z));
	//	treeGO->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	//	treeGO->SetScale(XMFLOAT3(size, size, size));
	//	
	//	treeGO->GetSkinnedMeshModel()->SetBoundingBoxSize(XMFLOAT3(0.1f, 1.0f, 0.1f));
	//	treeGO->GetSkinnedMeshModel()->SetBoundingBoxLocationOffset(XMFLOAT3(-0.55f, 0.0f, 0.2f));
	//	treeGO->SetColliderOwner(treeGO);
	//	treeGO->SetColliderEnable(true);
	//	treeGO->SetColliderType(ColliderType::TREE);
	//	CollisionManager::get_instance().RegisterDynamicCollider(&treeGO->GetCollider());

	//	skinnedMeshGroundGO.push_back(treeGO);
	//}

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
	fieldGO->SetCastShadow(false);
	skinnedMeshGroundGO.push_back(fieldGO);

	//GameObject<ModelInstance>* fieldGO = new GameObject<ModelInstance>();
	//fieldGO->Instantiate(MODEL_FIELD_PATH);
	//fieldGO->SetScale(XMFLOAT3(FIELD_SIZE, FIELD_SIZE, FIELD_SIZE));
	//fieldGO->SetRotation(XMFLOAT3(0, 0, 0.0f));
	//fieldGO->SetPosition(XMFLOAT3(0.0f, -650.0f, 0.0f));
	//fieldGO->Update();
	//worldMatrix = fieldGO->GetWorldMatrix();
	//fieldGO->GetModel()->BuildTrianglesByWorldMatrix(worldMatrix, true);
	//fieldGO->GetModel()->BuildOctree();
	//fieldGO->GetModel()->SetDrawBoundingBox(false);
	//fieldGO->SetCastShadow(false);
	//groundGO.push_back(fieldGO);

	GameObject<ModelInstance>* bonfireGO = new GameObject<ModelInstance>();
	bonfireGO->Instantiate(MODEL_BONFIRE_PATH);
	bonfireGO->SetScale(XMFLOAT3(BONFIRE_SIZE, BONFIRE_SIZE, BONFIRE_SIZE));
	bonfireGO->SetPosition(XMFLOAT3(12582.0f, -2051.0f, -19485.0f));
	bonfireGO->Update();
	bonfireGO->SetCastShadow(true);
	worldMatrix = bonfireGO->GetWorldMatrix();
	BOUNDING_BOX boundingBox = bonfireGO->GetCollider().bbox;
	bonfireGO->GetModel()->BuildTrianglesByBoundingBox(boundingBox);
	bonfireGO->GetModel()->BuildOctree();
	bonfireGO->GetModel()->SetDrawBoundingBox(true);
	groundGO.push_back(bonfireGO);

	ParticleEffectParams params;
	params.type = EffectType::Smoke;
	params.position = XMFLOAT3(12282.0f, -1990.0f, -19485.0f);
	params.scale = 250.0f;
	params.acceleration = XMFLOAT3(0.0f, 15.0f, 0.0f);
	params.spawnRateMin = 2.0f;
	params.spawnRateMax = 5.0f;
	params.lifeMin = 3.0f;
	params.lifeMax = 6.0f;
	params.startColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 0.3f); // ���F
	EffectSystem::get_instance().SpawnParticleEffect(params);

	FireBallEffectParams fireparams;
	fireparams.type = EffectType::FireBall;
	fireparams.position = XMFLOAT3(11882.0f, -1990.0f, -19485.0f);
	fireparams.scale = 43.0f;
	fireparams.lifeMin = 1.0f;
	fireparams.lifeMax = 1.0f;
	fireparams.spawnRateMin = 0.0f;
	fireparams.spawnRateMax = 10.0f;
	fireparams.acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);
	fireparams.startColor = XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f);
	fireparams.endColor = XMFLOAT4(1.2f, 0.5f, 0.0f, 0.0f);
	fireparams.tilesX = 7;
	fireparams.tilesY = 7;
	fireparams.coneAngleDegree = 25.0f;
	fireparams.coneRadius = 0.6f;
	fireparams.coneLength = 5.0f;
	fireparams.frameLerpCurve = 1.0f;
	fireparams.rotationSpeed = 0.0f;
	fireparams.startSpeedMin = 2.0f;
	fireparams.startSpeedMax = 3.0f;
	EffectSystem::get_instance().SpawnParticleEffect(fireparams);
	
	//GameObject<ModelInstance>* banyanGO = new GameObject<ModelInstance>();
	//banyanGO->Instantiate(MODEL_BANYAN_PATH);
	//banyanGO->SetPosition(XMFLOAT3(-12280.0f, -2254.0f, -4650.0f));
	//banyanGO->SetScale(XMFLOAT3(50.0f, 50.0f, 50.0f));
	//groundGO.push_back(banyanGO);

	InstanceParams instanceParams;
	instanceParams.type = EnvironmentObjectType::Bush_2;
	instanceParams.transformArray.push_back(InstanceTransformInfo(
		12882.0f,
		-18385.0f,
		0.5 * XM_PI,
		0.3f
	));

	environment = new Environment();
	environment->GenerateInstanceByParams(instanceParams, fieldGO->GetSkinnedMeshModel());
	//environment->GenerateRandomInstances(EnvironmentObjectType::Shrubbery_1, fieldGO->GetSkinnedMeshModel(), 15);
}

//=============================================================================
// �I������
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
// �X�V����
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
// �`�揈��
//=============================================================================
void Ground::Draw(void)
{
	renderer.SetLightModeBuffer(1);

	if (town)
		town->Draw();

	if (renderer.GetRenderMode() == RenderMode::INSTANCE)
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

