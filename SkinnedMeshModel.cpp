#include "SkinnedMeshModel.h"
#include "input.h"
#include "debugproc.h"
#include "AnimStateMachine.h"
#include "CollisionManager.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_MODEL_NUM		(30)

//*****************************************************************************
// グローバル変数
//*****************************************************************************
HashMap<char*, SkinnedMeshModelPool, CharPtrHash, CharPtrEquals> SkinnedMeshModel::modelHashMap(
    MAX_MODEL_NUM,
    CharPtrHash(),
    CharPtrEquals()
);

int ModelData::modelCnt = 0;

FBXLoader& fbxLoader = FBXLoader::get_instance();
TextureMgr& mTexMgr = TextureMgr::get_instance();


void SkinnedMeshModel::UpdateBoneTransform(SimpleArray<XMFLOAT4X4>* boneTransforms)
{

    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;

        if (boneTransforms)
        {
            modelData->boneTransformData->mBoneFinalTransforms.clear();



            int numBone = boneTransforms->getSize();
            for (int i = 0; i < numBone; i++)
            {
                modelData->boneTransformData->mBoneFinalTransforms.push_back((*boneTransforms)[i]);
            }

            //for (int i = 0; i < numBone - 1; i++)
            //{
            //    XMVECTOR maxPoint = XMVectorSet(
            //        modelData->boundingBoxes[i]->baseMaxPoint.x,
            //        modelData->boundingBoxes[i]->baseMaxPoint.y,
            //        modelData->boundingBoxes[i]->baseMaxPoint.z, 
            //        1.0f
            //    );

            //    XMVECTOR minPoint = XMVectorSet(
            //        modelData->boundingBoxes[i]->baseMinPoint.x,
            //        modelData->boundingBoxes[i]->baseMinPoint.y,
            //        modelData->boundingBoxes[i]->baseMinPoint.z, 
            //        1.0f
            //    );

            //    XMMATRIX worldMatrix = XMLoadFloat4x4(&modelData->boneTransformData->mModelGlobalTrans[i+1]);
            //    XMVECTOR translation = worldMatrix.r[3];
            //    XMVECTOR transformedMaxPoint = XMVectorAdd(maxPoint, translation);
            //    XMVECTOR transformedMinPoint = XMVectorAdd(minPoint, translation);

            //    XMStoreFloat3(&modelData->boundingBoxes[i]->maxPoint, transformedMaxPoint);
            //    XMStoreFloat3(&modelData->boundingBoxes[i]->minPoint, transformedMinPoint);

            //    CreateBoundingBoxVertex(modelData->boundingBoxes[i]);
            //}
        }
        else
        {
            ModelData* modelData = it.value;
            FbxNode* armatureNode = modelData->armatureNode;


            if (armatureNode)
            {
                modelData->boneTransformData->mModelGlobalScl.clear();
                modelData->boneTransformData->mModelGlobalRot.clear();
                modelData->boneTransformData->mModelTranslate.clear();

                modelData->boneTransformData->mModelGlobalTrans.clear();
                modelData->boneTransformData->mModelLocalTrans.clear();
                modelData->boneTransformData->mBoneFinalTransforms.clear();

                modelData->boneTransformData->limbHashMap.clear();

                int curIdx = 0;
                int prevIdx = -1;

                UpdateLimbGlobalTransform(currentAnimClip->armatureNode, armatureNode, curIdx, prevIdx, currentAnimClip->currentTime, modelData->boneTransformData, currentAnimClip->isLoop);
                GetBoneTransform(modelData->boneTransformData->mBoneFinalTransforms, modelData);
            }
        }
    }
}

void SkinnedMeshModel::DrawModel()
{

    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        FbxNode* armatureNode = modelData->armatureNode;

        if (armatureNode)
        {
            XMMATRIX	boneMatrices[BONE_MAX];
            int size = modelData->boneTransformData->mBoneFinalTransforms.getSize();
            for (int i = 0; i < size; i++)
            {
                boneMatrices[i] = XMMatrixTranspose(XMLoadFloat4x4(&modelData->boneTransformData->mBoneFinalTransforms[i]));
            }
            Renderer::get_instance().SetBoneMatrix(boneMatrices);
        }
        else
        {
            XMMATRIX	boneMatrices[BONE_MAX];
            boneMatrices[0] = XMMatrixTranspose(XMMatrixIdentity());
            Renderer::get_instance().SetBoneMatrix(boneMatrices);
        }

        // 頂点バッファ設定
        UINT stride = sizeof(SKINNED_VERTEX_3D);
        UINT offset = 0;
        Renderer::get_instance().GetDeviceContext()->IASetVertexBuffers(0, 1, &modelData->VertexBuffer, &stride, &offset);

        // インデックスバッファ設定
        Renderer::get_instance().GetDeviceContext()->IASetIndexBuffer(modelData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // プリミティブトポロジ設定
        Renderer::get_instance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        MATERIAL material;
        ZeroMemory(&material, sizeof(material));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        if (modelData->diffuseTexture)
            material.noTexSampling = FALSE;
        else
            material.noTexSampling = TRUE;
        if (modelType == ModelType::Town)
            material.noTexSampling = FALSE;

        Renderer::get_instance().SetMaterial(material);

        Renderer::get_instance().SetCullingMode(CULL_MODE_NONE);

        switch (modelType)
        {
        case ModelType::Sigewinne:
            DrawSigewinne(modelData);
            break;
        case ModelType::Klee:
            DrawKlee(modelData);
            break;
        case ModelType::Lumine:
            DrawLumine(modelData);
            break;
        case ModelType::Tree:
            DrawTree(modelData);
            break;
        case ModelType::Field:
            DrawField(modelData);
            break;
        case ModelType::Town:
            DrawTown(modelData);
            break;
        case ModelType::Weapon:
        default:
            Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &modelData->diffuseTexture);
            Renderer::get_instance().GetDeviceContext()->DrawIndexed(modelData->IndexNum, 0, 0);
            break;
        }

#ifdef _DEBUG
        if (drawBoundingBox)
            DrawBoundingBox(modelData);
#endif

        // カリング設定を戻す
        Renderer::get_instance().SetCullingMode(CULL_MODE_BACK);
    }
}

