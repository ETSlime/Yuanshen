//=============================================================================
//
// �����_�����O���� [ShadowMapRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Camera.h"
#include "Light.h"
#include "SimpleArray.h"
#include "Renderer.h"
#include "SingletonBase.h"
#include "CascadedShadowMap.h"
#include "ShaderManager.h"
#include "DebugBoundingBoxRenderer.h"
#include "ShadowMeshCollector.h"

class ShadowMapRenderer : public SingletonBase<ShadowMapRenderer>
{
public:

    bool Init(int shadowMapSize, int numCascades = 4);
    void Shutdown();

    // ���f���̉e�����W����
    void CollectFromScene_NoCulling(const SimpleArray<IGameObject*>& objects);
    void CollectFromScene(const SimpleArray<IGameObject*> objects, const XMMATRIX& lightViewProj);
    // �V���h�E�}�b�v��`�悷��
    void RenderCSMForLight(DirectionalLight* light, const SimpleArray<IGameObject*>& sceneObjects);

    ID3D11ShaderResourceView* GetShadowSRV(int cascadeIndex) const;
    const XMMATRIX& GetLightViewProj(int cascadeIndex) const;

    void VisualizeCascadeBounds(void);

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
    void RenderShadowPass(void);
    // �V���h�E�}�b�v�̕`����I��
    void EndShadowPass(void);

    // ���f���̉e��`�悷��
    void RenderStaticMesh(const StaticRenderData& mesh);
    // �{�[���A�j���[�V�������f���̉e��`�悷��
    void RenderSkinnedMesh(const SkinnedRenderData& mesh);
    // �C���X�^���X�����ꂽ���f���̉e��`�悷��
    void RenderInstancedMesh(const InstancedRenderData& mesh);

    bool IsAABBInsideLightFrustum(const BOUNDING_BOX& worldAABB, const XMMATRIX& lightViewProj);

    CascadedShadowMap m_csm;
    ShadowMeshCollector m_shadowCollector;

    XMMATRIX m_cameraView;
    XMMATRIX m_cameraProj;
    XMVECTOR m_lightDir;
    float m_nearZ = 0.1f;
    float m_farZ = 1000.0f;

    float m_alphaCutoff = 0.5f;

    bool m_enableStaticShadow = true;
    bool m_enableSkinnedShadow = true;
    bool m_enableInstancedShadow = true;

    //struct CascadeDebugData 
    //{
    //    BOUNDING_BOX cascadeBounds;
    //    //std::unique_ptr<GeometricPrimitive> debugBox;
    //};
    SimpleArray<BOUNDING_BOX> m_debugBounds;

    DebugBoundingBoxRenderer m_debugBoxRenderer;

    void UpdateCascadeDebugBounds(void);

    Camera& m_camera = Camera::get_instance();

    ShadowShaderSet m_staticModelShaderSet;
    ShadowShaderSet m_skinnedModelShaderSet;
    ShadowShaderSet m_instancedModelShaderSet;

    ID3D11Device* m_device = Renderer::get_instance().GetDevice();
    ID3D11DeviceContext* m_context = Renderer::get_instance().GetDeviceContext();
    ShaderManager& m_shaderManager = ShaderManager::get_instance();
};