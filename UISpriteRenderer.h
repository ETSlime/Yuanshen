//=============================================================================
//
// UISpriteRenderer処理 [UISpriteRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

class UISpriteRenderer
{
public:
    // 中心を基準にスプライトを描画する
    static void SetSpriteCenter(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH);

    // 左上を基準にスプライトを描画する
    static void SetSpriteLeftTop(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH);

    // 指定された色でスプライトを描画する（中心基準）※この関数では色を無視して描画します（UIVertexにはDiffuseが無いため）
    static void SetSpriteWithColor(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH, XMFLOAT4 color);

    // 回転付きでスプライトを描画する（中心基準）
    static void SetSpriteWithRotation(ID3D11Buffer* buf, float X, float Y, float Width, float Height,
        float U, float V, float UW, float VH,
        XMFLOAT4 color, float rotationRad);
};