void SkinnedMeshModel::SetBodyDiffuseTexture(char* texturePath)
{
    bodyDiffuseTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::SetBodyLightMapTexture(char* texturePath)
{
    bodyLightMapTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::SetBodyNormalMapTexture(char* texturePath)
{
    bodyNormalMapTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::SetHairDiffuseTexture(char* texturePath)
{
    hairDiffuseTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::SetHairLightMapTexture(char* texturePath)
{
    hairLightMapTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::SetFaceDiffuseTexture(char* texturePath)
{
    faceDiffuseTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::SetFaceLightMapTexture(char* texturePath)
{
    faceLightMapTexture = TextureMgr::get_instance().CreateTexture(texturePath);
}

void SkinnedMeshModel::LoadTownTexture(void)
{
    Area_Mdcity_Lvy01_Diffuse = TextureMgr::get_instance().CreateTexture("data/MODEL/Environment/Knight/Area_Mdcity_Lvy01_Diffuse.png");
    Area_Mdbuild_Wall06_Diffuse = TextureMgr::get_instance().CreateTexture("data/MODEL/Environment/Knight/Area_Mdbuild_Wall06_Diffuse.png");
    Indoor_OutDoor_MDSkyBox = TextureMgr::get_instance().CreateTexture("data/MODEL/Environment/Knight/Indoor_OutDoor_MDSkyBox.png");
}

void SkinnedMeshModel::GetBoneTransformByAnim(FbxNode* currentClipArmatureNode, uint64_t currentClipTime, SimpleArray<XMFLOAT4X4>* boneFinalTransform)
{
    (*boneFinalTransform).clear();
    
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        FbxNode* armatureNode = modelData->armatureNode;


        if (armatureNode)
        {
            modelData->boneTransformData->mModelGlobalScl.clear();
            modelData->boneTransformData->mModelGlobalRot.clear();
            modelData->boneTransformData->mModelTranslate.clear();

            modelData->boneTransformData->mModelGlobalTrans.clear();
            modelData->boneTransformData->mModelLocalTrans.clear();
            modelData->boneTransformData->mBoneFinalTransforms.clear();

            modelData->boneTransformData->limbHashMap.clear();

            int curIdx = 0;
            int prevIdx = -1;

            UpdateLimbGlobalTransform(currentClipArmatureNode, armatureNode, curIdx, prevIdx, currentClipTime, modelData->boneTransformData);
            GetBoneTransform(*boneFinalTransform, modelData);
        }
    }
}

void SkinnedMeshModel::SetBoundingBoxLocationOffset(XMFLOAT3 offset, int boneIdx)
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        int numBoundigBoxes = modelData->boundingBoxes.getSize();
        if (boneIdx >= numBoundigBoxes)
            return;


        for (int i = 0; i < numBoundigBoxes; i++)
        {
            XMFLOAT3 size = XMFLOAT3(
                (modelData->boundingBoxes[i]->baseMaxPoint.x - modelData->boundingBoxes[i]->baseMinPoint.x),
                (modelData->boundingBoxes[i]->baseMaxPoint.y - modelData->boundingBoxes[i]->baseMinPoint.y),
                (modelData->boundingBoxes[i]->baseMaxPoint.z - modelData->boundingBoxes[i]->baseMinPoint.z)
            );

            XMFLOAT3 offsetVal = XMFLOAT3(
                size.x * offset.x,
                size.y * offset.y,
                size.z * offset.z
            );

            XMFLOAT3 newMaxPoint = XMFLOAT3(
                modelData->boundingBoxes[i]->baseMaxPoint.x + offsetVal.x,
                modelData->boundingBoxes[i]->baseMaxPoint.y + offsetVal.y,
                modelData->boundingBoxes[i]->baseMaxPoint.z + offsetVal.z
            );


            XMFLOAT3 newMinPoint = XMFLOAT3(
                modelData->boundingBoxes[i]->baseMinPoint.x + offsetVal.x,
                modelData->boundingBoxes[i]->baseMinPoint.y + offsetVal.y,
                modelData->boundingBoxes[i]->baseMinPoint.z + offsetVal.z
            );

            modelData->boundingBoxes[i]->maxPoint = newMaxPoint;
            modelData->boundingBoxes[i]->minPoint = newMinPoint;
            modelData->boundingBoxes[i]->baseMaxPoint = newMaxPoint;
            modelData->boundingBoxes[i]->baseMinPoint = newMinPoint;

            CreateBoundingBoxVertex(modelData->boundingBoxes[i]);
        }
    }
}

void SkinnedMeshModel::SetBoundingBoxSize(XMFLOAT3 size, int boneIdx)
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        int numBoundigBoxes = modelData->boundingBoxes.getSize();
        if (boneIdx >= numBoundigBoxes)
            return;

        
        for (int i = 0; i < numBoundigBoxes; i++)
        {
            XMFLOAT3 oldSize = XMFLOAT3(
                (modelData->boundingBoxes[i]->baseMaxPoint.x - modelData->boundingBoxes[i]->baseMinPoint.x),
                (modelData->boundingBoxes[i]->baseMaxPoint.y - modelData->boundingBoxes[i]->baseMinPoint.y),
                (modelData->boundingBoxes[i]->baseMaxPoint.z - modelData->boundingBoxes[i]->baseMinPoint.z));

            XMFLOAT3 newSize = XMFLOAT3(
                oldSize.x * size.x,
                oldSize.y * size.y,
                oldSize.z * size.z);

            XMFLOAT3 newMaxPoint = XMFLOAT3(
                modelData->boundingBoxes[i]->baseMaxPoint.x + (newSize.x - oldSize.x) / 2,
                modelData->boundingBoxes[i]->baseMaxPoint.y + (newSize.y - oldSize.y) / 2,
                modelData->boundingBoxes[i]->baseMaxPoint.z + (newSize.z - oldSize.z) / 2);

            XMFLOAT3 newMinPoint = XMFLOAT3(
                modelData->boundingBoxes[i]->baseMinPoint.x - (newSize.x - oldSize.x) / 2,
                modelData->boundingBoxes[i]->baseMinPoint.y - (newSize.y - oldSize.y) / 2,
                modelData->boundingBoxes[i]->baseMinPoint.z - (newSize.z - oldSize.z) / 2);

            modelData->boundingBoxes[i]->maxPoint = newMaxPoint;
            modelData->boundingBoxes[i]->minPoint = newMinPoint;
            modelData->boundingBoxes[i]->baseMaxPoint = newMaxPoint;
            modelData->boundingBoxes[i]->baseMinPoint = newMinPoint;

            CreateBoundingBoxVertex(modelData->boundingBoxes[i]);

            boundingBox.maxPoint = newMaxPoint;
            boundingBox.minPoint = newMinPoint;
        }
    }
}

void SkinnedMeshModel::BuildTrianglesByWorldMatrix(XMMATRIX worldMatrix)
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        
        XMFLOAT3 worldPos1, worldPos2;

        XMVECTOR localAABBMax = XMVectorSet(
            modelData->boundingBoxes[0]->baseMaxPoint.x,
            modelData->boundingBoxes[0]->baseMaxPoint.y,
            modelData->boundingBoxes[0]->baseMaxPoint.z,
            1.0f
        );

        XMVECTOR localAABBMin = XMVectorSet(
            modelData->boundingBoxes[0]->baseMinPoint.x,
            modelData->boundingBoxes[0]->baseMinPoint.y,
            modelData->boundingBoxes[0]->baseMinPoint.z,
            1.0f
        );

        XMMATRIX boneTransform = GetBoneFinalTransform();

        XMVECTOR skinnedAABBMax = XMVector3Transform(localAABBMax, boneTransform);
        XMVECTOR skinnedAABBMin = XMVector3Transform(localAABBMin, boneTransform);

        XMVECTOR worldPosMax = XMVector3Transform(skinnedAABBMax, worldMatrix);
        XMVECTOR worldPosMin = XMVector3Transform(skinnedAABBMin, worldMatrix);


        XMStoreFloat3(&worldPos1, worldPosMax);
        XMStoreFloat3(&worldPos2, worldPosMin);

        boundingBox.maxPoint = XMFLOAT3(
            max(worldPos1.x, worldPos2.x),
            max(worldPos1.y, worldPos2.y),
            max(worldPos1.z, worldPos2.z)
        );

        boundingBox.minPoint = XMFLOAT3(
            min(worldPos1.x, worldPos2.x),
            min(worldPos1.y, worldPos2.y),
            min(worldPos1.z, worldPos2.z)
        );

        

        int vertexNum = modelData->VertexNum;
        SimpleArray<XMFLOAT3> triangleVertices(3);
        for (int i = 0; i < vertexNum; i++)
        {
            XMFLOAT3 worldPos;

            XMVECTOR localPosVec = XMVectorSet(
                modelData->VertexArray[i].Position.x,
                modelData->VertexArray[i].Position.y,
                modelData->VertexArray[i].Position.z, 
                1.0f
            );

            XMVECTOR skinnedPosVec = XMVector3Transform(localPosVec, boneTransform);
            XMVECTOR worldPosVec = XMVector3Transform(skinnedPosVec, worldMatrix);
            XMStoreFloat3(&worldPos, worldPosVec);

            triangleVertices.push_back(worldPos);

            if ((i + 1) % 3 == 0)
            {
                Triangle* triangle = new Triangle(
                    triangleVertices[0],
                    triangleVertices[1],
                    triangleVertices[2]
                );


                modelData->triangles.push_back(triangle);
                triangleVertices.clear();

            }
        }

    }
}

