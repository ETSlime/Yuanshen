//=============================================================================
//
// �����J�X�P�[�h�ɂ��V���h�E�}�b�v�����ƕ����Ǘ� [CascadedShadowMap.h]
// Author : 
// �J�����̎�����𕡐��J�X�P�[�h�ɕ������A�e�J�X�P�[�h���Ƃ�
// ���C�g�r���[���e�E�e�����E�e�N�Z���X�i�b�v�␳���s��
// 
//=============================================================================
#pragma once
#include "Core/Graphics/Renderer.h"
#include "Collision/AABBUtils.h"
#include "Scene/GameObject.h"
#include "Core/Camera.h"
#include "Utility/DebugBoundingBoxRenderer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
// �V���h�E�}�b�v�𑜓x�i1�J�X�P�[�h���Ƃ̃T�C�Y�j
#define CSM_SHADOW_MAP_SIZE 2048
// �J�X�P�[�h�̐��i�ő�4�j
#define MAX_CASCADES    4
#define SHADOW_CASTING_LIGHT_MAX 2
#define MIN_SHADOW_DISTANCE 15.0f
#define MAX_SHADOW_DISTANCE 40000.0f

// �f�o�b�O�p�̃J�X�P�[�h���E����
#define DEBUG_CASCADE_BOUND     0

enum class ShadowRadiusStrategy
{
    FitBoundingSphere,
    FitBoundingBox,
    PaddingBuffer
};

//*********************************************************
// �\����
//*********************************************************
struct CascadedShadowConfig
{
    int numCascades = MAX_CASCADES;
    int shadowMapSize = CSM_SHADOW_MAP_SIZE;
    float manualSplits[MAX_CASCADES] = { 0.05f, 0.15f, 0.3f, 1.0f };

    // �����̃^�C�v�𐧌䂷��W���i0.0 = ���`�A1.0 = �ΐ��A0.95�Ȃ�قڑΐ��j
    float lambda = 0.95f;

    // �X�v���b�g�̐[�x���蓮�Ŏw�肷�邩�ǂ���
    bool useManualSplits = false;
    // �e�N�Z���X�i�b�v��L�������邩�ǂ���
    bool enableTexelSnap = true;
    // �X�i�b�v�̐��x���w�肷��
    ShadowRadiusStrategy radiusStrategy = ShadowRadiusStrategy::FitBoundingBox;
    // �J�X�P�[�h�̊Ԋu�𒲐����邽�߂̃p�f�B���O
    float padding[MAX_CASCADES] = { 60.0f, 100.0f, 150.0f, 200.0f }; // ���C���[���Ƃ�padding
    float maxShadowDistance = MAX_SHADOW_DISTANCE;
    float minShadowDistance = MIN_SHADOW_DISTANCE;
};

struct CBCascadeData
{
    XMMATRIX lightViewProj;
    XMMATRIX lightView;
    XMMATRIX lightProj;
    float splitDepth;
    float padding[3]; // 16�o�C�g�A���C�����g
};

struct CascadePrivateData
{
    XMMATRIX lightViewProjLocal;
    XMVECTOR viewFrustumCenter;
};

struct CBCascadeDataArray
{
    CBCascadeData cascadeArray[MAX_CASCADES];    // �ő�4���C���[�̃V���h�E
    XMFLOAT4 cascadeSplits;                    // �s�N�Z���V�F�[�_�[�ł̐[�x��r�p
};

class CascadedShadowMap 
{
public:

    CascadedShadowMap();
    ~CascadedShadowMap();

    void Initialize(int shadowMapSize = CSM_SHADOW_MAP_SIZE, int numCascades = MAX_CASCADES);
    void Shutdown(void);

    void UpdateCascades(const XMVECTOR& lightDir, float nearZ, float farZ);

    void UpdateCascadeCBufferArray(void);
    void UpdateCascadeCBuffer(int cascadeIndex);

    void UnbindShadowSRVs(void);
    void BeginCascadeRender(int lightIndex, int cascadeIndex);
    void EndCascadeRender(void);
    void SetViewport(void);

    void BindShadowSRVsToPixelShader(int lightIndex, int startSlot);
    ID3D11ShaderResourceView* GetSRV(int lightIndex, int cascadeIndex) const;

    // ���f���̉e�����W����
    void CollectFromScene_NoCulling(int cascadeIndex, const SimpleArray<IGameObject*>& objects);
    void CollectFromScene(int cascadeIndex, const SimpleArray<IGameObject*>& objects);

	// �J�X�P�[�h�̃R���N�^���擾
    ShadowMeshCollector& GetCascadeCollector(int i) { return m_perCascadeCollector[i]; }
    int GetCascadeObjectCount(int i) const { return m_cascadeObjectCount[i]; }

