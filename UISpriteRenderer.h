//=============================================================================
//
// UISpriteRenderer���� [UISpriteRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

// UI�`��p�̒��_�\����
struct UIVertex
{
    XMFLOAT2 Position; // �ʒu�i�X�N���[�����W�j
    XMFLOAT2 TexCoord; // �e�N�X�`�����W
};

class UISpriteRenderer
{
public:
    // ���S����ɃX�v���C�g��`�悷��
    static void DrawSpriteCenter(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH);

    // �������ɃX�v���C�g��`�悷��
    static void DrawSpriteLeftTop(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH);

    // �w�肳�ꂽ�F�ŃX�v���C�g��`�悷��i���S��j�����̊֐��ł͐F�𖳎����ĕ`�悵�܂��iUIVertex�ɂ�Diffuse���������߁j
    static void DrawSpriteWithColor(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH, XMFLOAT4 color);

    // ��]�t���ŃX�v���C�g��`�悷��i���S��j
    static void DrawSpriteWithRotation(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH,
        XMFLOAT4 color, float rotationRad);
};
