//=============================================================================
//
// ParticleEffectRendererBase���� [ParticleEffectRendererBase.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IEffectRenderer.h"
#include "Timer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_PARTICLES       (512) // �ő�p�[�e�B�N����

//*********************************************************
// �\����
//*********************************************************
struct CBParticleUpdate
{
    XMFLOAT3 acceleration;
    float scale;
    
    float deltaTime;
    float totalTime;

    float lifeMin;
    float lifeMax;
    float spawnRateMin;
    float spawnRateMax;

    UINT maxParticlesCount;
    UINT particlesToEmitThisFrame;

    XMFLOAT4 startColor;
    XMFLOAT4 endColor;
};

struct CBParticleDraw
{
    // �s����
    XMMATRIX world;
    XMMATRIX viewProj;
};

struct DrawArgs
{
    UINT VertexCountPerInstance; // �e�C���X�^���X������̒��_���i�r���{�[�h�Ȃ�4�j
    UINT InstanceCount; // �C���X�^���X���i�A�N�e�B�u�ȃp�[�e�B�N�����j
    UINT StartVertexLocation;  // ���_�J�n�ʒu�i�ʏ��0�j
    UINT StartInstanceLocation; // �C���X�^���X�J�n�ʒu�i�ʏ��0�j
};

struct ParticleEffectParams;

// �p�[�e�B�N���G�t�F�N�g��p���N���X
class ParticleEffectRendererBase : public IEffectRenderer
{
public:
    ParticleEffectRendererBase() {}

    virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    virtual void Shutdown(void) override;
    virtual void Update(void) final override; // �p���N���X�ŃI�[�o�[���C�h�֎~
    virtual void Draw(const XMMATRIX& viewProj) final override; // �p���N���X�ŃI�[�o�[���C�h�֎~
    virtual EffectType GetEffectType() const override { return EffectType::None; } // ���N���X��None��Ԃ�
    virtual bool UseDrawIndirect() const { return false; } // DrawInstancedIndirect���g�p���邩�ǂ���
    virtual void ConfigureEffect(const ParticleEffectParams& params);

    void SetScale(float scale) { m_scale = scale; }
    void SetPosition(const XMFLOAT3& pos) { m_position = pos; }
    void SetAcceleration(const XMFLOAT3& acc) { m_acceleration = acc; }
    void SetSpawnRateRange(float min, float max) { m_spawnRateMin = min; m_spawnRateMax = max; }
    void SetLifeRange(float min, float max) { m_lifeMin = min; m_lifeMax = max; }
    void SetParticleNum(UINT numParticles) { m_maxParticles = numParticles; }

    void SetupPipeline(void);  // �p�C�v���C���ݒ�

    const XMFLOAT3& GetPosition() const { return m_position; }

    ParticleShaderGroup GetShaderGroupForEffect(EffectType type);
    ParticleComputeGroup GetComputeGroupForEffect(EffectType type);

    // �p�C�v���C����ԃ��Z�b�g
    static void ResetPipelineState(ParticleShaderGroup group) { s_pipelineBoundMap[TO_UINT64(group)] = false; }

protected:

    void DrawIndirect(void); // DrawInstancedIndirect���g�p����ꍇ�̕`��֐�

    // �p�[�e�B�N���X�V�pCB
    bool CreateConstantBuffer(void);

    // DrawInstancedIndirect() �ɕK�v�ȕ`������� GPU ���ŕێ����邽�߂̃o�b�t�@
    bool CreateDrawArgsBuffer(void);

    // GPU ��ŃA�N�e�B�u�ȃp�[�e�B�N���̃C���f�b�N�X���X�g�iUINT �z��j��ێ�����o�b�t�@
    // Compute Shader �ŏ������݁AVS ���ŎQ�Ƃ��ăC���X�^���X ID ���痱�q ID ���擾���܂�
    bool CreateAliveListBuffer(void);

    // GPU ��Ńt���[�ȃp�[�e�B�N���̃C���f�b�N�X���X�g�iUINT �z��j��ێ�����o�b�t�@
    bool CreateFreeListBuffer(void);

    // �A�N�e�B�u�ȃp�[�e�B�N�����X�g������������
    void InitializeFreeList(void);

    bool CreateParticleBuffer(void);  // �p�[�e�B�N���o�b�t�@���쐬����
    virtual void DispatchComputeShader(void) = 0; // �R���s���[�g�V�F�[�_�[���f�B�X�p�b�`����
    virtual void DrawParticles(void); // �p�[�e�B�N����`�悷��
    void DispatchEmitParticlesCS(void); // �p�[�e�B�N�������p�R���s���[�g�V�F�[�_�[���f�B�X�p�b�`����

    // �p�[�e�B�N���`��O�̋��ʃo�C���h
    virtual void BindParticleDrawResources(const XMMATRIX& worldMatrix, const XMMATRIX& viewProj);

