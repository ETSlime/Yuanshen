#include "SkinnedMeshModel.h"
#include "input.h"
#include "debugproc.h"


int ModelData::modelCnt = 0;

void SkinnedMeshModel::Update()
{
    PrintDebugProc("Cnt:%d\n", cnt);

    if (GetKeyboardPress(DIK_UP))
    {
        pos.y--;
    }
    if (GetKeyboardPress(DIK_DOWN))
    {
        pos.y++;
    }

    if (GetKeyboardPress(DIK_LEFT))
    {
        rot.y += 0.03f;
    }
    if (GetKeyboardPress(DIK_RIGHT))
    {
        rot.y -= 0.03f;
    }

    if (GetKeyboardPress(DIK_RETURN))
    {
        animationTime += 1539538600 * 0.55f;
    }

    if (GetKeyboardTrigger(DIK_SPACE))
    {
        cnt++;
    }

    if (GetKeyboardPress(DIK_RSHIFT))
    {
        animationTime -= 1539538600 * 0.55f;
    }

    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        FbxNode* armatureNode = modelData->armatureNode;

        
        if (armatureNode)
        {
            modelData->mModelGlobalScl.clear();
            modelData->mModelGlobalRot.clear();
            modelData->mModelTranslate.clear();

            modelData->mModelGlobalTrans.clear();
            modelData->mModelLocalTrans.clear();

            modelData->limbHashMap.clear();

            int curIdx = 0;
            int prevIdx = -1;

            UpdateLimbGlobalTransform(armatureNode, curIdx, prevIdx, animationTime, modelData);
            UpdateBoneTransform(modelData);
        }
    }
}

void SkinnedMeshModel::Draw()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	renderer.SetCurrentWorldMatrix(&mtxWorld);

    for (auto& it : meshDataMap)
    {
        ModelData* modelData = it.value;
        FbxNode* armatureNode = modelData->armatureNode;

        if (armatureNode)
        {
            XMMATRIX	boneMatrices[BONE_MAX];
            int size = modelData->mBoneFinalTransforms.getSize();
            for (int i = 0; i < size; i++)
            {
                boneMatrices[i] = XMMatrixTranspose(XMLoadFloat4x4(&modelData->mBoneFinalTransforms[i]));
            }
            renderer.SetBoneMatrix(boneMatrices);
        }
        else
        {
            XMMATRIX	boneMatrices[BONE_MAX];
            boneMatrices[0] = XMMatrixTranspose(XMMatrixIdentity());
            renderer.SetBoneMatrix(boneMatrices);
        }

        // 頂点バッファ設定
        UINT stride = sizeof(SKINNED_VERTEX_3D);
        UINT offset = 0;
        renderer.GetDeviceContext()->IASetVertexBuffers(0, 1, &modelData->VertexBuffer, &stride, &offset);

        // インデックスバッファ設定
        renderer.GetDeviceContext()->IASetIndexBuffer(modelData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // プリミティブトポロジ設定
        renderer.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        MATERIAL material;
        ZeroMemory(&material, sizeof(material));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        if (modelData->diffuseTexture)
            material.noTexSampling = FALSE;
        else
            material.noTexSampling = TRUE;
        renderer.SetMaterial(material);

        renderer.SetFillMode(D3D11_FILL_WIREFRAME);
        renderer.SetCullingMode(CULL_MODE_NONE);
        // ポリゴン描画
        int IndexNum, StartIndexLocation;

        renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
        CalculateDrawParameters(modelData, 0, 0.001894, IndexNum, StartIndexLocation);
        renderer.GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

        renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
        CalculateDrawParameters(modelData, 0.001894, 0.00715, IndexNum, StartIndexLocation);
        renderer.GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

        renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
        CalculateDrawParameters(modelData, 0.00715, 0.047, IndexNum, StartIndexLocation);
        renderer.GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

        renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &hairDiffuseTexture);
        CalculateDrawParameters(modelData, 0.047, 0.44, IndexNum, StartIndexLocation);
        renderer.GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

        renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &bodyDiffuseTexture);
        CalculateDrawParameters(modelData, 0.44, 0.916, IndexNum, StartIndexLocation);
        renderer.GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

        renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &faceDiffuseTexture);
        CalculateDrawParameters(modelData, 0.916, 1, IndexNum, StartIndexLocation);
        renderer.GetDeviceContext()->DrawIndexed(IndexNum, StartIndexLocation, 0);

        // カリング設定を戻す
        renderer.SetCullingMode(CULL_MODE_BACK);
    }
}


