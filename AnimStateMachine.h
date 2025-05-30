#pragma once
//=============================================================================
//
// スキンメッシュキャラクターのアニメーションステート制御 [AnimStateMachine.h]
// Author : 
// アニメーションクリップの再生状態、遷移条件、ブレンド処理、
// 終了コールバックの管理を行い、キャラクターの動作状態を制御する
// 
//=============================================================================

#include "FBXLoader.h"
#include "Renderer.h"
#include "HashMap.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ANIMSTATE_NUM       99
#define MAX_STATE_TRANS_NUM     99

#define STATE(state)                  (static_cast<uint64_t>(state))
// states
enum class PlayerState : uint64_t
{
    STANDING,
    IDLE,
    WALK,
    RUN,
    DASH,
    ATTACK_1,
    ATTACK_2,
    ATTACK_3,
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

enum class EnemyState : uint64_t
{
    IDLE,
    HILI_DANCE,
    HILI_STAND_UP,
    HILI_SIT,
    HILI_WALK,
    HILI_RUN,
    HILI_ATTACK,
    HILI_HIT1,
    HILI_HIT2,
    HILI_SURPRISED,
    HILI_DIE,
};

enum class AnimPlayMode
{
    ONCE,
    LOOP,
    PINGPONG
};

class SkinnedMeshModel;
class ISkinnedMeshModelChar;

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct AnimationPhase 
{
    float startMoveFraction; // 移動開始時間 (0~1)
    float endMoveFraction;   // 移動終了時間 (0~1)
    float startAttackFraction; // 攻撃開始時間 (0~1)
    float endAttackFraction;   // 攻撃終了時間 (0~1)
};

struct AnimationInfo
{
    BOOL                isReversed;
    AnimPlayMode        playMode;
    AnimationPhase      animPhase;
};

struct AnimationClip
{
    AnimClipName            name;
    FbxNode*                armatureNode;
    uint64_t                stopTime;
    uint64_t                currentTime;
    SkinnedMeshModel*       model;
    SimpleArray<XMFLOAT4X4> currBoneTransform;
    AnimationInfo           animInfo;

    AnimationClip()
    {
        name = AnimClipName::ANIM_NONE;
        armatureNode = nullptr;
        stopTime = 0;
        currentTime = 0;
        model = nullptr;
        currBoneTransform = SimpleArray<XMFLOAT4X4>();
        animInfo.isReversed = false;
        animInfo.playMode = AnimPlayMode::LOOP;
    }

    ~AnimationClip()
    {
    }

    inline void SetModel(SkinnedMeshModel* skinnedMeshModel)
    {
        model = skinnedMeshModel;
    }

    inline bool IsFinished() 
    {
        return currentTime >= stopTime;
    }

    inline void SetAnimPlayMode(AnimPlayMode playMode)
    {
        animInfo.playMode = playMode;
    }

    inline void CutAnimStopTime(uint64_t stopTime)
    {
        this->stopTime = stopTime;
    }

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices(SimpleArray<XMFLOAT4X4>* currBoneTransform);
};

// 状態遷移の定義
struct StateTransition 
{
    uint64_t currentState;
    uint64_t nextState;
    bool (ISkinnedMeshModelChar::* condition)() const;  // 遷移条件（ラムダ式で実装）
    bool waitForAnimationEnd;                           // アニメーションが終了してから遷移するか
    bool animBlend;
};

struct NextStateInfo
{
    uint64_t nextState;
    bool animBlend;

    NextStateInfo() = default;

    NextStateInfo(uint64_t nextState, bool animBlend) : nextState(nextState), animBlend(animBlend) {}
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
    NextStateInfo GetNextState(ISkinnedMeshModelChar* character);

    void AddTransition(uint64_t currentState, uint64_t nextState, bool (ISkinnedMeshModelChar::* condition)() const, 
        bool waitForAnimationEnd = false, bool animBlend = true);

    void SetEndCallback(void (ISkinnedMeshModelChar::* callback)());

    float GetCurrentAnimTime(void);

    // 各ボーンの変換行列をブレンドする
    void UpdateBlendedMatrix(void);

    SimpleArray<XMFLOAT4X4>* GetBlendedMatrix(void);
};

class AnimStateMachine 
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
    bool blendOn;


public:
    AnimStateMachine(ISkinnedMeshModelChar* c) : currentState(nullptr), character(c), blendOn(true) {}

    void AddState(uint64_t stateName, AnimationClip* clip);

    void SetEndCallback(uint64_t stateName, void (ISkinnedMeshModelChar::* callback)());

    void SetCurrentState(uint64_t newStateName);

    uint64_t GetCurrentState(void);

    void Update(float deltaTime, ISkinnedMeshModelChar* character);

    void AddTransition(uint64_t from, uint64_t to, bool (ISkinnedMeshModelChar::* condition)() const, 
        bool waitForAnimationEnd = false, bool animBlend = true);

    float GetCurrentAnimTime(void) { return currentState->GetCurrentAnimTime(); };

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices(void);

    AnimationClip* GetCurrentAnimClip(void);
};