    void BindCommonComputeResources(void); // �R���s���[�g�V�F�[�_�[�p�̋��ʃ��\�[�X���o�C���h����
    void UnbindComputeUAVs(void); // �R���s���[�g�V�F�[�_�[��UAV���A���o�C���h����
    void UnbindAllShaderSRVs(void); // �S�Ă�SRV���A���o�C���h����
    void PrepareForEmitCS(void); // �����pCS�̏���������
    void ClearInternalUAVs(void); // UAV���N���A����

    // �p�[�e�B�N���X�V�pCB�X�V�iDispatch�O�ɌĂяo���j
    void UpdateParticleUpdateCB(void);
    // �p�[�e�B�N��������̍X�V
    void UpdateEmitter(void);
    // �p�[�e�B�N���`��pCB�X�V�iDraw�O�ɌĂяo���j
    void UpdateParticleDrawCB(const XMMATRIX& worldMatrix, const XMMATRIX& viewProj);
    // DrawInstancedIndirect�p�̃C���X�^���X�����X�V
    void UpdateDrawArgsInstanceCount(void);
    // �V�F�[�_�[��ǂݍ���
    bool LoadShaders(void);

    // �o�^�^�֐��|�C���^
    using CreateBufferFunc = bool (*)(ParticleEffectRendererBase*);
    static CreateBufferFunc s_bufferFactoryTable[static_cast<UINT>(ParticleShaderGroup::Count)];

    // �����w���p�[
    static bool CreateBillboardSimpleBuffer(ParticleEffectRendererBase* self);
    static bool CreateBillboardFlipbookBuffer(ParticleEffectRendererBase* self);
    void RegisterarticleBufferFactories();

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    ID3D11Buffer* m_cbParticleUpdate = nullptr;  // �X�V�p�萔�o�b�t�@
    ID3D11Buffer* m_cbParticleDraw = nullptr;    // �`��p�萔�o�b�t�@
    ID3D11Buffer* m_particleBuffer = nullptr;    // �p�[�e�B�N���f�[�^�o�b�t�@
    ID3D11Buffer* m_drawArgsBuffer = nullptr;    // �`��p�����o�b�t�@
    ID3D11Buffer* m_aliveListBuffer = nullptr;  // �A�N�e�B�u�ȃp�[�e�B�N�����X�g�o�b�t�@
    ID3D11Buffer* m_freeListBuffer = nullptr;   // �t���[���X�g�o�b�t�@
    ID3D11ShaderResourceView* m_particleSRV = nullptr; // �p�[�e�B�N���ǂݎ��pSRV
    ID3D11ShaderResourceView* m_aliveListSRV = nullptr; // �A�N�e�B�u�ȃp�[�e�B�N�����X�g�pSRV
    ID3D11UnorderedAccessView* m_particleUAV = nullptr; // �p�[�e�B�N���������ݗpUAV
    ID3D11UnorderedAccessView* m_aliveListUAV = nullptr; // �A�N�e�B�u�ȃp�[�e�B�N�����X�g�pUAV
    ID3D11UnorderedAccessView* m_freeListUAV = nullptr; // �t���[���X�g�pUAV
    ID3D11UnorderedAccessView* m_freeListConsumeUAV = nullptr; // �t���[���X�g����pUAV

    ID3D11Buffer* m_cbParticleEffect = nullptr; // ��ʃo�b�t�@

    ParticleShaderGroup m_shaderGroup = ParticleShaderGroup::None;
    ParticleComputeGroup m_computeGroup = ParticleComputeGroup::None;
    ComputeShaderSet m_updateParticlesCS; // �R���s���[�g�V�F�[�_�[
    ComputeShaderSet m_emitParticlesCS; // �p�[�e�B�N�������p�R���s���[�g�V�F�[�_�[
    ShaderSet m_shaderSet; // VS/GS/PS�Z�b�g
    BLEND_MODE m_blendMode = BLEND_MODE_NONE;

    UINT m_maxParticles = MAX_PARTICLES; // �p�[�e�B�N���ő吔
    float m_scale = 1.0f; // �p�[�e�B�N���X�P�[��
    float m_lifeMin = 0.0f; // �ŏ�����
    float m_lifeMax = 0.0f; // �ő����
    float m_spawnRateMin = 0.0f; // �ŏ��������[�g
    float m_spawnRateMax = 0.0f; // �ő吶�����[�g
    XMFLOAT3 m_acceleration = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4 m_startColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMFLOAT4 m_endColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    float m_accumulatedEmitCount = 0.0f; // �ώZ�����J�E���^�i������ێ��j
    UINT m_particlesToEmitThisFrame = 0; // ���t���[����������p�[�e�B�N����
    UINT m_countBufferSizeBytes = sizeof(UINT);

    static HashMap<uint64_t, bool, HashUInt64, EqualUInt64> s_pipelineBoundMap;

    ShaderResourceBinder& m_shaderResourceBinder = ShaderResourceBinder::get_instance();
    Renderer& m_renderer = Renderer::get_instance();
    DebugProc& m_debugProc = DebugProc::get_instance();
    Timer& m_timer = Timer::get_instance();

    ID3D11Buffer* m_drawArgsStaging = nullptr;
};