bool SkinnedMeshModel::BuildOctree(void)
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;

        if (!modelData->triangles.getSize())
            return false;

        if (!modelData->boundingBoxes.getSize())
            return false;

        if (CollisionManager::get_instance().staticOctree == nullptr)
            CollisionManager::get_instance().staticOctree = new OctreeNode(boundingBox);

        int numTriangles = modelData->triangles.getSize();
        for (int i = 0; i < numTriangles; i++)
        {
            if (!CollisionManager::get_instance().staticOctree->insert(modelData->triangles[i]))
                return false;
        }
    }

    return true;
}

const SimpleArray<Triangle*>& SkinnedMeshModel::GetTriangles(void) const
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;

        return modelData->triangles;
    }

    return SimpleArray<Triangle*>();    
}

void SkinnedMeshModel::SetCurrentAnim(AnimationClipName clipName, float startTime)
{
    AnimationClip** animClip = animationClips.search(clipName);

    if (animClip)
    {
        currentAnimClip = *animClip;
        currentAnimClip->currentTime = (*animClip)->stopTime * startTime;
    }
}

AnimationClip* SkinnedMeshModel::GetAnimationClip(AnimationClipName clipName)
{
    AnimationClip** animClip = animationClips.search(clipName);
    if (animClip)
        return *animClip;

    return nullptr;
}

void SkinnedMeshModel::PlayCurrentAnim(float playSpeed)
{
    currentAnimClip->currentTime += ANIM_SPD * playSpeed;
}

XMMATRIX SkinnedMeshModel::GetWeaponTransformMtx(void)
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        XMFLOAT4X4 transform =  modelData->boneTransformData->mBoneFinalTransforms[weaponTransformIdx];

        return XMLoadFloat4x4(&transform);
    }

    return XMMatrixIdentity();
}

XMMATRIX SkinnedMeshModel::GetBoneFinalTransform(int boneIdx)
{
    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;

        if (boneIdx >= modelData->boneTransformData->mBoneFinalTransforms.getSize())
            return XMMatrixIdentity();

        XMFLOAT4X4 transform = modelData->boneTransformData->mBoneFinalTransforms[boneIdx];

        return XMLoadFloat4x4(&transform);
    }

    return XMMatrixIdentity();
}