void SkinnedMeshModel::CalculateDrawParameters(ModelData* modelData, float startPercentage, float endPercentage, int& IndexNum, int& StartIndexLocation)
{

    StartIndexLocation = static_cast<int>(floor(startPercentage * modelData->IndexNum));
    StartIndexLocation = (StartIndexLocation / 3) * 3;

    int EndIndexLocation = static_cast<int>(ceil(endPercentage * modelData->IndexNum));
    EndIndexLocation = (EndIndexLocation / 3) * 3;

    IndexNum = EndIndexLocation - StartIndexLocation;
}

void SkinnedMeshModel::SetBodyDiffuseTexture(TextureMgr& texMgr, char* texturePath)
{
    bodyDiffuseTexture = texMgr.CreateTexture(texturePath);
}

void SkinnedMeshModel::SetHairDiffuseTexture(TextureMgr& texMgr, char* texturePath)
{
    hairDiffuseTexture = texMgr.CreateTexture(texturePath);
}

void SkinnedMeshModel::SetFaceDiffuseTexture(TextureMgr& texMgr, char* texturePath)
{
    faceDiffuseTexture = texMgr.CreateTexture(texturePath);
}

SkinnedMeshModel::SkinnedMeshModel()
{
	FbxNode* head = new FbxNode(0);
	fbxNodes.insert(0, head);
    animationTime = 0;
    armatureNode = nullptr;

    pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
    scl = XMFLOAT3(1.f, 1.f, 1.f);
}

SkinnedMeshModel::~SkinnedMeshModel()
{
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

void SkinnedMeshModel::UpdateLimbGlobalTransform(FbxNode* node, int& curIdx, int prevIdx, uint64_t time, ModelData* modelData)
{
    ModelProperty* modelProperty = static_cast<ModelProperty*>(node->nodeData);

    if (modelProperty)
    {
        modelProperty->modelIdx = curIdx;
        //modelData->mModelHierarchy.push_back(prevIdx);// [curIdx] = prevIdx;
        modelData->limbHashMap.insert(node->nodeID, curIdx);
        XMMATRIX mtxLocalScl, mtxLocalRot, mtxLocalTranslate, mtxLcl, mtxGlobalTrans;

        mtxLcl = XMMatrixIdentity();

        LimbNodeAnimation* limbNodeAnimation = static_cast<LimbNodeAnimation*>(node->limbNodeAnimation);
        if (limbNodeAnimation)
        {
            FbxNode** animationCurveNodeScl = fbxNodes.search(limbNodeAnimation->LclScl);
            XMFLOAT3 animationValueScl = GetAnimationValue(animationCurveNodeScl, modelProperty->Scaling, time);
            mtxLocalScl = XMMatrixScaling(animationValueScl.x, animationValueScl.y, animationValueScl.z);

            FbxNode** animationCurveNodeRot = fbxNodes.search(limbNodeAnimation->LclRot);
            XMFLOAT3 animationValueRot = GetAnimationValue(animationCurveNodeRot, modelProperty->Rotation, time);
            mtxLocalRot = CreateRotationMatrix(XMConvertToRadians(animationValueRot.x), XMConvertToRadians(animationValueRot.y), XMConvertToRadians(animationValueRot.z));

            FbxNode** animationCurveNodeTranslation = fbxNodes.search(limbNodeAnimation->LclTranslation);
            XMFLOAT3 animationValueTranslation = GetAnimationValue(animationCurveNodeTranslation, modelProperty->Translation, time);
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
            mtxParentScl = XMLoadFloat4x4(&modelData->mModelGlobalScl[prevIdx]);
            parentTranslate = XMLoadFloat4x4(&modelData->mModelTranslate[prevIdx]);
            mtxParentRot = XMLoadFloat4x4(&modelData->mModelGlobalRot[prevIdx]);
            parentTrans = XMLoadFloat4x4(&modelData->mModelGlobalTrans[prevIdx]);
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
        modelData->mModelGlobalScl.push_back(globalScl);
        modelData->mModelGlobalRot.push_back(globalRot);
        modelData->mModelTranslate.push_back(globalTranslate);

        modelData->mModelGlobalTrans.push_back(globalTrans);
        modelData->mModelLocalTrans.push_back(localTrans);

        SimpleArray<FbxNode*> childNodes = node->childNodes;
        for (int i = 0; i < childNodes.getSize(); i++)
        {
            if (childNodes[i]->nodeType == FbxNodeType::LimbNode)
            {
                int prev = modelProperty->modelIdx;
                curIdx++;
                UpdateLimbGlobalTransform(node->childNodes[i], curIdx, prev, time, modelData);
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
                        UpdateLimbGlobalTransform(node->childNodes[i]->childNodes[j], curIdx, prev, time, modelData);
                    }
                }
            }

        }
    }
}

