//=============================================================================
//
// FireBallEffectRenderer 処理 [FireBallEffectRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "ParticleEffectRendererBase.h"

//*********************************************************
// 構造体
//*********************************************************

// 火球用のパーティクルデータ（StructuredBuffer）
struct FireBallParticle
{
    XMFLOAT3 position;              // 現在位置
    float lifeRemaining;            // 残り寿命（秒）
    float life;
    XMFLOAT3 velocity;              // 速度
    float rotation;                 // 回転角（ラジアン）

    float size;                     // サイズ
    float frameIndex;               // アニメーションの現在フレーム（float、小数対応）
    float frameSpeed;              // 1秒あたり再生速度（補間対応用
    XMFLOAT4 color;
};

struct CBFireBall
{
    UINT tilesX = 7;
    UINT tilesY = 7;
    float initialFrameOffset = 0.0f;
    float frameLerpCurve = 1.0f;

    float rotationSpeed = 0.0f;
    float coneAngleDegree = 25.0f;
    float coneRadius = 0.6193843f;
    float coneLength = 5.0f;
};

class FireBallEffectRenderer : public ParticleEffectRendererBase
{
public:
    virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    virtual void Shutdown(void) override;
    EffectType GetEffectType() const override { return EffectType::FireBall; }
    bool UseDrawIndirect(void) const override { return true; }
    virtual void ConfigureEffect(const ParticleEffectParams& params) override;

private:
    virtual bool CreateParticleBuffer(void) override;
    virtual void DispatchComputeShader(void) override; // パーティクル更新（Dispatch）
    virtual void DrawParticles(void) override;

    bool CreateFireBallCB(void);
    void UploadFireBallCB(void);

    bool CreateDiffuseTex(void);

    float m_spawnConeAngleDegree = 25.0f;   // コーン角度（度）
    float m_spawnConeRadius = 0.6193843f;   // コーン底面の半径
    float m_spawnConeLength = 5.0f;         // コーンの高さ
    float m_rotationSpeed = 0.0f;
    float m_frameLerpCurve = 0.0f;

    UINT m_tilesX = 7;                      // テクスチャシート横枚数
    UINT m_tilesY = 7;                      // テクスチャシート縦枚数

    ID3D11Buffer* m_cbFireBall = nullptr;
    ID3D11ShaderResourceView* m_diffuseTextureSRV = nullptr;
};