    // ��������E�J�X�P�[�h��SRV���擾
    ID3D11ShaderResourceView* GetShadowSRV(int lightIndex, int cascadeIndex) const;
    // ��������E�J�X�P�[�h��DSV���擾
    ID3D11DepthStencilView* GetDSV(int lightIndex, int cascadeIndex) const;
    // ��������̑S�J�X�P�[�h����SRV�z����擾
    ID3D11ShaderResourceView** GetAllShadowSRVs(int lightIndex);

    const CBCascadeData& GetCascadeData(int index) const { return m_cascadeData[index]; }
    const CBCascadeData& GetCurrentCascadeData(void) { return m_cascadeData[m_currentCascadeIndex]; }
    int GetCascadeCount() const { return m_config.numCascades; }

    void RenderImGui(void); // ImGui�ł̐ݒ�\��
	void VisualizeCascadeBounds(void); // �f�o�b�O�p�̃J�X�P�[�h���E����

private:
	// �J�X�P�[�h�̕������v�Z����֐�
    void ComputeCascadeMatrices(int cascadeIndex, float splitNear, float splitFar, const XMVECTOR& lightDir);
    // �J�X�P�[�h�p��CBuffer��������
	void InitCascadeCBuffer(void); 

    // �V���h�E�}�b�v�̃e�N�Z���O���b�h�ɃX�i�b�v����␳�֐�
    XMMATRIX SnapShadowMatrixToTexelGrid(const XMMATRIX& lightViewProjLocal);
    XMMATRIX SnapShadowMatrixToTexelGrid(const XMMATRIX& lightView, const XMMATRIX& lightProj, XMVECTOR center);

    // �J�X�P�[�h�V���h�E�}�b�v�̔��a���v�Z���郆�[�e�B���e�B�֐�
    float ComputeCascadeRadius(XMVECTOR* corners, XMVECTOR center, ShadowRadiusStrategy strategy, float padding);
    // ���C���X�^���X���J�����O����
    void CullVisibleInstancesForShadow(InstanceModelAttribute& attribute, int cascadeIndex);
	// AABB�����C�g�̎���ɓ����Ă��邩�ǂ������m�F����֐�
    bool IsAABBInsideLightFrustum(const BOUNDING_BOX& worldAABB, int cascadeIndex);

    // CascadedShadowMap�Ƒ�����maxDistance�����J�����̎�����č\�z����֐�
    XMMATRIX ComputeCSMViewProjMatrixWithMaxDistance(float maxDistance);

    // ���C�g�r���[�v���W�F�N�V�����s������Z����֐�
    XMMATRIX ComputeLightProjMatrixFromSliceCorners(const XMVECTOR sliceCorners[8], const XMMATRIX& lightView, float padding);

    void AdjustCascadePadding(void);

    int m_currentCascadeIndex = 0;

	CBCascadeData m_cascadeData[MAX_CASCADES]; // �J�X�P�[�h�f�[�^
	CascadePrivateData m_cascadePrivateData[MAX_CASCADES]; // �J�X�P�[�h�v���C�x�[�g�f�[�^

    ShadowMeshCollector m_perCascadeCollector[MAX_CASCADES]; // �e���C���[�̃R���N�^
    int m_cascadeObjectCount[MAX_CASCADES]; // �e���C���[�̃J�E���g

    float m_lastCascadeRadius;
    XMVECTOR m_lastLightDir;

	CascadedShadowConfig m_config{}; // �V���h�E�}�b�v�̐ݒ�

	ID3D11Texture2D* m_shadowMaps[SHADOW_CASTING_LIGHT_MAX][MAX_CASCADES] = {}; // �V���h�E�}�b�v  
	ID3D11DepthStencilView* m_dsvs[SHADOW_CASTING_LIGHT_MAX][MAX_CASCADES] = {}; // �[�x�X�e���V���r���[
	ID3D11ShaderResourceView* m_srvs[SHADOW_CASTING_LIGHT_MAX][MAX_CASCADES] = {}; // �V���h�ESRV

	D3D11_VIEWPORT m_shadowViewport = {}; // �r���[�|�[�g

	ID3D11Buffer* m_cascadeSingleCBuffer = nullptr; // �P��J�X�P�[�h�p��CBuffer
	ID3D11Buffer* m_cascadeArrayCBuffer = nullptr; // �J�X�P�[�h�z��p��CBuffer

    // �ۑ����ꂽRTV&DSV
	ID3D11RenderTargetView* m_savedRTV = nullptr; 
    ID3D11DepthStencilView* m_savedDSV = nullptr;

    ID3D11Texture2D* m_dummyRTTexture = nullptr;
    ID3D11RenderTargetView* m_dummyRTV = nullptr;

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    Camera& m_camera = Camera::get_instance();
    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();

    // debug
    XMVECTOR m_tempFrustumCenter[MAX_CASCADES];
    XMMATRIX m_tempInvViewProj;
    XMVECTOR m_tempSliceCorners[MAX_CASCADES][8];
    DebugBoundingBoxRenderer m_debugBoundingBoxRenderer[MAX_CASCADES];
    bool m_showCascadeBox[MAX_CASCADES] = { true, true, true, true };
};
