#pragma once

//=============================================================================
//
// 動画処理 [AnimationStateMachine.h]
// Author : 
//
//=============================================================================

#include "FBXLoader.h"
#include "SkinnedMeshModel.h"
#include "renderer.h"

class AnimationState 
{
public:
    char*   stateName;
    AnimationClip* currentClip; // 現在のアニメーション
    AnimationClip* prevClip;    // 前のアニメーション（ブレンド用）
    void (*onEndCallback)();    // アニメーション終了時のコールバック関数

    float blendFactor;  // ブレンド係数（0~1）

    AnimationState(char* stateName, AnimationClip* animClip)
        : stateName(stateName), currentClip(animClip), prevClip(nullptr), blendFactor(0.0f) {}

    void StartBlend(AnimationClip* newClip) 
    {
        prevClip = currentClip; // 旧アニメーションを保存
        currentClip = newClip;  // 新アニメーションに切り替え
        blendFactor = 0.0f; // ブレンド開始
    }

    void Update(float deltaTime) 
    {
        if (blendFactor < 1.0f) 
        {
            blendFactor += deltaTime * 2.0f;
        }
    }

    // 各ボーンの変換行列をブレンドする
    XMMATRIX* GetBlendedMatrix() 
    {
        if (!currentClip) return {};  // アニメーションがない場合は空リストを返す
        if (!prevClip || blendFactor >= 1.0f) 
        {
            return currentClip->GetBoneMatrices();  // 前のアニメーションがない場合、またはブレンド完了時
        }

        //const std::vector<XMMATRIX>& prevMatrices = previousClip->GetBoneMatrices();
        //const std::vector<XMMATRIX>& currMatrices = currentClip->GetBoneMatrices();

        //size_t boneCount = currMatrices.size();
        //std::vector<XMMATRIX> blendedMatrices(boneCount);

        //for (size_t i = 0; i < boneCount; i++) {
        //    // 各ボーンの変換行列を分解（縮小、回転、平行移動）
        //    XMVECTOR prevScale, prevRot, prevTrans;
        //    XMVECTOR currScale, currRot, currTrans;
        //    XMMatrixDecompose(&prevScale, &prevRot, &prevTrans, prevMatrices[i]);
        //    XMMatrixDecompose(&currScale, &currRot, &currTrans, currMatrices[i]);

        //    // 位置（平行移動）の補間
        //    XMVECTOR blendedTrans = XMVectorLerp(prevTrans, currTrans, blendFactor);

        //    // 回転（四元数）の補間
        //    XMVECTOR blendedRot = XMQuaternionSlerp(prevRot, currRot, blendFactor);

        //    // 拡大縮小の補間
        //    XMVECTOR blendedScale = XMVectorLerp(prevScale, currScale, blendFactor);

        //    // ブレンド後の行列を再構築
        //    blendedMatrices[i] = XMMatrixScalingFromVector(blendedScale) *
        //        XMMatrixRotationQuaternion(blendedRot) *
        //        XMMatrixTranslationFromVector(blendedTrans);
        //}

        //return blendedMatrices;
        return nullptr;
    }
};
