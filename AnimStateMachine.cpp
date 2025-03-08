//=============================================================================
//
// AnimStateMachine���� [AnimStateMachine.cpp]
// Author : 
//
//=============================================================================
#include "AnimStateMachine.h"
#include "SkinnedMeshModel.h"
#include "debugproc.h"
#include "GameObject.h"
SimpleArray<XMFLOAT4X4>* AnimationClip::GetBoneMatrices(SimpleArray<XMFLOAT4X4>* currBoneTransform)
{
    if (model)
    {
        model->GetBoneTransformByAnim(armatureNode, currentTime, currBoneTransform, isLoop);
        return currBoneTransform;
    }
    else
        return nullptr;
}

void AnimationState::StartBlend(AnimationClip* prevClip)
{
    this->prevClip = prevClip; // ���A�j���[�V������ۑ�
    blendFactor = 0.0f; // �u�����h�J�n
}

void AnimationState::Update(float deltaTime)
{
    if (blendFactor < 1.0f)
    {
        blendFactor += deltaTime * 2.0f;
    }
    if (blendFactor > 1.0f) blendFactor = 1.0f;

    UpdateBlendedMatrix();
}

NextStateInfo AnimationState::GetNextState(ISkinnedMeshModelChar* character)
{
    for (const auto& transition : transitions) 
    {
        if (transition.value.condition && (character->*transition.value.condition)())
        {
            if (currentClip && currentClip->IsFinished())
            {
                if (onEndCallback)
                    (character->*onEndCallback)();
                return NextStateInfo(transition.key, transition.value.animBlend);
            }
            else if (!transition.value.waitForAnimationEnd) 
            {
                return NextStateInfo(transition.key, transition.value.animBlend);
            }
        }
    }
    return NextStateInfo(currentStateName, false); // �ύX�Ȃ�
}

void AnimationState::AddTransition(uint64_t currentState, uint64_t nextState,
    bool (ISkinnedMeshModelChar::* condition)() const, bool waitForAnimationEnd, bool animBlend)
{
    transitions[nextState] = { currentState, nextState, condition, waitForAnimationEnd, animBlend };
}

void AnimationState::SetEndCallback(void (ISkinnedMeshModelChar::* callback)())
{
    onEndCallback = callback;
}

void AnimationState::UpdateBlendedMatrix(void)
{
    if (!currentClip) return;  // �A�j���[�V�������Ȃ��ꍇ�͋󃊃X�g��Ԃ�

    SimpleArray<XMFLOAT4X4>* prevMatrices = nullptr;
    SimpleArray<XMFLOAT4X4>* currMatrices = nullptr;

    currMatrices = currentClip->GetBoneMatrices(&currentClip->currBoneTransform);
    int boneCount; 
    boneCount = currMatrices->getSize();

    blendedMatrices.clear();

    if (!prevClip || blendFactor >= 1.0f)// || ) blendOn == false)
    {
        // �O�̃A�j���[�V�������Ȃ��ꍇ�A�܂��̓u�����h������
        for (int i = 0; i < boneCount; i++)
        {
            blendedMatrices.push_back((*currMatrices)[i]);
        }

        return;
    }

    prevMatrices = prevClip->GetBoneMatrices(&prevClip->currBoneTransform);

    for (int i = 0; i < boneCount; i++)
    {
        // �e�{�[���̕ϊ��s��𕪉��i�k���A��]�A���s�ړ��j
        XMVECTOR prevScale, prevRot, prevTrans;
        XMVECTOR currScale, currRot, currTrans;
        XMMATRIX prevMtx = XMLoadFloat4x4(&(*prevMatrices)[i]);
        XMMATRIX currMtx = XMLoadFloat4x4(&(*currMatrices)[i]);

        XMMatrixDecompose(&prevScale, &prevRot, &prevTrans, prevMtx);
        XMMatrixDecompose(&currScale, &currRot, &currTrans, currMtx);

        // �ʒu�i���s�ړ��j�̕��
        XMVECTOR blendedTrans = XMVectorLerp(prevTrans, currTrans, blendFactor);

        // ��]�i�l�����j�̕��
        XMVECTOR blendedRot = XMQuaternionSlerp(prevRot, currRot, blendFactor);

        // �g��k���̕��
        XMVECTOR blendedScale = XMVectorLerp(prevScale, currScale, blendFactor);

        // �u�����h��̍s����č\�z
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

void AnimationStateMachine::SetEndCallback(uint64_t stateName, void (ISkinnedMeshModelChar::* callback)())
{
    AnimationState** animState = animStates.search(stateName);
    if (animState) 
    {
        (*animState)->SetEndCallback(callback);
    }
}

void AnimationStateMachine::SetCurrentState(uint64_t newStateName)
{
    AnimationState** newState = animStates.search(newStateName);
    if (newState)
    {
        if (currentState)
        {
            if (currentState->currentStateName == newStateName)
                return;
            (*newState)->currentClip->currentTime = 0;
            if (blendOn)
                (*newState)->StartBlend(currentState->currentClip);
        }
        currentState = *newState;
    }
}

uint64_t AnimationStateMachine::GetCurrentState(void)
{
    if (currentState)
        return currentState->currentStateName;

    return UINT64_MAX;
}

void AnimationStateMachine::Update(float deltaTime, ISkinnedMeshModelChar* character)
{
    if (currentState)
    {
        // �������`�F�b�N���Ď��̏�ԂɈڍs
        NextStateInfo nextStateInfo = currentState->GetNextState(character);
        if (nextStateInfo.nextState != currentState->currentStateName)
        {
            blendOn = nextStateInfo.animBlend;
            SetCurrentState(nextStateInfo.nextState);

        }

        currentState->Update(deltaTime);
    }
}

void AnimationStateMachine::AddTransition(uint64_t from, uint64_t to, bool (ISkinnedMeshModelChar::* condition)() const, bool waitForAnimationEnd, bool animBlend)
{
    AnimationState** animState = animStates.search(from);
    if (animState)
    {
        (*animState)->AddTransition(from, to, condition, waitForAnimationEnd, animBlend);
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

AnimationClip* AnimationStateMachine::GetCurrentAnimClip()
{
    if (currentState)
    {
        return currentState->currentClip;
    }

    return nullptr;
}
