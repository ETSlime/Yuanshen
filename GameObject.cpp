//=============================================================================
//
// GameObject処理 [GameObject.cpp]
// Author : 
//
//=============================================================================
#include "GameObject.h"

template <>
GameObject<SkinnedMeshModelInstance>::~GameObject()
{
	if (instance.pModel)
	{
		instance.load = false;
		SkinnedMeshModelPool* modelPool = SkinnedMeshModel::GetModel(instance.modelFullPath);
		if (modelPool)
		{
			modelPool->count--;
			if (modelPool->count == 0)
			{
				delete modelPool->pModel;
				Model::RemoveModel(instance.modelFullPath);
			}
		}
	}

	Scene::get_instance().UnregisterGameObject(this);
}

template <>
void GameObject<SkinnedMeshModelInstance>::Instantiate(char* modelPath, char* modelName, 
	ModelType modelType, AnimClipName clipName)
{
	char* modelFullPath = new char[MODEL_NAME_LENGTH] {};

	strcat(modelFullPath, modelPath);

	size_t modelPathLength = strlen(modelPath);
	if (modelPathLength > 0 && modelPath[modelPathLength - 1] != '/' && modelPath[modelPathLength - 1] != '\\')
	{
		strcat(modelFullPath, "/");
	}

	strcat(modelFullPath, modelName);

	instance.modelFullPath = modelFullPath;
	instance.modelPath = modelPath;
	instance.modelName = modelName;
	instance.pModel = SkinnedMeshModel::StoreModel(modelPath, modelName, modelFullPath, modelType, clipName);
	if (instance.pModel)
	{
		instance.load = true;
		instance.use = true;
	}
}

template<>
void GameObject<SkinnedMeshModelInstance>::Update()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(instance.transform.scl.x, instance.transform.scl.y, instance.transform.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(instance.transform.rot.x, instance.transform.rot.y + XM_PI, instance.transform.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	instance.transform.mtxWorld = mtxWorld;

	XMMATRIX boneTransform = XMMatrixIdentity();// instance.pModel->GetBoneFinalTransform();

	XMFLOAT3 worldPos1, worldPos2;

	// ローカルAABBの最大頂点
	XMVECTOR localAABBMax = XMVectorSet(
		instance.pModel->GetBoundingBox().maxPoint.x,
		instance.pModel->GetBoundingBox().maxPoint.y,
		instance.pModel->GetBoundingBox().maxPoint.z,
		1.0f
	);

	// ローカルAABBの最小頂点
	XMVECTOR localAABBMin = XMVectorSet(
		instance.pModel->GetBoundingBox().minPoint.x,
		instance.pModel->GetBoundingBox().minPoint.y,
		instance.pModel->GetBoundingBox().minPoint.z,
		1.0f
	);

	// まずボーン変換を適用
	XMVECTOR skinnedAABBMax = XMVector3Transform(localAABBMax, boneTransform);
	XMVECTOR skinnedAABBMin = XMVector3Transform(localAABBMin, boneTransform);

	// 次にワールド変換を適用
	XMVECTOR worldPosMax = XMVector3Transform(skinnedAABBMax, mtxWorld);
	XMVECTOR worldPosMin = XMVector3Transform(skinnedAABBMin, mtxWorld);

	XMStoreFloat3(&worldPos1, worldPosMax);
	XMStoreFloat3(&worldPos2, worldPosMin);

	// コライダーのAABBを更新
	instance.collider.bbox.maxPoint = XMFLOAT3(
		max(worldPos1.x, worldPos2.x),
		max(worldPos1.y, worldPos2.y),
		max(worldPos1.z, worldPos2.z)
	);

	instance.collider.bbox.minPoint = XMFLOAT3(
		min(worldPos1.x, worldPos2.x),
		min(worldPos1.y, worldPos2.y),
		min(worldPos1.z, worldPos2.z)
	);
}