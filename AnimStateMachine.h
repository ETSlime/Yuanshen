#pragma once

//=============================================================================
//
// ���揈�� [AnimStateMachine.h]
// Author : 
//
//=============================================================================

#include "FBXLoader.h"
#include "renderer.h"
#include "HashMap.h"
//*****************************************************************************
// �}�N����`
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

    ~AnimationClip()
    {
        SAFE_DELETE(currBoneTransform);
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

// ��ԑJ�ڂ̒�`
struct StateTransition 
{
    uint64_t currentState;
    uint64_t nextState;
    bool (ISkinnedMeshModelChar::* condition)() const;  // �J�ڏ����i�����_���Ŏ����j
    bool waitForAnimationEnd;                           // �A�j���[�V�������I�����Ă���J�ڂ��邩
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
    AnimationClip* currentClip; // ���݂̃A�j���[�V����
    AnimationClip* prevClip;    // �O�̃A�j���[�V�����i�u�����h�p�j
    void (ISkinnedMeshModelChar::* onEndCallback)();    // �A�j���[�V�����I�����̃R�[���o�b�N�֐�
    SimpleArray<XMFLOAT4X4> blendedMatrices;
    float blendFactor;          // �u�����h�W���i0~1�j

    // ��ԑJ�ڃ��X�g
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

    // �J�ډ\�ȏ�Ԃ����邩���`�F�b�N
    NextStateInfo GetNextState(ISkinnedMeshModelChar* character);

    void AddTransition(uint64_t currentState, uint64_t nextState, bool (ISkinnedMeshModelChar::* condition)() const, bool waitForAnimationEnd = false, bool animBlend = true);

    void SetEndCallback(void (ISkinnedMeshModelChar::* callback)());

    // �e�{�[���̕ϊ��s����u�����h����
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
    bool blendOn;


public:
    AnimationStateMachine(ISkinnedMeshModelChar* c) : currentState(nullptr), character(c), blendOn(true) {}

    void AddState(uint64_t stateName, AnimationClip* clip);

    void SetEndCallback(uint64_t stateName, void (ISkinnedMeshModelChar::* callback)());

    void SetCurrentState(uint64_t newStateName);

    uint64_t GetCurrentState(void);

    void Update(float deltaTime, ISkinnedMeshModelChar* character);

    void AddTransition(uint64_t from, uint64_t to, bool (ISkinnedMeshModelChar::* condition)() const, bool waitForAnimationEnd = false, bool animBlend = true);

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices();
};
