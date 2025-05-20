//=============================================================================
//
// �e�����p�X�ƃ��f���`��̓����V���h�E�����_���[ [ShadowMapRenderer.h]
// Author : 
// �J�X�P�[�h�V���h�E�}�b�v�ɑΉ������e�`�揈���𓝊����A
// �ÓI�E�X�L�����b�V���E�C���X�^���X���f���̉e�����𐧌䂷��
// 
//=============================================================================
#pragma once
#include "Camera.h"
#include "Light.h"
#include "CascadedShadowMap.h"
#include "ShaderManager.h"
#include "DebugBoundingBoxRenderer.h"
#include "ShadowMeshCollector.h"
#include "Debugproc.h"



class ShadowMapRenderer : public SingletonBase<ShadowMapRenderer>, public IDebugUI
{
public:

    bool Init(int shadowMapSize, int numCascades = 4);
    void Shutdown();

    // �V���h�E�}�b�v��`�悷��
    void RenderCSMForLight(DirectionalLight* light, int lightIndex, const SimpleArray<IGameObject*>& sceneObjects);

    ID3D11ShaderResourceView* GetShadowSRV(int lightIndex, int cascadeIndex) const;
    const XMMATRIX& GetLightViewProj(int cascadeIndex) const;

    inline void SetEnableStaticShadow(bool enable) { m_enableStaticShadow = enable; }
    inline void SetEnableSkinnedShadow(bool enable) { m_enableSkinnedShadow = enable; }
    inline void SetEnableInstancedShadow(bool enable) { m_enableInstancedShadow = enable; }

    inline bool IsStaticShadowEnabled() const { return m_enableStaticShadow; }
    inline bool IsSkinnedShadowEnabled() const { return m_enableSkinnedShadow; }
    inline bool IsInstancedShadowEnabled() const { return m_enableInstancedShadow; }

private:

    // �V���h�E�}�b�v�̏�����
    void BeginShadowPass(const DirectionalLight* light);
    // �V���h�E�}�b�v�̕`����J�n
    void RenderShadowPass(int cascadeIndex);
    // �V���h�E�}�b�v�̕`����I��
    void EndShadowPass(void);

    // ���f���̉e��`�悷��
    void RenderStaticMesh(const StaticRenderData& mesh);
    // �{�[���A�j���[�V�������f���̉e��`�悷��
    void RenderSkinnedMesh(const SkinnedRenderData& mesh);
    // �C���X�^���X�����ꂽ���f���̉e��`�悷��
    void RenderInstancedMesh(const InstancedRenderData& mesh);

    bool IsAABBInsideLightFrustum(const BOUNDING_BOX& worldAABB, const XMMATRIX& lightViewProj);


    virtual void RenderImGui(void) override;
    virtual const char* GetPanelName(void) const override { return "Shadow Map Renderer"; };
    virtual void RenderDebugInfo(void) override;

    CascadedShadowMap m_csm;
    ShadowMeshCollector m_shadowCollector;

    XMVECTOR m_lightDir;

    float m_alphaCutoff = 0.5f;

    bool m_enableStaticShadow = true;
    bool m_enableSkinnedShadow = true;
    bool m_enableInstancedShadow = true;

    Camera& m_camera = Camera::get_instance();

    ShadowShaderSet m_staticModelShaderSet;
    ShadowShaderSet m_skinnedModelShaderSet;
    ShadowShaderSet m_instancedModelShaderSet;

    ID3D11Device* m_device = Renderer::get_instance().GetDevice();
    ID3D11DeviceContext* m_context = Renderer::get_instance().GetDeviceContext();
    Renderer& m_renderer = Renderer::get_instance();
    ShaderManager& m_shaderManager = ShaderManager::get_instance();
};