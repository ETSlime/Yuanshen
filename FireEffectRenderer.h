//=============================================================================
//
// FireEffectRenderer処理 [FireEffectRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IEffectRenderer.h"
#include "Timer.h"
#include "SimpleArray.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PARTICLES       (512) // 最大粒子数
#define PARTICLE_SIZE       (0.5f)
#define PARTICLE_LIFE_TIME  (2.0f) // 粒子の寿命
//*********************************************************
// 構造体
//*********************************************************

// 粒子データ構造体
struct FireParticle
{
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float    life;
    float    size;
    XMFLOAT4 color;
};

// 定数バッファ構造体
struct CBFireEffect
{
    // 時間情報
    float deltaTime;
    float totalTime;
    
    float scale;
    float padding;

    // 行列情報
    XMMATRIX world;
    XMMATRIX viewProj;
};

class FireEffectRenderer : public IEffectRenderer
{
public:

    ~FireEffectRenderer();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Update(void) override;
    void Draw(const XMMATRIX& viewProj) override;
    void SetupPipeline() override;

    void SetScale(float scale) { m_scale = scale; }
    void SetPosition(const XMFLOAT3& pos) { m_position = pos; }
    const XMFLOAT3& GetPosition() const { return m_position; }
    EffectType GetEffectType() const override { return EffectType::Fire; }

    static void ResetPipelineState(void) { s_pipelineBound = false; }

private:
    bool CreateParticleBuffer(void);
    bool LoadShaders(void);
    void DispatchComputeShader(void);
    void DrawParticles(const XMMATRIX& worldMatrix, const XMMATRIX& viewProj);


    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    ID3D11Buffer* m_particleBuffer = nullptr;
    ID3D11ShaderResourceView* m_particleSRV = nullptr;
    ID3D11UnorderedAccessView* m_particleUAV = nullptr;
    ID3D11BlendState* m_blendState = nullptr;

    // コンピュートシェーダー
    ComputeShaderSet m_computeShader;
    ShaderSet m_fireShaderSet;

    ID3D11Buffer* m_cbFireEffect = nullptr;

    ID3D11ShaderResourceView* m_flameTextureSRV = nullptr;

    // パーティクル最大数
    UINT m_maxParticles = MAX_PARTICLES;
    // パーティクルスケール
    float m_scale = 1.0f;

    XMFLOAT3 m_position = { 0, 0, 0 };

    static bool s_pipelineBound;

    Timer& m_timer = Timer::get_instance();
    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();
    Renderer& m_renderer = Renderer::get_instance();
};

