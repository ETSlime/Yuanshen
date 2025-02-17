#pragma once

//=============================================================================
//
// 動画処理 [AnimStateMachine.h]
// Author : 
//
//=============================================================================

#include "FBXLoader.h"
#include "renderer.h"
#include "HashMap.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ANIMSTATE_NUM       99
#define MAX_STATE_TRANS_NUM     99

// states
enum PlayerState : uint64_t
{
    IDLE,
    WALK,
    RUN,
    DASH,
    ATTACK,
    JUMP,
    FALL,
    HARD_LANDING,
    HIT,
    KNOCKDOWN,
    REBOUND,
    DEFEND,
    CAST,
    DIE,
};

class SkinnedMeshModel;
class ISkinnedMeshModelChar;

struct AnimationClip
{
    AnimationClipName       name;
    FbxNode*                armatureNode;
    uint64_t                stopTime;
    uint64_t                currentTime;
    SkinnedMeshModel*       model;
    SimpleArray<XMFLOAT4X4>* currBoneTransform;
    BOOL                    isLoop;

    AnimationClip()
    {
        name = AnimationClipName::ANIM_NONE;
        armatureNode = nullptr;
        stopTime = 0;
        currentTime = 0;
        model = nullptr;
        isLoop = TRUE;
        currBoneTransform = new SimpleArray<XMFLOAT4X4>();
    }

    inline void SetModel(SkinnedMeshModel* skinnedMeshModel)
    {
        model = skinnedMeshModel;
    }

    inline bool IsFinished() 
    {
        return currentTime >= stopTime;
    }

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices(SimpleArray<XMFLOAT4X4>* currBoneTransform);
};

// 状態遷移の定義
struct StateTransition 
{
    uint64_t nextState;
    bool (ISkinnedMeshModelChar::* condition)() const;  // 遷移条件（ラムダ式で実装）
    bool waitForAnimationEnd;                           // アニメーションが終了してから遷移するか
};

class AnimationState 
{
public:
    uint64_t   currentStateName;
    AnimationClip* currentClip; // 現在のアニメーション
    AnimationClip* prevClip;    // 前のアニメーション（ブレンド用）
    void (ISkinnedMeshModelChar::* onEndCallback)();    // アニメーション終了時のコールバック関数
    SimpleArray<XMFLOAT4X4> blendedMatrices;
    float blendFactor;          // ブレンド係数（0~1）

    // 状態遷移リスト
    HashMap<uint64_t, StateTransition, HashUInt64, EqualUInt64> transitions =
        HashMap<uint64_t, StateTransition, HashUInt64, EqualUInt64>(
            MAX_STATE_TRANS_NUM,
            HashUInt64(),
            EqualUInt64()
        );

    AnimationState(uint64_t stateName, AnimationClip* animClip)
        : currentStateName(stateName), currentClip(animClip), prevClip(nullptr), blendFactor(0.0f), onEndCallback(nullptr) {}

    void StartBlend(AnimationClip* newClip);

    void Update(float deltaTime);

    // 遷移可能な状態があるかをチェック
    uint64_t GetNextState(ISkinnedMeshModelChar* character);

    void AddTransition(uint64_t nextState, bool (ISkinnedMeshModelChar::* condition)() const, bool waitForAnimationEnd = false);

    void SetEndCallback(void (ISkinnedMeshModelChar::* callback)());

    // 各ボーンの変換行列をブレンドする
    void UpdateBlendedMatrix(void);

    SimpleArray<XMFLOAT4X4>* GetBlendedMatrix(void);
};

class AnimationStateMachine 
{
private:
    HashMap<uint64_t, AnimationState*, HashUInt64, EqualUInt64> animStates =
        HashMap<uint64_t, AnimationState*, HashUInt64, EqualUInt64>(
        MAX_ANIMSTATE_NUM,
        HashUInt64(),
        EqualUInt64()
    );
    AnimationState* currentState;

    ISkinnedMeshModelChar* character;

public:
    AnimationStateMachine(ISkinnedMeshModelChar* c) : currentState(nullptr), character(c) {}

    void AddState(uint64_t stateName, AnimationClip* clip);

    void SetEndCallback(uint64_t stateName, void (ISkinnedMeshModelChar::* callback)());

    void SetCurrentState(uint64_t newStateName);

    uint64_t GetCurrentState(void);

    void Update(float deltaTime, ISkinnedMeshModelChar* character);

    void AddTransition(uint64_t from, uint64_t to, bool (ISkinnedMeshModelChar::* condition)() const, bool waitForAnimationEnd = false);

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices();
};
