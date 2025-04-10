//=============================================================================
//
// レンダリング処理 [ShadowMapRenderer.cpp]
// Author : 
//
//=============================================================================
#include "ShadowMapRenderer.h"
#include "ShaderLoader.h"

bool ShadowMapRenderer::Init(int shadowMapSize, int numCascades)
{
    m_csm.Initialize(shadowMapSize, numCascades);
    m_debugBounds.resize(numCascades);

    m_device = Renderer::get_instance().GetDevice();
    m_context = Renderer::get_instance().GetDeviceContext();

    //for (auto& debug : m_debugBounds) 
    //{
    //    debug.debugBox = GeometricPrimitive::CreateBox(device);
    //}

    // 空のピクセルシェーダーを読み込む
    ID3D11PixelShader* emptyPS = nullptr;
    if (ShaderLoader::LoadEmptyPixelShader(m_device, &emptyPS, "DepthMap.hlsl", "DummyPS"))
        m_shaderManager.SetSharedShadowPixelShader(emptyPS);
    else
        return false;

    bool loadShaders = true;

    // 静的モデル用シャドウシェーダー
    loadShaders &= m_shaderManager.HasShadowShaderSet(ShaderSetID::StaticModel);
    m_staticModelShaderSet = m_shaderManager.GetShadowShaderSet(ShaderSetID::StaticModel);

    // スキニングメッシュ用シャドウシェーダー
    loadShaders &= m_shaderManager.HasShadowShaderSet(ShaderSetID::SkinnedModel);
    m_skinnedModelShaderSet = m_shaderManager.GetShadowShaderSet(ShaderSetID::SkinnedModel);

    // インスタンスモデル（草、木など）
    loadShaders &= m_shaderManager.HasShadowShaderSet(ShaderSetID::Instanced_Tree);
    m_instancedModelShaderSet = m_shaderManager.GetShadowShaderSet(ShaderSetID::Instanced_Tree);

    return loadShaders;
}

void ShadowMapRenderer::Shutdown() 
{
    m_csm.Shutdown();
    //m_debugBounds.clear();
}

void ShadowMapRenderer::BeginShadowPass(const DirectionalLight* light)
{
    m_cameraView = XMLoadFloat4x4(&m_camera.GetViewMatrix());
    m_cameraProj = XMLoadFloat4x4(&m_camera.GetProjMatrix());
    XMFLOAT3 dir = light->GetDirection();
    m_lightDir = XMVectorSet(dir.x, dir.y, dir.z, 0.0f);
    m_nearZ = m_camera.GetNearZ();
    m_farZ = m_camera.GetFarZ();  

    m_csm.UpdateCascades(m_cameraView, m_cameraProj, m_lightDir, m_nearZ, m_farZ);
    UpdateCascadeDebugBounds();
}

void ShadowMapRenderer::RenderCSMForLight(DirectionalLight* light, const SimpleArray<IGameObject*>& sceneObjects)
{
    BeginShadowPass(light);

    for (int i = 0; i < m_csm.GetCascadeCount(); ++i)
    {
        m_csm.BeginCascadeRender(i);
        m_csm.SetViewport();

        m_context->ClearDepthStencilView(m_csm.GetDSV(i), D3D11_CLEAR_DEPTH, 1.0f, 0);

        const auto& cascade = m_csm.GetCascadeData(i);
        CollectFromScene(sceneObjects, cascade.lightViewProj);
        RenderShadowPass();
    }

    EndShadowPass();
}

void ShadowMapRenderer::RenderShadowPass(void)
{
    if (m_enableStaticShadow)
    {
        const auto& staticMeshes = m_shadowCollector.GetStaticMeshes();
        for (UINT i = 0; i < staticMeshes.getSize(); ++i)
        {
            RenderStaticMesh(staticMeshes[i]);
        }
    }

    if (m_enableSkinnedShadow)
    {
        const auto& skinnedMeshes = m_shadowCollector.GetSkinnedMeshes();
        for (UINT i = 0; i < skinnedMeshes.getSize(); ++i)
        {
            RenderSkinnedMesh(skinnedMeshes[i]);
        }
    }

    if (m_enableInstancedShadow)
    {
        const auto& instancedMeshes = m_shadowCollector.GetInstancedMeshes();
        for (UINT i = 0; i < instancedMeshes.getSize(); ++i)
        {
            RenderInstancedMesh(instancedMeshes[i]);
        }
    }
}

void ShadowMapRenderer::CollectFromScene_NoCulling(const SimpleArray<IGameObject*>& objects)
{
    m_shadowCollector.Clear();

    for (auto* obj : objects)
    {
        if (!obj->GetUse() || !obj->GetCastShadow()) 
            continue;

        obj->CollectShadowMesh(m_shadowCollector);
    }
}

void ShadowMapRenderer::CollectFromScene(const SimpleArray<IGameObject*> objects, const XMMATRIX& lightViewProj)
{
    m_shadowCollector.Clear();

    for (auto* obj : objects)
    {
        if (!obj->GetUse() || !obj->GetCastShadow()) continue;

        const BOUNDING_BOX& worldAABB = obj->GetBoundingBoxWorld();
        if (!IsAABBInsideLightFrustum(worldAABB, lightViewProj)) continue;

        obj->CollectShadowMesh(m_shadowCollector);
    }
}

