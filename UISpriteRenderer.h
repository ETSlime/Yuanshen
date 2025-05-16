//=============================================================================
//
// UI���؂��؂��`�����G�`���X�v���C�g����� [UISpriteRenderer.h]
// Author : 
// UI�X�v���C�g����ʂɂ؂����Ɠ\��t����`�您�����N���X�ł���
// ��]�E�ʒu�E�T�C�Y�EUV�܂ōׂ��������ł��āA�܂�ł��G�`�����@�̂悤�ɓ�����
// 
//=============================================================================
#pragma once
#include "Renderer.h"

class UISpriteRenderer
{
public:
    // ���S����ɃX�v���C�g��`�悷��
    static void SetSpriteCenter(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH);

    // �������ɃX�v���C�g��`�悷��
    static void SetSpriteLeftTop(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH);

    // �w�肳�ꂽ�F�ŃX�v���C�g��`�悷��i���S��j�����̊֐��ł͐F�𖳎����ĕ`�悵�܂��iUIVertex�ɂ�Diffuse���������߁j
    static void SetSpriteWithColor(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH, XMFLOAT4 color);

    // ��]�t���ŃX�v���C�g��`�悷��i���S��j
    static void SetSpriteWithRotation(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH,
        XMFLOAT4 color, float rotationRad);
};
