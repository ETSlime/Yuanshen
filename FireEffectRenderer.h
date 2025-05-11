//=============================================================================
//
// FireEffectRenderer���� [FireEffectRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IEffectRenderer.h"
#include "Timer.h"
#include "SimpleArray.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_PARTICLES       (512) // �ő嗱�q��
#define PARTICLE_SIZE       (0.5f)
#define PARTICLE_LIFE_TIME  (2.0f) // ���q�̎���
//*********************************************************
// �\����
//*********************************************************

// ���q�f�[�^�\����
struct FireParticle
{
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float    life;
    float    size;
    XMFLOAT4 color;
};

// �萔�o�b�t�@�\����
struct CBFireEffect
{
    // ���ԏ��
    float deltaTime;
    float totalTime;
    
    float scale;
    float padding;

    // �s����
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

    // �R���s���[�g�V�F�[�_�[
    ComputeShaderSet m_computeShader;
    ShaderSet m_fireShaderSet;

    ID3D11Buffer* m_cbFireEffect = nullptr;

    ID3D11ShaderResourceView* m_flameTextureSRV = nullptr;

    // �p�[�e�B�N���ő吔
    UINT m_maxParticles = MAX_PARTICLES;
    // �p�[�e�B�N���X�P�[��
    float m_scale = 1.0f;

    XMFLOAT3 m_position = { 0, 0, 0 };

    static bool s_pipelineBound;

    Timer& m_timer = Timer::get_instance();
    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();
    Renderer& m_renderer = Renderer::get_instance();
};