SkinnedMeshModel::SkinnedMeshModel()
{
    armatureNode = nullptr;
    currentRootNodeID = 0;
    weaponTransformIdx = 0;
    currentAnimClip = nullptr;

    armatureNode = nullptr;

    bodyDiffuseTexture = nullptr;
    bodyLightMapTexture = nullptr;
    bodyNormalMapTexture = nullptr;
    hairDiffuseTexture = nullptr;
    hairLightMapTexture = nullptr;
    faceDiffuseTexture = nullptr;
    faceLightMapTexture = nullptr;

    drawBoundingBox = true;
}

SkinnedMeshModel::~SkinnedMeshModel()
{
    SafeRelease(&bodyDiffuseTexture);
    SafeRelease(&bodyLightMapTexture);
    SafeRelease(&bodyNormalMapTexture);
    SafeRelease(&hairDiffuseTexture);
    SafeRelease(&hairLightMapTexture);
    SafeRelease(&faceDiffuseTexture);
    SafeRelease(&faceLightMapTexture);

    for (auto& it : fbxNodes)
    {
        SAFE_DELETE(it.value);
    }

    for (auto& it : meshDataMap)
    {
        SAFE_DELETE(it.value);
    }

    for (auto& it : animationClips)
    {
        SAFE_DELETE(it.value);
    }
}

