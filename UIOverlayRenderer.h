//=============================================================================
//
// UIOverlayRenderer���� [UIOverlayRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define UI_TEXTURE_PATH "data/TEXTURE/white_1x1.png"

// UI�t�F�[�h�E�I�[�o�[���C�`��N���X
class UIOverlayRenderer
{
public:
    bool Initialize(ID3D11Device* device);
    void Shutdown();

    // �u�ԕ\���i�A���t�@�w��j
    void Draw(ID3D11DeviceContext* ctx, float x, float y, float width, float height, XMFLOAT4 color);

    // �t�F�[�h���ʁiAlpha�l0.0f~1.0f�j
    void StartFadeIn(float duration, XMFLOAT4 color);
    void StartFadeOut(float duration, XMFLOAT4 color);

    void Update(float deltaTime);
    bool IsFading() const { return m_isFading; }

private:
    ID3D11Buffer* m_vertexBuffer = nullptr;
    ID3D11ShaderResourceView* m_whiteTextureSRV = nullptr;
    float m_fadeTime = 0.0f;
    float m_fadeDuration = 0.0f;
    bool m_fadeIn = true;
    bool m_isFading = false;
    XMFLOAT4 m_targetColor = { 0, 0, 0, 0 };

    // ���݂̃A���t�@�l���擾
    float ComputeAlpha() const;
};