void ShadowMapRenderer::RenderStaticMesh(const StaticRenderData& mesh)
{
    m_context->IASetInputLayout(m_staticModelShaderSet.inputLayout);

    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &mesh.stride, &offset);
    m_context->IASetIndexBuffer(mesh.indexBuffer, mesh.indexFormat, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    XMMATRIX lightVP = m_csm.GetCurrentCascadeData().lightViewProj;
    XMMATRIX world = mesh.worldMatrix;
    XMMATRIX wvp = XMMatrixTranspose(world * lightVP);

    //D3D11_MAPPED_SUBRESOURCE mapped = {};
    //m_context->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    //CBPerObject* cb = reinterpret_cast<CBPerObject*>(mapped.pData);
    //cb->worldViewProj = wvp;
    //context->Unmap(m_cbPerObject, 0);

    m_context->VSSetShader(m_staticModelShaderSet.vs, nullptr, 0);
    //m_context->VSSetConstantBuffers(0, 1, &m_cbPerObject);

    if (mesh.enableAlphaTest && mesh.opacityMapSRV)
    {
        m_context->PSSetShader(m_staticModelShaderSet.alphaPs, nullptr, 0);
        m_context->PSSetShaderResources(0, 1, &mesh.opacityMapSRV);
    }
    else
    {
        m_context->PSSetShader(m_staticModelShaderSet.ps, nullptr, 0);
    }

    m_context->DrawIndexed(mesh.indexCount, 0, 0);
}

void ShadowMapRenderer::RenderSkinnedMesh(const SkinnedRenderData& mesh)
{
}

void ShadowMapRenderer::RenderInstancedMesh(const InstancedRenderData& mesh)
{
}

bool ShadowMapRenderer::IsAABBInsideLightFrustum(const BOUNDING_BOX& worldAABB, const XMMATRIX& lightViewProj)
{
    // AABBの8つの頂点を取得
    XMVECTOR corners[8];
    worldAABB.GetCorners(corners);

    for (int i = 0; i < 8; ++i)
    {
        // ライトの視点空間へ変換
        XMVECTOR clipPos = XMVector3Transform(corners[i], lightViewProj);

        // 正規化デバイス座標(NDC)に変換
        XMVECTOR ndcPos = clipPos / XMVectorGetW(clipPos);

        float x = XMVectorGetX(ndcPos);
        float y = XMVectorGetY(ndcPos);
        float z = XMVectorGetZ(ndcPos);

        // ひとつでも視野内に入っていれば採用
        if (x >= -1.0f && x <= 1.0f &&
            y >= -1.0f && y <= 1.0f &&
            z >= 0.0f && z <= 1.0f)
        {
            return true;
        }
    }

    return false;
}

void ShadowMapRenderer::EndShadowPass()
{
    for (int i = 0; i < m_csm.GetCascadeCount(); ++i) 
    {
        m_csm.EndCascadeRender();
    }
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowSRV(int cascadeIndex) const
{
    return m_csm.GetShadowSRV(cascadeIndex);
}

const XMMATRIX& ShadowMapRenderer::GetLightViewProj(int cascadeIndex) const
{
    return m_csm.GetCascadeData(cascadeIndex).lightViewProj;
}

void ShadowMapRenderer::VisualizeCascadeBounds(void)
{
    XMFLOAT4 colors[] = {
    {1, 0, 0, 1}, // red
    {1, 1, 0, 1}, // yellow
    {0, 1, 0, 1}, // green
    {0, 1, 1, 1}  // cyan
    };

    for (int i = 0; i < m_csm.GetCascadeCount(); ++i)
    {
        const BOUNDING_BOX& box = m_debugBounds[i];
        m_debugBoxRenderer.DrawBox(box, m_camera.GetViewProjMtx(), colors[i]);
    }
}

void ShadowMapRenderer::UpdateCascadeDebugBounds(void)
{
    const int cascadeCount = m_csm.GetCascadeCount();

    float nearZ = m_camera.GetNearZ();
    float farZ = m_camera.GetFarZ();
    XMMATRIX camView = XMLoadFloat4x4(&m_camera.GetViewMatrix());
    XMMATRIX camProj = XMLoadFloat4x4(&m_camera.GetProjMatrix());

    float lambda = 0.5f;
    float splitDepths[CascadedShadowMap::MaxCascades];

    for (int i = 0; i < cascadeCount; ++i)
    {
        float p = static_cast<float>(i + 1) / cascadeCount;
        float logSplit = nearZ * powf(farZ / nearZ, p);
        float uniSplit = nearZ + (farZ - nearZ) * p;
        splitDepths[i] = lambda * logSplit + (1 - lambda) * uniSplit;
    }

    XMMATRIX invViewProj = XMMatrixInverse(nullptr, m_camera.GetViewProjMtx());


    for (int i = 0; i < cascadeCount; ++i)
    {
        float zn = (i == 0) ? nearZ : splitDepths[i - 1];
        float zf = splitDepths[i];

        SimpleArray<XMVECTOR>frustumCorners;


        for (int y = 0; y <= 1; ++y)
        {
            for (int x = 0; x <= 1; ++x)
            {
                for (int z = 0; z <= 1; ++z)
                {
                    float ndcX = (float)x * 2.0f - 1.0f;
                    float ndcY = (float)y * 2.0f - 1.0f;
                    float ndcZ = z ? (zf / farZ * 2.0f - 1.0f) : (zn / farZ * 2.0f - 1.0f);
                    XMVECTOR cornerNDC = XMVectorSet(ndcX, ndcY, ndcZ, 1.0f);
                    XMVECTOR cornerWorld = XMVector3TransformCoord(cornerNDC, invViewProj);
                    frustumCorners.push_back(cornerWorld);
                }
            }
        }

        m_debugBounds[i] = AABBUtils::CreateFromPoints(frustumCorners);
    }
}
