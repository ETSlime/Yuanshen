//=============================================================================
//
// UISpriteRenderer���� [UISpriteRenderer.cpp]
// Author : 
//
//=============================================================================
#include "UISpriteRenderer.h"

void UISpriteRenderer::DrawSpriteCenter(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
    float U, float V, float UW, float VH)
{
    D3D11_MAPPED_SUBRESOURCE msr;
    Renderer::get_instance().GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    UIVertex* vertex = reinterpret_cast<UIVertex*>(msr.pData);

    float hw = Width * 0.5f;
    float hh = Height * 0.5f;

    // ���S����ɒ��_��ݒ�
    vertex[0].Position = XMFLOAT2(X - hw, Y - hh); // ����
    vertex[1].Position = XMFLOAT2(X + hw, Y - hh); // �E��
    vertex[2].Position = XMFLOAT2(X - hw, Y + hh); // ����
    vertex[3].Position = XMFLOAT2(X + hw, Y + hh); // �E��

    vertex[0].TexCoord = XMFLOAT2(U, V);
    vertex[1].TexCoord = XMFLOAT2(U + UW, V);
    vertex[2].TexCoord = XMFLOAT2(U, V + VH);
    vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

    Renderer::get_instance().GetDeviceContext()->Unmap(buf, 0);
}

void UISpriteRenderer::DrawSpriteLeftTop(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
    float U, float V, float UW, float VH)
{
    D3D11_MAPPED_SUBRESOURCE msr;
    Renderer::get_instance().GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    UIVertex* vertex = reinterpret_cast<UIVertex*>(msr.pData);

    // �������ɒ��_��ݒ�
    vertex[0].Position = XMFLOAT2(X, Y);                   // ����
    vertex[1].Position = XMFLOAT2(X + Width, Y);           // �E��
    vertex[2].Position = XMFLOAT2(X, Y + Height);          // ����
    vertex[3].Position = XMFLOAT2(X + Width, Y + Height);  // �E��

    vertex[0].TexCoord = XMFLOAT2(U, V);
    vertex[1].TexCoord = XMFLOAT2(U + UW, V);
    vertex[2].TexCoord = XMFLOAT2(U, V + VH);
    vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

    Renderer::get_instance().GetDeviceContext()->Unmap(buf, 0);
}

void UISpriteRenderer::DrawSpriteWithColor(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
    float U, float V, float UW, float VH, XMFLOAT4 /*color*/)
{
    // ���F��UIVertex�Ɋ܂܂�Ă��Ȃ��̂Ŗ�������܂��i�ʂ̕����ŃA���t�@������F�ύX���l���j
    DrawSpriteCenter(buf, X, Y, Width, Height, U, V, UW, VH);
}

void UISpriteRenderer::DrawSpriteWithRotation(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
    float U, float V, float UW, float VH,
    XMFLOAT4 /*color*/, float Rot)
{
    D3D11_MAPPED_SUBRESOURCE msr;
    Renderer::get_instance().GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    UIVertex* vertex = reinterpret_cast<UIVertex*>(msr.pData);

    float hw = Width * 0.5f;
    float hh = Height * 0.5f;

    // ��]���S����̋����Ɗp�x���v�Z
    float baseAngle = atan2f(hh, hw);
    float radius = sqrtf(hw * hw + hh * hh);

    float x, y;

    // ���_����]���Ĕz�u
    x = X - cosf(baseAngle + Rot) * radius;
    y = Y - sinf(baseAngle + Rot) * radius;
    vertex[0].Position = XMFLOAT2(x, y); // ����

    x = X + cosf(baseAngle - Rot) * radius;
    y = Y - sinf(baseAngle - Rot) * radius;
    vertex[1].Position = XMFLOAT2(x, y); // �E��

    x = X - cosf(baseAngle - Rot) * radius;
    y = Y + sinf(baseAngle - Rot) * radius;
    vertex[2].Position = XMFLOAT2(x, y); // ����

    x = X + cosf(baseAngle + Rot) * radius;
    y = Y + sinf(baseAngle + Rot) * radius;
    vertex[3].Position = XMFLOAT2(x, y); // �E��

    vertex[0].TexCoord = XMFLOAT2(U, V);
    vertex[1].TexCoord = XMFLOAT2(U + UW, V);
    vertex[2].TexCoord = XMFLOAT2(U, V + VH);
    vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

    Renderer::get_instance().GetDeviceContext()->Unmap(buf, 0);
}
