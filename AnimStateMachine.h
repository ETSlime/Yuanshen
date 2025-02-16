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
#define MAX_ANIMSTATE_NUM   99

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

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices(SimpleArray<XMFLOAT4X4>* currBoneTransform);
};

class AnimationState 
{
public:
    uint64_t   stateName;
    AnimationClip* currentClip; // ���݂̃A�j���[�V����
    AnimationClip* prevClip;    // �O�̃A�j���[�V�����i�u�����h�p�j
    void (*onEndCallback)();    // �A�j���[�V�����I�����̃R�[���o�b�N�֐�
    SimpleArray<XMFLOAT4X4> blendedMatrices;
    float blendFactor;  // �u�����h�W���i0~1�j

    AnimationState(uint64_t stateName, AnimationClip* animClip)
        : stateName(stateName), currentClip(animClip), prevClip(nullptr), blendFactor(0.0f), onEndCallback(nullptr) {}

    void StartBlend(AnimationClip* newClip);

    void Update(float deltaTime);

    void SetEndCallback(void (*callback)());

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

public:
    AnimationStateMachine() : currentState(nullptr) {}

    void AddState(uint64_t stateName, AnimationClip* clip);

    void SetCurrentState(uint64_t newStateName);

    uint64_t GetCurrentState(void);

    void Update(float deltaTime);

    SimpleArray<XMFLOAT4X4>* GetBoneMatrices();
};