XMMATRIX CreateRotationMatrix(float pitch, float yaw, float roll)
{
    XMMATRIX Rz = XMMATRIX(
        cos(roll), -sin(roll), 0, 0,
        sin(roll), cos(roll), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    XMMATRIX Ry = XMMATRIX(
        cos(yaw), 0, sin(yaw), 0,
        0, 1, 0, 0,
        -sin(yaw), 0, cos(yaw), 0,
        0, 0, 0, 1
    );

    XMMATRIX Rx = XMMATRIX(
        1, 0, 0, 0,
        0, cos(pitch), -sin(pitch), 0,
        0, sin(pitch), cos(pitch), 0,
        0, 0, 0, 1
    );

    return XMMatrixTranspose(Rz * Ry * Rx);
}

SkinnedMeshModel* SkinnedMeshModel::StoreModel(char* modelPath, char* modelName, char* modelFullPath, 
    ModelType modelType, AnimationClipName clipName)
{
    SkinnedMeshModelPool* modelPool = GetModel(modelFullPath);
    if (modelPool == nullptr)
    {
        modelPool = new SkinnedMeshModelPool;
        modelPool->pModel = new SkinnedMeshModel();
        bool loadSuccess;
        loadSuccess = fbxLoader.LoadModel(Renderer::get_instance().GetDevice(), mTexMgr,
            *modelPool->pModel, modelPath, modelName,
            nullptr, clipName, modelType);
        if (!loadSuccess)
            return nullptr;
        modelPool->count = 1;
        modelHashMap.insert(modelFullPath, *modelPool);
    }
    else
        modelPool->count++;

    return modelPool->pModel;
}

SkinnedMeshModelPool* SkinnedMeshModel::GetModel(char* modelFullPath)
{
    return modelHashMap.search(modelFullPath);
}

void SkinnedMeshModel::RemoveModel(char* modelPath)
{
    modelHashMap.remove(modelPath);
}

void SkinnedMeshModel::UpdateLimbGlobalTransform(FbxNode* node, FbxNode* deformNode, 
    int& curIdx, int prevIdx, uint64_t time, BoneTransformData* boneTransformData, BOOL isLoop)
{
    ModelProperty* modelProperty = static_cast<ModelProperty*>(node->nodeData);

    if (modelProperty)
    {
        modelProperty->modelIdx = curIdx;
        boneTransformData->limbHashMap.insert(deformNode->nodeID, curIdx);
        XMMATRIX mtxLocalScl, mtxLocalRot, mtxLocalTranslate, mtxLcl, mtxGlobalTrans;

        mtxLcl = XMMatrixIdentity();

        LimbNodeAnimation* limbNodeAnimation = static_cast<LimbNodeAnimation*>(node->limbNodeAnimation);
        if (limbNodeAnimation)
        {
            FbxNode** animationCurveNodeScl = fbxNodes.search(limbNodeAnimation->LclScl);
            XMFLOAT3 animationValueScl = GetAnimationValue(animationCurveNodeScl, modelProperty->Scaling, time, isLoop);
            mtxLocalScl = XMMatrixScaling(animationValueScl.x, animationValueScl.y, animationValueScl.z);

            FbxNode** animationCurveNodeRot = fbxNodes.search(limbNodeAnimation->LclRot);
            XMFLOAT3 animationValueRot = GetAnimationValue(animationCurveNodeRot, modelProperty->Rotation, time, isLoop);
            mtxLocalRot = CreateRotationMatrix(XMConvertToRadians(animationValueRot.x), XMConvertToRadians(animationValueRot.y), XMConvertToRadians(animationValueRot.z));

            FbxNode** animationCurveNodeTranslation = fbxNodes.search(limbNodeAnimation->LclTranslation);
            XMFLOAT3 animationValueTranslation = GetAnimationValue(animationCurveNodeTranslation, modelProperty->Translation, time, isLoop);
            mtxLocalTranslate = XMMatrixTranslation(animationValueTranslation.x, animationValueTranslation.y, animationValueTranslation.z);

        }
        else
        {
            mtxLocalScl = XMMatrixScaling(modelProperty->Scaling.x, modelProperty->Scaling.y, modelProperty->Scaling.z);
            mtxLocalRot = CreateRotationMatrix(XMConvertToRadians(modelProperty->Rotation.x), XMConvertToRadians(modelProperty->Rotation.y), XMConvertToRadians(modelProperty->Rotation.z));
            mtxLocalTranslate = XMMatrixTranslation(modelProperty->Translation.x, modelProperty->Translation.y, modelProperty->Translation.z);
        }



        XMFLOAT4X4 localTrans;
        XMFLOAT4X4 globalTranslate, globalRot, globalScl, globalTrans;
        XMMATRIX parentTranslate, mtxParentRot, mtxParentScl, parentTrans, mtxGlobalTranslate;

        if (prevIdx == -1)
        {
            mtxParentScl = XMMatrixIdentity();
            parentTranslate = XMMatrixIdentity();
            mtxParentRot = XMMatrixIdentity();
            parentTrans = XMMatrixIdentity();

        }
        else
        {
            mtxParentScl = XMLoadFloat4x4(&boneTransformData->mModelGlobalScl[prevIdx]);
            parentTranslate = XMLoadFloat4x4(&boneTransformData->mModelTranslate[prevIdx]);
            mtxParentRot = XMLoadFloat4x4(&boneTransformData->mModelGlobalRot[prevIdx]);
            parentTrans = XMLoadFloat4x4(&boneTransformData->mModelGlobalTrans[prevIdx]);
        }

        mtxLcl = XMMatrixMultiply(mtxLcl, mtxLocalScl);
        mtxLcl = XMMatrixMultiply(mtxLcl, mtxLocalRot);
        mtxLcl = XMMatrixMultiply(mtxLcl, mtxLocalTranslate);

        XMStoreFloat4x4(&localTrans, mtxLcl);
        XMStoreFloat4x4(&globalScl, XMMatrixMultiply(mtxParentScl, mtxLocalScl));

        XMVECTOR localTranslationVec = XMVectorSetW(mtxLocalTranslate.r[3], 0.0f);
        XMVECTOR globalTranslationVec = XMVector3Transform(localTranslationVec, parentTrans);
        mtxGlobalTranslate = XMMatrixTranslationFromVector(globalTranslationVec);
        XMStoreFloat4x4(&globalTranslate, mtxGlobalTranslate);

        // eInheritRSrs: lGlobalRS = lParentGRM * lParentGSM * lLRM * lLSM;
        mtxGlobalTrans = mtxLocalScl * mtxLocalRot * mtxParentScl * mtxParentRot;
        // eInheritRrSs: lGlobalRS = lParentGRM * lLRM * lParentGSM * lLSM;
        //mtxGlobalTrans = mtxLocalScl * mtxParentScl * mtxLocalRot * mtxParentRot;
        mtxGlobalTrans = XMMatrixMultiply(mtxGlobalTrans, mtxGlobalTranslate);
        globalTranslationVec = XMVector3Transform(localTranslationVec, parentTrans);
        XMStoreFloat4x4(&globalTrans, mtxGlobalTrans);
        XMStoreFloat4x4(&globalRot, XMMatrixMultiply(mtxLocalRot, mtxParentRot));

        boneTransformData->mModelGlobalScl.push_back(globalScl);
        boneTransformData->mModelGlobalRot.push_back(globalRot);
        boneTransformData->mModelTranslate.push_back(globalTranslate);
        boneTransformData->mModelGlobalTrans.push_back(globalTrans);
        boneTransformData->mModelLocalTrans.push_back(localTrans);

        SimpleArray<FbxNode*> childNodes = node->childNodes;
        for (int i = 0; i < childNodes.getSize(); i++)
        {
            if (childNodes[i]->nodeType == FbxNodeType::LimbNode)
            {
                int prev = modelProperty->modelIdx;
                curIdx++;
                UpdateLimbGlobalTransform(node->childNodes[i], deformNode->childNodes[i], curIdx, prev, time, boneTransformData, isLoop);
            }
            else if (childNodes[i]->nodeType == FbxNodeType::Mesh)
            {
                SimpleArray<FbxNode*> limbNodes = childNodes[i]->childNodes;
                for (int j = 0; j < limbNodes.getSize(); j++)
                {
                    if (limbNodes[j]->nodeType == FbxNodeType::LimbNode)
                    {
                        int prev = modelProperty->modelIdx;
                        curIdx++;
                        UpdateLimbGlobalTransform(node->childNodes[i]->childNodes[j], deformNode->childNodes[i]->childNodes[j], curIdx, prev, time, boneTransformData, isLoop);
                    }
                }
            }
        }
    }
}

void SkinnedMeshModel::GetBoneTransform(SimpleArray<XMFLOAT4X4>& boneFinalTransform, ModelData* modelData)
{
    int numBones = modelData->mBoneHierarchy.getSize();

    if (numBones > 0)
    {
        if (boneFinalTransform.getSize() != numBones)
            boneFinalTransform.resize(numBones);

        SimpleArray<XMFLOAT4X4> toRootTransforms(numBones);
        toRootTransforms[0] = modelData->mBoneToParentTransforms[0];
        for (int i = 1; i < numBones; i++)
        {
            XMMATRIX toParent = XMLoadFloat4x4(&modelData->mBoneToParentTransforms[i]);
            int parentIndex = modelData->mBoneHierarchy[i];
            XMMATRIX parentToRoot;
            if (parentIndex == -1)
                parentToRoot = XMMatrixIdentity();
            else
                parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
            XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
            XMStoreFloat4x4(&toRootTransforms[i], toRoot);
        }

        for (int i = 0; i < numBones; ++i)
        {
            int limbIdx = 0;
            int deformerIdx = 0;
            int* pLimbIdx = nullptr;
            int* pDeformerIdx = nullptr;
            uint64_t* pDeformerID = nullptr;
            uint64_t* pLimbID = nullptr;

            pDeformerID = deformerHashMap.search(i);
            if (pDeformerID == nullptr) continue;

            pDeformerIdx = deformerIdxHashMap.search(*pDeformerID);
            if (pDeformerIdx)
                deformerIdx = *pDeformerIdx;

            pLimbID = deformerToLimb.search(*pDeformerID);
            if (pLimbID == nullptr)
                limbIdx = i;
            else
            {
                pLimbIdx = modelData->boneTransformData->limbHashMap.search(*pLimbID);
                if (pLimbIdx == nullptr) continue;
                limbIdx = *pLimbIdx;
            }



            XMMATRIX mtxInverseRootTransform;

            XMMATRIX mtxGlobalTrans = XMLoadFloat4x4(&modelData->boneTransformData->mModelGlobalTrans[limbIdx]);
            XMMATRIX mtxLocalTrans = XMLoadFloat4x4(&modelData->boneTransformData->mModelLocalTrans[limbIdx]);

            XMMATRIX offset = XMLoadFloat4x4(&modelData->mBoneOffsets[deformerIdx]);
            XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[deformerIdx]);
            XMMATRIX mtxTrans = XMMatrixIdentity();


            XMFLOAT4X4* pBindPose = nullptr;
            if (pLimbID)
                pBindPose = bindPose.mtxBindPoses.search(*pLimbID);
            XMMATRIX bindPose, inverseBindPose;
            if (pBindPose == nullptr)
            {
                bindPose = XMMatrixIdentity();
                inverseBindPose = XMMatrixIdentity();
            }
            else
            {
                bindPose = XMLoadFloat4x4(pBindPose);
                inverseBindPose = XMMatrixInverse(nullptr, XMLoadFloat4x4(pBindPose));
            }

            if (i > 0)
            {
                XMMATRIX rootTransform = XMLoadFloat4x4(&boneFinalTransform[0]);
                rootTransform.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
                mtxInverseRootTransform = XMMatrixInverse(nullptr, rootTransform);
            }
            else
            {
                mtxInverseRootTransform = XMMatrixIdentity();
            }



            XMMATRIX inverseTransformLink = XMMatrixInverse(nullptr, offset);
            XMMATRIX newBindPose = XMMatrixMultiply(inverseTransformLink, toRoot);
            XMMATRIX newBindPoseChange = newBindPose * bindPose;
            XMVECTOR row0 = newBindPoseChange.r[0];
            XMVECTOR row1 = newBindPoseChange.r[1];
            XMVECTOR row2 = newBindPoseChange.r[2];
            XMVECTOR row3 = newBindPoseChange.r[3];

            row0 = XMVectorSetZ(row0, -XMVectorGetZ(row0));
            row1 = XMVectorSetZ(row1, -XMVectorGetZ(row1));
            row2 = XMVectorSetX(row2, -XMVectorGetX(row2));
            row2 = XMVectorSetY(row2, -XMVectorGetY(row2));
            row2 = XMVectorSetW(row2, -XMVectorGetW(row2));
            row3 = XMVectorSetZ(row3, -XMVectorGetZ(row3));

            newBindPoseChange = XMMATRIX(row0, row1, row2, row3);
            newBindPose = newBindPoseChange;

            XMVECTOR bindPoseTranslationVec = XMVectorSetW(newBindPose.r[3], 0.0f);
            XMVECTOR inverseBindPoseTranslationVec = XMVector3Transform(bindPoseTranslationVec, mtxInverseRootTransform);
            newBindPose.r[3] = inverseBindPoseTranslationVec;

            if (pLimbID == nullptr)
            {
                mtxTrans = mtxLocalTrans * toRoot * offset;
            }
            else
                mtxTrans = XMMatrixMultiply(toRoot, offset);

            XMMATRIX mtxTransTmp = mtxGlobalTrans;
            row0 = mtxTransTmp.r[0];
            row1 = mtxTransTmp.r[1];
            row2 = mtxTransTmp.r[2];
            row3 = mtxTransTmp.r[3];

            if (i > 0)
            {
                row0 = XMVectorSetZ(row0, -XMVectorGetZ(row0));
                row1 = XMVectorSetZ(row1, -XMVectorGetZ(row1));
                row2 = XMVectorSetX(row2, -XMVectorGetX(row2));
                row2 = XMVectorSetY(row2, -XMVectorGetY(row2));
                row3 = XMVectorSetZ(row3, -XMVectorGetZ(row3));
            }
            mtxTransTmp = XMMATRIX(row0, row1, row2, row3);
            mtxGlobalTrans = mtxTransTmp;

            {

                float temp = newBindPose.r[3].m128_f32[1];
                newBindPose.r[3].m128_f32[1] = newBindPose.r[3].m128_f32[2];
                newBindPose.r[3].m128_f32[2] = temp;

                newBindPose.r[3].m128_f32[2] = -newBindPose.r[3].m128_f32[2];
            }

            mtxTrans = newBindPose * mtxGlobalTrans;


            float* pMtxTrans = reinterpret_cast<float*>(&mtxTrans);

            for (int i = 0; i < 16; i++)
            {
                if (fabs(pMtxTrans[i]) < SMALL_NUM_THRESHOLD)
                {
                    pMtxTrans[i] = 0.0f;
                }
            }

            //if (i == cnt - 1 && pLimbID)
            //{
            //    //PrintDebugProc("%d\n", *pLimbID);
            //    PrintDebugProc("%f, %f, %f, %f \n", pMtxTrans[0],
            //        pMtxTrans[1], pMtxTrans[2], pMtxTrans[3]);
            //    PrintDebugProc("%f, %f, %f, %f \n", mtxTrans.r[1].m128_f32[0],
            //        mtxTrans.r[1].m128_f32[1], mtxTrans.r[1].m128_f32[2], mtxTrans.r[1].m128_f32[3]);
            //    PrintDebugProc("%f, %f, %f, %f \n", mtxTrans.r[2].m128_f32[0],
            //        mtxTrans.r[2].m128_f32[1], mtxTrans.r[2].m128_f32[2], mtxTrans.r[2].m128_f32[3]);
            //    PrintDebugProc("%f, %f, %f, %f \n", mtxTrans.r[3].m128_f32[0],
            //        mtxTrans.r[3].m128_f32[1], mtxTrans.r[3].m128_f32[2], mtxTrans.r[3].m128_f32[3]);
            //}

            XMFLOAT4X4 finalTransform;
            XMStoreFloat4x4(&finalTransform, mtxTrans);
            boneFinalTransform.push_back(finalTransform);
        }

    }
}

