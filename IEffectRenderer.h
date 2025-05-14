//=============================================================================
//
// IEffectRenderer処理 [IEffectRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
// 総スレッド数 total を groupSize 単位で割り、足りない分を繰り上げて補う
#define GET_THREAD_GROUP_COUNT(total, groupSize) (((total) + (groupSize) - 1) / (groupSize))

enum class EffectType
{
    None,

    Particle_Start,
    Fire,
    FireBall,
    Smoke,
    Particle_End,

    SoftBody,
};

class IEffectRenderer
{
public:
    virtual ~IEffectRenderer() = default;

    virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) = 0;
    virtual void Shutdown(void) = 0;
    virtual void Update(void) = 0;
    virtual void Draw(const XMMATRIX& viewProj) = 0;
    virtual void SetupPipeline() = 0;

    virtual const XMFLOAT3& GetPosition() const = 0;
    virtual void SetPosition(const XMFLOAT3& pos) = 0;
    virtual EffectType GetEffectType() const = 0;
};
