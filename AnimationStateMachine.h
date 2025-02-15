#pragma once

//=============================================================================
//
// ���揈�� [AnimationStateMachine.h]
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
    AnimationClip* currentClip; // ���݂̃A�j���[�V����
    AnimationClip* prevClip;    // �O�̃A�j���[�V�����i�u�����h�p�j
    void (*onEndCallback)();    // �A�j���[�V�����I�����̃R�[���o�b�N�֐�

    float blendFactor;  // �u�����h�W���i0~1�j

    AnimationState(char* stateName, AnimationClip* animClip)
        : stateName(stateName), currentClip(animClip), prevClip(nullptr), blendFactor(0.0f) {}

    void StartBlend(AnimationClip* newClip) 
    {
        prevClip = currentClip; // ���A�j���[�V������ۑ�
        currentClip = newClip;  // �V�A�j���[�V�����ɐ؂�ւ�
        blendFactor = 0.0f; // �u�����h�J�n
    }

    void Update(float deltaTime) 
    {
        if (blendFactor < 1.0f) 
        {
            blendFactor += deltaTime * 2.0f;
        }
    }

    // �e�{�[���̕ϊ��s����u�����h����
    XMMATRIX* GetBlendedMatrix() 
    {
        if (!currentClip) return {};  // �A�j���[�V�������Ȃ��ꍇ�͋󃊃X�g��Ԃ�
        if (!prevClip || blendFactor >= 1.0f) 
        {
            return currentClip->GetBoneMatrices();  // �O�̃A�j���[�V�������Ȃ��ꍇ�A�܂��̓u�����h������
        }

        //const std::vector<XMMATRIX>& prevMatrices = previousClip->GetBoneMatrices();
        //const std::vector<XMMATRIX>& currMatrices = currentClip->GetBoneMatrices();

        //size_t boneCount = currMatrices.size();
        //std::vector<XMMATRIX> blendedMatrices(boneCount);

        //for (size_t i = 0; i < boneCount; i++) {
        //    // �e�{�[���̕ϊ��s��𕪉��i�k���A��]�A���s�ړ��j
        //    XMVECTOR prevScale, prevRot, prevTrans;
        //    XMVECTOR currScale, currRot, currTrans;
        //    XMMatrixDecompose(&prevScale, &prevRot, &prevTrans, prevMatrices[i]);
        //    XMMatrixDecompose(&currScale, &currRot, &currTrans, currMatrices[i]);

        //    // �ʒu�i���s�ړ��j�̕��
        //    XMVECTOR blendedTrans = XMVectorLerp(prevTrans, currTrans, blendFactor);

        //    // ��]�i�l�����j�̕��
        //    XMVECTOR blendedRot = XMQuaternionSlerp(prevRot, currRot, blendFactor);

        //    // �g��k���̕��
        //    XMVECTOR blendedScale = XMVectorLerp(prevScale, currScale, blendFactor);

        //    // �u�����h��̍s����č\�z
        //    blendedMatrices[i] = XMMatrixScalingFromVector(blendedScale) *
        //        XMMatrixRotationQuaternion(blendedRot) *
        //        XMMatrixTranslationFromVector(blendedTrans);
        //}

        //return blendedMatrices;
        return nullptr;
    }
};
