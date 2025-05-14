//=============================================================================
//
// ParticleStructs���� [ParticleStructs.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IEffectRenderer.h"

//*********************************************************
// �\����
//*********************************************************

// --- BillboardSimple ---
struct BillboardSimpleParticle
{
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float size;
    float life;
    float lifeRemaining;
    float rotation;
    XMFLOAT4 color;
    XMFLOAT4 startColor;
    XMFLOAT4 endColor;
};

// --- BillboardFlipbook ---
struct BillboardFlipbookParticle
{
    XMFLOAT3 position;              // ���݈ʒu
    XMFLOAT3 velocity;              // ���x
    float size;                     // �T�C�Y
    float life;
    float lifeRemaining;            // �c������i�b�j
    float rotation;                 // ��]�p�i���W�A���j
    XMFLOAT4 color;
    XMFLOAT4 startColor;
    XMFLOAT4 endColor;
    float frameIndex;               // �A�j���[�V�����̌��݃t���[���ifloat�A�����Ή��j
    float frameSpeed;               // 1�b������Đ����x�i��ԑΉ��p
};