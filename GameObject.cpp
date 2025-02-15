#include "GameObject.h"

template <>
GameObject<SkinnedMeshModelInstance>::~GameObject()
{
	if (instance.pModel)
	{
		instance.load = false;
		SkinnedMeshModelPool* modelPool = SkinnedMeshModel::GetModel(instance.modelPath);
		if (modelPool)
		{
			modelPool->count--;
			if (modelPool->count == 0)
			{
				delete modelPool->pModel;
				Model::RemoveModel(instance.modelPath);
			}
		}
	}
}

template <>
void GameObject<SkinnedMeshModelInstance>::Instantiate(char* modelPath, char* modelName, 
	ModelType modelType, AnimationClipName clipName)
{
	instance.modelPath = modelPath;
	instance.modelName = modelName;
	instance.pModel = SkinnedMeshModel::StoreModel(modelPath, modelName, modelType, clipName);
	instance.load = true;
	instance.use = true;
}

