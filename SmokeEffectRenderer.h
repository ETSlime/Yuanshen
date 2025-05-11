//=============================================================================
//
// SmokeEffectRenderer処理 [SmokeEffectRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "ParticleEffectRendererBase.h"
#include "SimpleArray.h"

//*********************************************************
// 構造体
//*********************************************************

// パーティクルデータ
struct SmokeParticle
{
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float size;
    float life;
    float lifeRemaining;
    XMFLOAT4 color;
    float rotation;
};

class SmokeEffectRenderer : public ParticleEffectRendererBase
{
public:

    virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    virtual void Shutdown(void) override;
    EffectType GetEffectType() const override { return EffectType::Smoke; }
    bool UseDrawIndirect() const override { return true; }

private:
    virtual bool CreateParticleBuffer(void) override;
    virtual void DispatchComputeShader(void) override; // パーティクル更新（Dispatch）
    virtual void DrawParticles(void) override;

    bool CreateDiffuseTex(void);

    ID3D11ShaderResourceView* m_diffuseTextureSRV = nullptr; 
};

