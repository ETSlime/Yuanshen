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
    DebugProc::get_instance().Register(this);

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
    XMFLOAT3 dir = light->GetDirection();
    m_lightDir = XMVectorSet(dir.x, dir.y, dir.z, 0.0f);

    m_csm.UpdateCascades(m_lightDir, m_camera.GetNearZ(), m_camera.GetFarZ());
    //UpdateCascadeDebugBounds();

    m_csm.UnbindShadowSRVs();
}

void ShadowMapRenderer::RenderCSMForLight(DirectionalLight* light, int lightIndex, const SimpleArray<IGameObject*>& sceneObjects)
{
    BeginShadowPass(light);

    for (int cascadeIndex = 0; cascadeIndex < m_csm.GetCascadeCount(); cascadeIndex++)
    {
        m_csm.BeginCascadeRender(lightIndex, cascadeIndex);
        m_csm.SetViewport();
        m_csm.UpdateCascadeCBuffer(cascadeIndex);

        m_csm.CollectFromScene(cascadeIndex, sceneObjects);
        RenderShadowPass(cascadeIndex);
    }

    EndShadowPass();

    m_csm.UpdateCascadeCBufferArray();
    m_csm.BindShadowSRVsToPixelShader(lightIndex, CSM_SRV_SLOT);
}

void ShadowMapRenderer::RenderShadowPass(int cascadeIndex)
{
    ShadowMeshCollector& collector = m_csm.GetCascadeCollector(cascadeIndex);

    if (m_enableStaticShadow)
    {
        const auto& staticMeshes = collector.GetStaticMeshes();
        for (UINT i = 0; i < staticMeshes.getSize(); ++i)
        {
            RenderStaticMesh(staticMeshes[i]);
        }
    }

    if (m_enableSkinnedShadow)
    {
        const auto& skinnedMeshes = collector.GetSkinnedMeshes();
        for (UINT i = 0; i < skinnedMeshes.getSize(); ++i)
        {
            RenderSkinnedMesh(skinnedMeshes[i]);
        }
    }

    if (m_enableInstancedShadow)
    {
        const auto& instancedMeshes = collector.GetInstancedMeshes();
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

    m_renderer.SetCurrentWorldMatrix(&mesh.worldMatrix);

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
        m_context->PSSetShader(nullptr, nullptr, 0);
    }

    m_context->DrawIndexed(mesh.indexCount, 0, 0);
}

void ShadowMapRenderer::RenderSkinnedMesh(const SkinnedRenderData& mesh)
{
    m_context->IASetInputLayout(m_skinnedModelShaderSet.inputLayout);

    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &mesh.stride, &offset);
    m_context->IASetIndexBuffer(mesh.indexBuffer, mesh.indexFormat, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_renderer.SetCurrentWorldMatrix(&mesh.worldMatrix);
    if (mesh.pBoneMatrices)
    {
        m_renderer.SetBoneMatrix(mesh.pBoneMatrices);
    }
    else
    {
        XMMATRIX	boneMatrices[BONE_MAX];
        boneMatrices[0] = XMMatrixTranspose(XMMatrixIdentity());
        m_renderer.SetBoneMatrix(boneMatrices);
    }

    //D3D11_MAPPED_SUBRESOURCE mapped = {};
    //m_context->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    //CBPerObject* cb = reinterpret_cast<CBPerObject*>(mapped.pData);
    //cb->worldViewProj = wvp;
    //context->Unmap(m_cbPerObject, 0);

    m_context->VSSetShader(m_skinnedModelShaderSet.vs, nullptr, 0);
    //m_context->VSSetConstantBuffers(0, 1, &m_cbPerObject);

    if (mesh.enableAlphaTest && mesh.opacityMapSRV)
    {
        m_context->PSSetShader(m_skinnedModelShaderSet.alphaPs, nullptr, 0);
        m_context->PSSetShaderResources(0, 1, &mesh.opacityMapSRV);
    }
    else
    {
        m_context->PSSetShader(nullptr, nullptr, 0);
    }

    m_context->DrawIndexed(mesh.indexCount, 0, 0);
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

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowSRV(int lightIndex, int cascadeIndex) const
{
    return m_csm.GetShadowSRV(lightIndex, cascadeIndex);
}

const XMMATRIX& ShadowMapRenderer::GetLightViewProj(int cascadeIndex) const
{
    return m_csm.GetCascadeData(cascadeIndex).lightViewProj;
}

void ShadowMapRenderer::RenderImGui(void)
{
    // シャドウのタイプごとの有効化設定（1行ずつチェックボックス表示）
    ImGui::Checkbox("Enable Static Shadow", &m_enableStaticShadow);
    ImGui::Checkbox("Enable Skinned Shadow", &m_enableSkinnedShadow);
    ImGui::Checkbox("Enable Instanced Shadow", &m_enableInstancedShadow);

    for (int i = 0; i < m_csm.GetCascadeCount(); ++i)
    {
        ImGui::Text("Cascade %d", i);
        ImGui::Image((ImTextureID)m_csm.GetSRV(0, i), ImVec2(128, 128));
    }


    // CSM 設定のデバッグUIを表示
    ImGui::Separator();
    ImGui::Text("Cascaded Shadow Map");
    m_csm.RenderImGui();
}

void ShadowMapRenderer::RenderDebugInfo(void)
{
    m_csm.VisualizeCascadeBounds();
}

void ShadowMapRenderer::UpdateCascadeDebugBounds(void)
{
    const int cascadeCount = m_csm.GetCascadeCount();

    float nearZ = m_camera.GetNearZ();
    float farZ = m_camera.GetFarZ();
    XMMATRIX camView = XMLoadFloat4x4(&m_camera.GetViewMatrix());
    XMMATRIX camProj = XMLoadFloat4x4(&m_camera.GetProjMatrix());

    float lambda = 0.5f;
    float splitDepths[MAX_CASCADES];

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