XMFLOAT3 SkinnedMeshModel::GetAnimationValue(FbxNode** ppAnimationCurve, XMFLOAT3 defaultValue, uint64_t time, BOOL isLoop)
{
	if (ppAnimationCurve)
	{
		XMFLOAT3 animationValue;
		AnimationCurveNode* animationCurveNode = static_cast<AnimationCurveNode*>((*ppAnimationCurve)->nodeData);
		if (animationCurveNode)
		{
			FbxNode** animationCurveX = fbxNodes.search(animationCurveNode->dX);
			FbxNode** animationCurveY = fbxNodes.search(animationCurveNode->dY);
			FbxNode** animationCurveZ = fbxNodes.search(animationCurveNode->dZ);
			animationValue.x = GetAnimationCurveValue(animationCurveX, time, defaultValue.x, isLoop);
			animationValue.y = GetAnimationCurveValue(animationCurveY, time, defaultValue.y, isLoop);
			animationValue.z = GetAnimationCurveValue(animationCurveZ, time, defaultValue.z, isLoop);
			return animationValue;
		}
		else
			return defaultValue;

	}
	else
		return defaultValue;
}

float SkinnedMeshModel::GetAnimationCurveValue(FbxNode** ppAnimationCurveNode, uint64_t time, float defaultValue, BOOL isLoop)
{
	if (ppAnimationCurveNode)
	{
		AnimationCurve* animationCurve = static_cast<AnimationCurve*>((*ppAnimationCurveNode)->nodeData);
		if (animationCurve)
		{
            if (animationCurve->KeyAttrRefCount == 1)
                return animationCurve->KeyValue[0];
            else if (animationCurve->KeyAttrRefCount == 0)
                return 0;

            int keyCount = animationCurve->KeyAttrRefCount;
            uint64_t startTime = animationCurve->KeyTime[0];
            uint64_t endTime = animationCurve->KeyTime[keyCount - 1];

            if (time < startTime) 
            {
                time = endTime - fmod(startTime - time, endTime - startTime);
            }
            else if (time > endTime) 
            {
                time = fmod(time - startTime, endTime - startTime) + startTime;
            }

			for (int i = 0; i < animationCurve->KeyAttrRefCount; i++)
			{
                if (time >= animationCurve->KeyTime[i] && time <= animationCurve->KeyTime[i + 1])
                {
                    float lerpPercent = static_cast<float>(time - animationCurve->KeyTime[i]) / 
                        ((static_cast<float>(animationCurve->KeyTime[i + 1]) - static_cast<float>(animationCurve->KeyTime[i])));

                    float v0 = animationCurve->KeyValue[i];
                    float v1 = animationCurve->KeyValue[i + 1];

                    float v = v0 + (v1 - v0) * lerpPercent;
                    return v;
                }

			}
			return defaultValue;
		}
		else
			return defaultValue;
	}
	else
		return defaultValue;
}

