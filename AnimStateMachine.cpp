//=============================================================================
//
// AnimStateMachine処理 [AnimStateMachine.cpp]
// Author : 
//
//=============================================================================
#include "AnimStateMachine.h"
#include "SkinnedMeshModel.h"
#include "debugproc.h"

SimpleArray<XMFLOAT4X4>* AnimationClip::GetBoneMatrices(SimpleArray<XMFLOAT4X4>* currBoneTransform)
{
    if (model)
    {
        model->GetBoneTransformByAnim(armatureNode, currentTime, currBoneTransform);
        return currBoneTransform;
    }
    else
        return nullptr;
}

void AnimationState::StartBlend(AnimationClip* prevClip)
{
    this->prevClip = prevClip; // 旧アニメーションを保存
    //currentClip = newClip;  // 新アニメーションに切り替え
    blendFactor = 0.0f; // ブレンド開始
}

void AnimationState::Update(float deltaTime)
{
    if (blendFactor < 1.0f)
    {
        blendFactor += deltaTime * 2.0f;
    }
    UpdateBlendedMatrix();
}

void AnimationState::SetEndCallback(void(*callback)())
{
    onEndCallback = callback;
}

void AnimationState::UpdateBlendedMatrix(void)
{
    if (!currentClip) return;  // アニメーションがない場合は空リストを返す

    SimpleArray<XMFLOAT4X4>* prevMatrices = nullptr;
    SimpleArray<XMFLOAT4X4>* currMatrices = nullptr;

    XMMATRIX mtxTrans;

    currMatrices = currentClip->GetBoneMatrices(currentClip->currBoneTransform);
    int boneCount; 
    boneCount = currMatrices->getSize();

    blendedMatrices.clear();

    if (!prevClip || blendFactor >= 1.0f)
    {
        // 前のアニメーションがない場合、またはブレンド完了時
        for (int i = 0; i < boneCount; i++)
        {
            blendedMatrices.push_back((*currMatrices)[i]);
        }

        return;
    }

    mtxTrans = XMLoadFloat4x4(&(*currMatrices)[15]);
    prevMatrices = prevClip->GetBoneMatrices(prevClip->currBoneTransform);


    for (int i = 0; i < boneCount; i++)
    {
        // 各ボーンの変換行列を分解（縮小、回転、平行移動）
        XMVECTOR prevScale, prevRot, prevTrans;
        XMVECTOR currScale, currRot, currTrans;
        XMMATRIX prevMtx = XMLoadFloat4x4(&(*prevMatrices)[i]);
        XMMATRIX currMtx = XMLoadFloat4x4(&(*currMatrices)[i]);

        XMMatrixDecompose(&prevScale, &prevRot, &prevTrans, prevMtx);
        XMMatrixDecompose(&currScale, &currRot, &currTrans, currMtx);

        // 位置（平行移動）の補間
        XMVECTOR blendedTrans = XMVectorLerp(prevTrans, currTrans, blendFactor);

        // 回転（四元数）の補間
        XMVECTOR blendedRot = XMQuaternionSlerp(prevRot, currRot, blendFactor);

        // 拡大縮小の補間
        XMVECTOR blendedScale = XMVectorLerp(prevScale, currScale, blendFactor);

        // ブレンド後の行列を再構築

        XMMATRIX blendedMtx = XMMatrixScalingFromVector(blendedScale) *
            XMMatrixRotationQuaternion(blendedRot) *
            XMMatrixTranslationFromVector(blendedTrans);

        XMFLOAT4X4 blended;
        XMStoreFloat4x4(&blended, blendedMtx);
        blendedMatrices.push_back(blended);
    }
}

SimpleArray<XMFLOAT4X4>* AnimationState::GetBlendedMatrix(void)
{
    return &blendedMatrices;
}

void AnimationStateMachine::AddState(uint64_t stateName, AnimationClip* clip)
{
    AnimationState* state = new AnimationState(stateName, clip);
    animStates.insert(stateName, state);
}

void AnimationStateMachine::SetCurrentState(uint64_t newStateName)
{
    AnimationState** newState = animStates.search(newStateName);
    if (newState)
    {
        if (currentState)
        {
            if (currentState->stateName == newStateName)
                return;
            (*newState)->currentClip->currentTime = 0;
            (*newState)->StartBlend(currentState->currentClip);
        }
        currentState = *newState;
    }
}

uint64_t AnimationStateMachine::GetCurrentState(void)
{
    if (currentState)
        return currentState->stateName;

    return UINT64_MAX;
}

void AnimationStateMachine::Update(float deltaTime)
{
    if (currentState)
    {
        currentState->Update(deltaTime);
    }
}

SimpleArray<XMFLOAT4X4>* AnimationStateMachine::GetBoneMatrices()
{
    if (currentState)
    {
        return currentState->GetBlendedMatrix();
    }
    return nullptr;
}
