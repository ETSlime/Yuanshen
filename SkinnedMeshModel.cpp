#include "SkinnedMeshModel.h"
#include "input.h"
void SkinnedMeshModel::Update()
{
    int curIdx = 0;
    int prevIdx = -1;
    UpdateLimbGlobalTransform(armatureNode, curIdx, prevIdx, animationTime);
    UpdateBoneTransform();
    if (GetKeyboardPress(DIK_RETURN))
    {
        animationTime += 1539538600 * 0.5f;
    }

	XMMATRIX	boneMatrices[BONE_MAX];
	int size = mBoneFinalTransforms.getSize();
	for (int i = 0; i < size; i++)
	{
		boneMatrices[i] = XMMatrixTranspose(XMLoadFloat4x4(&mBoneFinalTransforms[i]));
		//boneMatrices[i] = XMLoadFloat4x4(&mBoneFinalTransforms[i]);
	}
	renderer.SetBoneMatrix(boneMatrices);
}

void SkinnedMeshModel::Draw()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(1.f, 1.f, 1.f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(1.0f, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	renderer.SetCurrentWorldMatrix(&mtxWorld);

	// 頂点バッファ設定
	UINT stride = sizeof(SKINNED_VERTEX_3D);
	UINT offset = 0;
	renderer.GetDeviceContext()->IASetVertexBuffers(0, 1, &this->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	renderer.GetDeviceContext()->IASetIndexBuffer(this->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// プリミティブトポロジ設定
	renderer.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &this->Texture);

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	if (this->Texture)
		material.noTexSampling = FALSE;
	else
		material.noTexSampling = TRUE;
	renderer.SetMaterial(material);

	renderer.SetFillMode(D3D11_FILL_WIREFRAME);
	renderer.SetCullingMode(CULL_MODE_NONE);
	// ポリゴン描画
	renderer.GetDeviceContext()->DrawIndexed(this->modelData->IndexNum, 0, 0);
	// カリング設定を戻す
	renderer.SetCullingMode(CULL_MODE_BACK);
}

SkinnedMeshModel::SkinnedMeshModel()
{
	FbxNode* head = new FbxNode(0);
	fbxNodes.insert(0, head);
    animationTime = 0;
    armatureNode = nullptr;
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

void SkinnedMeshModel::UpdateLimbGlobalTransform(FbxNode* node, int& curIdx, int prevIdx, uint64_t time)
{
    ModelProperty* modelProperty = static_cast<ModelProperty*>(node->nodeData);

    if (modelProperty)
    {
        modelProperty->modelIdx = curIdx;
        mModelHierarchy[curIdx] = prevIdx;
        limbHashMap.insert(node->nodeID, curIdx);
        XMMATRIX mtxLocalScl, mtxLocalRot, mtxLocalTranslate, mtxLcl, mtxGlobalTrans;
        XMMATRIX mtxLocalScl2, mtxLocalRot2, mtxLocalTranslate2;

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



        XMFLOAT4X4 localTranslate, localRot, localScl, localTrans;
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
            mtxParentScl = XMLoadFloat4x4(&mModelGlobalScl[prevIdx]);
            parentTranslate = XMLoadFloat4x4(&mModelTranslate[prevIdx]);
            mtxParentRot = XMLoadFloat4x4(&mModelGlobalRot[prevIdx]);
            parentTrans = XMLoadFloat4x4(&mModelGlobalTrans[prevIdx]);
        }

        mtxLcl = XMMatrixMultiply(mtxLcl, mtxLocalScl);
        mtxLcl = XMMatrixMultiply(mtxLcl, mtxLocalRot);
        mtxLcl = XMMatrixMultiply(mtxLcl, mtxLocalTranslate);

        XMStoreFloat4x4(&localScl, mtxLocalScl);
        XMStoreFloat4x4(&localTranslate, mtxLocalTranslate);
        XMStoreFloat4x4(&localRot, mtxLocalRot);
        XMStoreFloat4x4(&localTrans, mtxLcl);
        XMStoreFloat4x4(&globalScl, XMMatrixMultiply(mtxParentScl, mtxLocalScl));


        XMVECTOR scaleX = XMVector3Length(parentTrans.r[0]);
        XMVECTOR scaleY = XMVector3Length(parentTrans.r[1]);
        XMVECTOR scaleZ = XMVector3Length(parentTrans.r[2]);
        XMVECTOR scale = XMVectorSet(
            XMVectorGetX(scaleX),
            XMVectorGetY(scaleY),
            XMVectorGetZ(scaleZ),
            0.0f);

        XMVECTOR localTranslationVec = XMVectorSetW(mtxLocalTranslate.r[3], 0.0f);
        XMVECTOR globalTranslationVec = XMVector3Transform(localTranslationVec, parentTrans);
        mtxGlobalTranslate = XMMatrixTranslationFromVector(globalTranslationVec);
        XMStoreFloat4x4(&globalTranslate, mtxGlobalTranslate);

        mtxGlobalTrans = mtxLocalScl * mtxLocalRot * mtxParentScl * mtxParentRot;
        mtxGlobalTrans = XMMatrixMultiply(mtxGlobalTrans, mtxGlobalTranslate);
        globalTranslationVec = XMVector3Transform(localTranslationVec, parentTrans);
        XMStoreFloat4x4(&globalTrans, mtxGlobalTrans);

        XMStoreFloat4x4(&globalRot, XMMatrixMultiply(mtxLocalRot, mtxParentRot));
        mModelGlobalScl[curIdx] = globalScl;
        mModelLocalScl[curIdx] = localScl;
        mModelGlobalRot[curIdx] = globalRot;
        mModelLocalRot[curIdx] = localRot;
        mModelTranslate[curIdx] = globalTranslate;
        mModelLocalTranslate[curIdx] = localTranslate;

        mModelGlobalTrans[curIdx] = globalTrans;
        mModelLocalTrans[curIdx] = localTrans;

        for (int i = 0; i < node->childNodes.getSize(); i++)
        {
            if (node->childNodes[i]->nodeType == FbxNodeType::LimbNode)
            {
                int prev = modelProperty->modelIdx;
                curIdx++;
                UpdateLimbGlobalTransform(node->childNodes[i], curIdx, prev, time);
            }

        }
    }
}

void SkinnedMeshModel::UpdateBoneTransform()
{
    int numBones = mBoneHierarchy.getSize();

    if (numBones > 0)
    {
        mBoneFinalTransforms.resize(numBones);
        mBoneFinalTransforms.clear();
        SimpleArray<XMFLOAT4X4> toRootTransforms(numBones);
        toRootTransforms[0] = mBoneToParentTransforms[0];
        for (int i = 1; i < numBones; i++)
        {
            XMMATRIX toParent = XMLoadFloat4x4(&mBoneToParentTransforms[i]);
            int parentIndex = mBoneHierarchy[i];
            XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
            XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
            XMStoreFloat4x4(&toRootTransforms[i], toRoot);
        }

        for (int i = 0; i < numBones; ++i)
        {
            int limbIdx = 0;
            uint64_t* pDeformerID = nullptr;
            uint64_t* pLimbID = nullptr;
            int* pLimbIdx = nullptr;

            pDeformerID = deformerHashMap.search(i);
            if (pDeformerID == nullptr) continue;

            pLimbID = deformerToLimb.search(*pDeformerID);
            if (pLimbID == nullptr)
                limbIdx = i;
            else
            {
                pLimbIdx = limbHashMap.search(*pLimbID);
                if (pLimbIdx == nullptr) continue;
                limbIdx = *pLimbIdx;
            }

            XMMATRIX mtxInverseRootTransform;

            XMMATRIX mtxGlobalTrans = XMLoadFloat4x4(&mModelGlobalTrans[limbIdx]);
            XMMATRIX mtxLocalTrans = XMLoadFloat4x4(&mModelLocalTrans[limbIdx]);

            XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
            XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
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
                XMMATRIX rootTransform = XMLoadFloat4x4(&mBoneFinalTransforms[0]);
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

            mtxTrans = newBindPose * mtxGlobalTrans;


            XMFLOAT4X4 finalTransform;
            XMStoreFloat4x4(&finalTransform, mtxTrans);
            mBoneFinalTransforms.push_back(finalTransform);

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