void SkinnedMeshModel::CalculateDrawParameters(ModelData* modelData, float startPercentage, float endPercentage, int& IndexNum, int& StartIndexLocation)
{

    StartIndexLocation = static_cast<int>(floor(startPercentage * modelData->IndexNum));
    StartIndexLocation = (StartIndexLocation / 3) * 3;

    int EndIndexLocation = static_cast<int>(ceil(endPercentage * modelData->IndexNum));
    EndIndexLocation = (EndIndexLocation / 3) * 3;

    IndexNum = EndIndexLocation - StartIndexLocation;
}

void SkinnedMeshModel::DrawSigewinne(ModelData* modelData)
{
    // ポリゴン描画
    int IndexNum, StartIndexLocation;

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
    if (faceLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &faceLightMapTexture);
    CalculateDrawParameters(modelData, 0, 0.001894f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
    if (bodyLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &bodyLightMapTexture);
    CalculateDrawParameters(modelData, 0.001894f, 0.00715f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
    if (faceLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &faceLightMapTexture);
    CalculateDrawParameters(modelData, 0.00715f, 0.047f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &hairDiffuseTexture);
    if (hairLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &hairLightMapTexture);
    CalculateDrawParameters(modelData, 0.047f, 0.44f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
    if (bodyLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &bodyLightMapTexture);
    CalculateDrawParameters(modelData, 0.44f, 0.916f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
    if (faceLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &faceLightMapTexture);
    CalculateDrawParameters(modelData, 0.916f, 0.996f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);
}

void SkinnedMeshModel::DrawKlee(ModelData* modelData)
{
    // ポリゴン描画
    int IndexNum, StartIndexLocation;

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &hairDiffuseTexture);
    if (hairLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &hairLightMapTexture);
    CalculateDrawParameters(modelData, 0, 0.149f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
    if (bodyLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &bodyLightMapTexture);
    CalculateDrawParameters(modelData, 0.149f, 0.854f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
    if (faceLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &faceLightMapTexture);
    CalculateDrawParameters(modelData, 0.8628f, 0.9966f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);
}

void SkinnedMeshModel::DrawLumine(ModelData* modelData)
{
    // ポリゴン描画
    int IndexNum, StartIndexLocation;

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &hairDiffuseTexture);
    if (hairLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &hairLightMapTexture);
    CalculateDrawParameters(modelData, 0, 0.1323f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
    if (bodyLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &bodyLightMapTexture);
    CalculateDrawParameters(modelData, 0.1323f, 0.847f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
    if (faceLightMapTexture)
        Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(8, 1, &faceLightMapTexture);
    CalculateDrawParameters(modelData, 0.847f, 1.0f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);
}

void SkinnedMeshModel::DrawTree(ModelData* modelData)
{
    // ポリゴン描画
    int IndexNum, StartIndexLocation;

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
    CalculateDrawParameters(modelData, 0, 0.0423f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &modelData->diffuseTexture);
    CalculateDrawParameters(modelData, 0.0423f, 1.0f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);
}

void SkinnedMeshModel::DrawField(ModelData* modelData)
{
    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &modelData->diffuseTexture);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(modelData->IndexNum, 0, 0);
}

void SkinnedMeshModel::DrawTown(ModelData* modelData)
{
    int IndexNum, StartIndexLocation;
    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &Area_Mdbuild_Wall06_Diffuse);
    CalculateDrawParameters(modelData, 0, 0.190335f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

    Renderer::get_instance().SetLightModeBuffer(2);
    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &Indoor_OutDoor_MDSkyBox);
    CalculateDrawParameters(modelData, 0.190335f, 0.20087f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);
    Renderer::get_instance().SetLightModeBuffer(0);

    Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &Area_Mdbuild_Wall06_Diffuse);
    CalculateDrawParameters(modelData, 0.20087f, 1.0f, IndexNum, StartIndexLocation);
    Renderer::get_instance().GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);
}


void SkinnedMeshModel::DrawBoundingBox(ModelData* modelData)
{
    if (Renderer::get_instance().GetRenderMode() == RENDER_MODE_SHADOW) return;

    Renderer::get_instance().SetFillMode(D3D11_FILL_WIREFRAME);

    int numBoundingBox = modelData->boundingBoxes.getSize();
    for (int i = 0; i < numBoundingBox; i++)
    {

        // 頂点バッファ設定
        UINT stride = sizeof(SKINNED_VERTEX_3D);
        UINT offset = 0;
        Renderer::get_instance().GetDeviceContext()->IASetVertexBuffers(0, 1, &modelData->boundingBoxes[i]->BBVertexBuffer, &stride, &offset);

        // プリミティブトポロジ設定
        Renderer::get_instance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        MATERIAL material;
        ZeroMemory(&material, sizeof(material));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        material.noTexSampling = TRUE;
        Renderer::get_instance().SetMaterial(material);

        Renderer::get_instance().GetDeviceContext()->Draw(24, 0);
    }

    Renderer::get_instance().SetFillMode(D3D11_FILL_SOLID);
}

void SkinnedMeshModel::CreateBoundingBoxVertex(SKINNED_MESH_BOUNDING_BOX* boundingBox) const
{
    D3D11_MAPPED_SUBRESOURCE msr;
    Renderer::get_instance().GetDeviceContext()->Map(boundingBox->BBVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    SKINNED_VERTEX_3D* vertex = (SKINNED_VERTEX_3D*)msr.pData;
   
    for (int i = 0; i < 24; i++)
    {
        vertex[i].Weights = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
        vertex[i].BoneIndices = XMFLOAT4(static_cast<float>(boundingBox->boneIdx), 0.0f, 0.0f, 0.0f);
    }


    // 頂点座標の設定
    vertex[0].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->minPoint.y, boundingBox->minPoint.z);
    vertex[1].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->minPoint.z);
    vertex[2].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->minPoint.z);

    vertex[3].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->minPoint.z);
    vertex[4].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->maxPoint.y, boundingBox->minPoint.z);
    vertex[5].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->minPoint.z);

    vertex[6].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->minPoint.y, boundingBox->maxPoint.z);
    vertex[7].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->maxPoint.z);
    vertex[8].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->maxPoint.z);

    vertex[9].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->maxPoint.z);
    vertex[10].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->maxPoint.y, boundingBox->maxPoint.z);
    vertex[11].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->maxPoint.z);

    vertex[12].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->minPoint.z);
    vertex[13].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->maxPoint.y, boundingBox->minPoint.z);
    vertex[14].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->maxPoint.z);

    vertex[15].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->maxPoint.y, boundingBox->minPoint.z);
    vertex[16].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->maxPoint.y, boundingBox->maxPoint.z);
    vertex[17].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->maxPoint.y, boundingBox->maxPoint.z);

    vertex[18].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->minPoint.y, boundingBox->minPoint.z);
    vertex[19].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->minPoint.z);
    vertex[20].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->minPoint.y, boundingBox->maxPoint.z);

    vertex[21].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->minPoint.z);
    vertex[22].Position = XMFLOAT3(boundingBox->maxPoint.x, boundingBox->minPoint.y, boundingBox->maxPoint.z);
    vertex[23].Position = XMFLOAT3(boundingBox->minPoint.x, boundingBox->minPoint.y, boundingBox->maxPoint.z);


    // 法線の設定
    vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[4].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[5].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[6].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[7].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[8].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[9].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[10].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[11].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[12].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[13].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[14].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[15].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[16].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[17].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[18].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[19].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[20].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[21].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[22].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
    vertex[23].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    // 拡散光の設定
    vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[4].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[5].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[6].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[7].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[8].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[9].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[10].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[11].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[12].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[13].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[14].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[15].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[16].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[17].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[18].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[19].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[20].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[21].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[22].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[23].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // テクスチャ座標の設定
    vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
    vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
    vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
    vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
    vertex[4].TexCoord = XMFLOAT2(0.0f, 0.0f);
    vertex[5].TexCoord = XMFLOAT2(1.0f, 0.0f);
    vertex[6].TexCoord = XMFLOAT2(0.0f, 1.0f);
    vertex[7].TexCoord = XMFLOAT2(1.0f, 1.0f);
    vertex[8].TexCoord = XMFLOAT2(0.0f, 0.0f);
    vertex[9].TexCoord = XMFLOAT2(1.0f, 0.0f);
    vertex[10].TexCoord = XMFLOAT2(0.0f, 1.0f);
    vertex[11].TexCoord = XMFLOAT2(1.0f, 1.0f);
    vertex[12].TexCoord = XMFLOAT2(0.0f, 0.0f);
    vertex[13].TexCoord = XMFLOAT2(1.0f, 0.0f);
    vertex[14].TexCoord = XMFLOAT2(0.0f, 1.0f);
    vertex[15].TexCoord = XMFLOAT2(1.0f, 1.0f);
    vertex[16].TexCoord = XMFLOAT2(0.0f, 0.0f);
    vertex[17].TexCoord = XMFLOAT2(1.0f, 0.0f);
    vertex[18].TexCoord = XMFLOAT2(0.0f, 1.0f);
    vertex[19].TexCoord = XMFLOAT2(1.0f, 1.0f);
    vertex[20].TexCoord = XMFLOAT2(0.0f, 0.0f);
    vertex[21].TexCoord = XMFLOAT2(1.0f, 0.0f);
    vertex[22].TexCoord = XMFLOAT2(0.0f, 1.0f);
    vertex[23].TexCoord = XMFLOAT2(1.0f, 1.0f);

    Renderer::get_instance().GetDeviceContext()->Unmap(boundingBox->BBVertexBuffer, 0);
}