void SkinnedMeshModel::UpdateBoneTransform(ModelData* modelData)
{

    int numBones = modelData->mBoneHierarchy.getSize();

    if (numBones > 0)
    {
        modelData->mBoneFinalTransforms.clear();
        modelData->mBoneFinalTransforms.resize(numBones);

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

            pDeformerID = modelData->deformerHashMap.search(i);
            if (pDeformerID == nullptr) continue;

            pDeformerIdx = modelData->deformerIdxHashMap.search(*pDeformerID);
            if (pDeformerIdx)
                deformerIdx = *pDeformerIdx;

            pLimbID = modelData->deformerToLimb.search(*pDeformerID);
            if (pLimbID == nullptr)
                limbIdx = i;
            else
            {
                pLimbIdx = modelData->limbHashMap.search(*pLimbID);
                if (pLimbIdx == nullptr) continue;
                limbIdx = *pLimbIdx;
            }



            XMMATRIX mtxInverseRootTransform;

            XMMATRIX mtxGlobalTrans = XMLoadFloat4x4(&modelData->mModelGlobalTrans[limbIdx]);
            XMMATRIX mtxLocalTrans = XMLoadFloat4x4(&modelData->mModelLocalTrans[limbIdx]);

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
                XMMATRIX rootTransform = XMLoadFloat4x4(&modelData->mBoneFinalTransforms[0]);
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

            if (i < cnt)
                mtxTrans = newBindPose * mtxGlobalTrans;
            else
                mtxTrans = XMMatrixIdentity();

            mtxTrans = newBindPose * mtxGlobalTrans;


            float* pMtxTrans = reinterpret_cast<float*>(&mtxTrans);

            for (int i = 0; i < 16; i++)
            {
                if (fabs(pMtxTrans[i]) < SMALL_NUM_THRESHOLD)
                {
                    pMtxTrans[i] = 0.0f;
                }
            }

            if (i == cnt - 1 && pLimbID)
            {
                //PrintDebugProc("%d\n", *pLimbID);
                PrintDebugProc("%f, %f, %f, %f \n", pMtxTrans[0],
                    pMtxTrans[1], pMtxTrans[2], pMtxTrans[3]);
                PrintDebugProc("%f, %f, %f, %f \n", mtxTrans.r[1].m128_f32[0],
                    mtxTrans.r[1].m128_f32[1], mtxTrans.r[1].m128_f32[2], mtxTrans.r[1].m128_f32[3]);
                PrintDebugProc("%f, %f, %f, %f \n", mtxTrans.r[2].m128_f32[0],
                    mtxTrans.r[2].m128_f32[1], mtxTrans.r[2].m128_f32[2], mtxTrans.r[2].m128_f32[3]);
                PrintDebugProc("%f, %f, %f, %f \n", mtxTrans.r[3].m128_f32[0],
                    mtxTrans.r[3].m128_f32[1], mtxTrans.r[3].m128_f32[2], mtxTrans.r[3].m128_f32[3]);
            }



            XMFLOAT4X4 finalTransform;
            XMStoreFloat4x4(&finalTransform, mtxTrans);
            modelData->mBoneFinalTransforms.push_back(finalTransform);

        }

    }
}

XMFLOAT3 SkinnedMeshModel::GetAnimationValue(FbxNode** ppAnimationCurve, XMFLOAT3 defaultValue, uint64_t time)
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
			animationValue.x = GetAnimationCurveValue(animationCurveX, time, defaultValue.x);
			animationValue.y = GetAnimationCurveValue(animationCurveY, time, defaultValue.y);
			animationValue.z = GetAnimationCurveValue(animationCurveZ, time, defaultValue.z);
			return animationValue;
		}
		else
			return defaultValue;

	}
	else
		return defaultValue;
}

float SkinnedMeshModel::GetAnimationCurveValue(FbxNode** ppAnimationCurveNode, uint64_t time, float defaultValue)
{
	if (ppAnimationCurveNode)
	{
		AnimationCurve* animationCurve = static_cast<AnimationCurve*>((*ppAnimationCurveNode)->nodeData);
		if (animationCurve)
		{
			for (int i = 0; i < animationCurve->KeyAttrRefCount; i++)
			{
                if (time >= animationCurve->KeyTime[i] && time <= animationCurve->KeyTime[i + 1])
                {
                    float lerpPercent = (time - animationCurve->KeyTime[i]) / (animationCurve->KeyTime[i + 1] - animationCurve->KeyTime[i]);


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

