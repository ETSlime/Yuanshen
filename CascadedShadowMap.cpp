//=============================================================================
//
// CascadedShadowMap���� [CascadedShadowMap.cpp]
// Author : 
//
//=============================================================================
#include "CascadedShadowMap.h"
#include "Debugproc.h"

CascadedShadowMap::CascadedShadowMap() {}
CascadedShadowMap::~CascadedShadowMap() { Shutdown(); }

void CascadedShadowMap::Initialize(int shadowMapSize, int numCascades) 
{
    m_device = Renderer::get_instance().GetDevice();
    m_context = Renderer::get_instance().GetDeviceContext();

    assert(numCascades <= MAX_CASCADES);

    m_config.shadowMapSize = shadowMapSize;
    m_config.numCascades = max(1, numCascades); // 1�ȏ�ɐ���

    m_shadowViewport = {};
    m_shadowViewport.TopLeftX = 0.0f;
    m_shadowViewport.TopLeftY = 0.0f;
    m_shadowViewport.Width = static_cast<float>(shadowMapSize);
    m_shadowViewport.Height = static_cast<float>(shadowMapSize);
    m_shadowViewport.MinDepth = 0.0f;
    m_shadowViewport.MaxDepth = 1.0f;

    for (int lightIdx = 0; lightIdx < SHADOW_CASTING_LIGHT_MAX; ++lightIdx)
    {
        for (int cascadeIdx = 0; cascadeIdx < m_config.numCascades; ++cascadeIdx)
        {
            // �e�N�X�`���[�̐ݒ�
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = shadowMapSize;
            texDesc.Height = shadowMapSize;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
            texDesc.SampleDesc.Count = 1;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

            HRESULT hr = m_device->CreateTexture2D(&texDesc, nullptr, &m_shadowMaps[lightIdx][cascadeIdx]);
            if (FAILED(hr)) continue; // �G���[����

            // DSV �ݒ�
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;

            hr = m_device->CreateDepthStencilView(m_shadowMaps[lightIdx][cascadeIdx], &dsvDesc, &m_dsvs[lightIdx][cascadeIdx]);
            if (FAILED(hr)) continue;

            // SRV �ݒ�
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;

            hr = m_device->CreateShaderResourceView(m_shadowMaps[lightIdx][cascadeIdx], &srvDesc, &m_srvs[lightIdx][cascadeIdx]);
            if (FAILED(hr)) continue;
        }
    }

    InitCascadeCBuffer();


#if DEBUG_CASCADE_BOUND
    for (int cascadeIndex = 0; cascadeIndex < m_config.numCascades; ++cascadeIndex)
    {
        char name[DEBUG_BOUNDING_BOX_NAME_LENGTH] = "Cascade BoundingBox: ";
        char idx[8] = {};
        sprintf(idx, "%d", cascadeIndex);
        strcat(name, idx);
        m_debugBoundingBoxRenderer[cascadeIndex].Initialize(name);
    }
#endif // _DEBUG

}

void CascadedShadowMap::Shutdown() 
{
    for (int lightIdx = 0; lightIdx < SHADOW_CASTING_LIGHT_MAX; ++lightIdx)
    {
        for (int cascadeIdx = 0; cascadeIdx < m_config.numCascades; ++cascadeIdx)
        {
            SafeRelease(&m_srvs[lightIdx][cascadeIdx]);
            SafeRelease(&m_dsvs[lightIdx][cascadeIdx]);
            SafeRelease(&m_shadowMaps[lightIdx][cascadeIdx]);
        }
    }
}

void CascadedShadowMap::SetViewport(void) 
{
    m_context->RSSetViewports(1, &m_shadowViewport);
}

void CascadedShadowMap::BindShadowSRVsToPixelShader(int lightIndex, int startSlot)
{
    m_ShaderResourceBinder.BindShaderResources(ShaderStage::PS, startSlot, m_config.numCascades, m_srvs[lightIndex]);
}

ID3D11ShaderResourceView* CascadedShadowMap::GetSRV(int lightIndex, int cascadeIndex) const
{
    assert(lightIndex >= 0 && lightIndex < SHADOW_CASTING_LIGHT_MAX);
    assert(cascadeIndex >= 0 && cascadeIndex < MAX_CASCADES);
    return m_srvs[lightIndex][cascadeIndex];
}

void CascadedShadowMap::RenderImGui(void)
{
    if (ImGui::CollapsingHeader("Cascaded Shadow Settings"))
    {
        ImGui::SliderInt("Cascade Count", &m_config.numCascades, 1, MAX_CASCADES);
        ImGui::SliderFloat("Max Shadow Distance", &m_config.maxShadowDistance, MIN_SHADOW_DISTANCE, MAX_SHADOW_DISTANCE, "%.1f");
        ImGui::Checkbox("Use Manual Splits", &m_config.useManualSplits);

        if (m_config.useManualSplits)
        {
            ImGui::Text("Manual Cascade Splits:");
            for (int i = 0; i < m_config.numCascades; ++i)
            {
                char label[32];
                sprintf_s(label, "Split %d", i);
                ImGui::SliderFloat(label, &m_config.manualSplits[i], 0.0f, 1.0f);
            }

            for (int i = 1; i < m_config.numCascades; ++i)
            {
                if (m_config.manualSplits[i] < m_config.manualSplits[i - 1])
                    m_config.manualSplits[i] = m_config.manualSplits[i - 1];
            }
        }
        else
        {
            ImGui::SliderFloat("Lambda (auto split)", &m_config.lambda, 0.0f, 1.0f);
        }

        ImGui::Checkbox("Enable Texel Snap", &m_config.enableTexelSnap);

        const char* strategyNames[] = {
            "FitBoundingSphere",
            "FitBoundingBox",
            "PaddingBuffer"
        };

        int strategyIndex = static_cast<int>(m_config.radiusStrategy);
        if (ImGui::Combo("Radius Strategy", &strategyIndex, strategyNames, IM_ARRAYSIZE(strategyNames)))
        {
            m_config.radiusStrategy = static_cast<ShadowRadiusStrategy>(strategyIndex);
        }

        if (m_config.radiusStrategy == ShadowRadiusStrategy::PaddingBuffer)
        {
            for (int i = 0; i < m_config.numCascades; ++i)
            {
                char label[32];
                sprintf_s(label, "Padding Layer %d", i);
                ImGui::SliderFloat("Padding", &m_config.padding[i], 0.0f, 100.0f, "%.1f");
            }
        }

        if (ImGui::CollapsingHeader("Cascade Debug Draw"))
        {
            ImGui::Checkbox("Show Cascade 0", &m_showCascadeBox[0]);
            ImGui::Checkbox("Show Cascade 1", &m_showCascadeBox[1]);
            ImGui::Checkbox("Show Cascade 2", &m_showCascadeBox[2]);
            ImGui::Checkbox("Show Cascade 3", &m_showCascadeBox[3]);
        }

        ImGui::Separator();

        ImGui::Text("Cascade Statistics:");
        for (int i = 0; i < m_config.numCascades; ++i)
        {
            const auto& cascade = m_cascadeData[i];
            float depth = cascade.splitDepth;

            char label[64];
            sprintf_s(label, "Cascade %d: Depth %.2f", i, depth);
            ImGui::Text("%s", label);

            ImGui::Text("Objects: %d", m_cascadeObjectCount[i]);

            XMFLOAT3 center;
            XMStoreFloat3(&center, m_tempFrustumCenter[i]);
            ImGui::Text("Cascade[%d] Center: (%.2f, %.2f, %.2f)", i, center.x, center.y, center.z);
        }




        //if (m_config.useManualSplits)
        //{
        //    for (int i = 0; i < m_config.numCascades; ++i)
        //    {
        //        char* label = "Split[" + std::to_string(i) + "]";
        //        ImGui::SliderFloat(label.c_str(), &m_config.manualSplits[i], 0.0f, 1.0f);
        //    }
        //}
    }
}

void CascadedShadowMap::CollectFromScene_NoCulling(int cascadeIndex, const SimpleArray<IGameObject*>& objects)
{
    m_perCascadeCollector[cascadeIndex].Clear();
    m_cascadeObjectCount[cascadeIndex] = 0;

    for (auto* obj : objects)
    {
        if (!obj->GetUse() || !obj->GetCastShadow())
            continue;

        obj->CollectShadowMesh(m_perCascadeCollector[cascadeIndex]);
        m_cascadeObjectCount[cascadeIndex]++;
    }
}

void CascadedShadowMap::CollectFromScene(int cascadeIndex, const SimpleArray<IGameObject*>& objects)
{
    m_perCascadeCollector[cascadeIndex].Clear();
    m_cascadeObjectCount[cascadeIndex] = 0;

    for (auto* obj : objects)
    {
        if (!obj->GetUse() || !obj->GetCastShadow()) continue;

		if (obj->GetModelType() == ModelType::Instanced)
		{
			// Instanced���f���̏ꍇ�A�C���X�^���X�̉������m�F
			GameObject<ModelInstance>* instancedObj = dynamic_cast<GameObject<ModelInstance>*>(obj);
			if (!instancedObj) continue; // dynamic_cast���s���̓X�L�b�v

            InstanceModelAttribute& attribute = instancedObj->GetInstancedAttribute();
			CullVisibleInstancesForShadow(attribute, cascadeIndex);
            m_cascadeObjectCount[cascadeIndex] += attribute.visibleInstanceDataArray->getSize();
		}
        else
        {
            // Static�܂���Skinned���f���̏ꍇ�AAABB�̉������m�F
            const BOUNDING_BOX& worldAABB = obj->GetBoundingBoxWorld();
            if (!IsAABBInsideLightFrustum(worldAABB, cascadeIndex))
                continue;

            m_cascadeObjectCount[cascadeIndex]++;
        }

        obj->CollectShadowMesh(m_perCascadeCollector[cascadeIndex]);
    }
}

void CascadedShadowMap::CullVisibleInstancesForShadow(InstanceModelAttribute& attribute, int cascadeIndex)
{

    // �폜�O�ɔz������Z�b�g�isize=0, capacity�ێ��j
    if (attribute.visibleInstanceDataArray)
        attribute.visibleInstanceDataArray->clear();
    else
        return; // visibleInstances��nullptr�̏ꍇ�A�������Ȃ�

    const XMMATRIX& lightViewProjLocal = m_cascadePrivateData[cascadeIndex].lightViewProjLocal;
    const XMVECTOR& frustumCenter = m_cascadePrivateData[cascadeIndex].viewFrustumCenter;

    for (UINT i = 0; i < attribute.instanceCount; ++i)
    {
        //if (attribute.visibleInstanceDraw && (*attribute.visibleInstanceDraw)[i]) continue;

        const InstanceData& inst = attribute.instanceData[i];
        XMVECTOR worldPos = XMLoadFloat3(&inst.OffsetPosition);

		if (IsAABBInsideLightFrustum((*attribute.colliderArray)[i].aabb, cascadeIndex)) // AABB�����C�g�̎���ɓ����Ă���ꍇ
        {
            attribute.visibleInstanceDataArray->push_back(inst);
            (*attribute.visibleInstanceDraw)[i] = true; // �L�^
        }
    }

    // ���C���X�^���X�����݂���ꍇ�AGPU�ɃA�b�v���[�h
    const UINT visibleCount = attribute.visibleInstanceDataArray->getSize();
    if (visibleCount > 0)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_context->Map(attribute.instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            memcpy(mapped.pData, attribute.visibleInstanceDataArray->data(), sizeof(InstanceData) * visibleCount);
            m_context->Unmap(attribute.instanceBuffer, 0);
        }
    }
}

ID3D11ShaderResourceView* CascadedShadowMap::GetShadowSRV(int lightIndex, int cascadeIndex) const
{
    return m_srvs[lightIndex][cascadeIndex];
}

ID3D11DepthStencilView* CascadedShadowMap::GetDSV(int lightIndex, int cascadeIndex) const
{
    return m_dsvs[lightIndex][cascadeIndex];
}

ID3D11ShaderResourceView** CascadedShadowMap::GetAllShadowSRVs(int lightIndex)
{
    return m_srvs[lightIndex];
}

void CascadedShadowMap::UnbindShadowSRVs(void)
{
    ID3D11ShaderResourceView* nullSRVs[MAX_CASCADES] = { nullptr, nullptr, nullptr, nullptr };
    m_ShaderResourceBinder.BindShaderResources(ShaderStage::PS, SLOT_TEX_CSM, m_config.numCascades, nullSRVs);
}

void CascadedShadowMap::BeginCascadeRender(int lightIndex, int cascadeIndex)
{
    m_context->OMGetRenderTargets(1, &m_savedRTV, &m_savedDSV);
    m_context->OMSetRenderTargets(0, nullptr, m_dsvs[lightIndex][cascadeIndex]);
    m_currentCascadeIndex = cascadeIndex;

    // DSV���N���A
    m_context->ClearDepthStencilView(m_dsvs[lightIndex][cascadeIndex], D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CascadedShadowMap::EndCascadeRender(void) 
{
    m_context->OMSetRenderTargets(1, &m_savedRTV, m_savedDSV);
    SafeRelease(&m_savedRTV);
    SafeRelease(&m_savedDSV);
}

void CascadedShadowMap::UpdateCascades(const XMVECTOR& lightDir, float nearZ, float farZ) 
{
    // �e�J�X�P�[�h�̕��������i0.0~1.0�j���i�[����z��
    float cascadeSplits[MAX_CASCADES] = {};


    float lambda = m_config.lambda;
    float range = m_config.maxShadowDistance; // ������̉��s���͈̔�
    float ratio = (nearZ + range) / nearZ; // �ΐ��X�P�[���̌v�Z�Ɏg�p

    // �J�X�P�[�h���ƂɃX�v���b�g�ʒu���v�Z
    for (int i = 0; i < m_config.numCascades; ++i)
    {
        if (m_config.useManualSplits)
        {
            float split = nearZ + m_config.manualSplits[i] * range;
            cascadeSplits[i] = (split - nearZ) / range;
        }
        else
        {

            float p = (i + 1) / static_cast<float>(m_config.numCascades);   // �����̈ʒu�i���K���j
            float logSplit = nearZ * powf(ratio, p);                        // �ΐ��X�v���b�g
            float uniSplit = nearZ + range * p;                             // ���`�X�v���b�g
            float split = lambda * logSplit + (1.0f - lambda) * uniSplit;   // �n�C�u���b�h����
            cascadeSplits[i] = (split - nearZ) / range;                     // 0�`1 �ɐ��K�����ĕۑ�
        }
    }


    float lastSplitDist = 0.0f;

    // �e�J�X�P�[�h�ɑΉ�����t���X�^���͈͂�����
    for (int i = 0; i < m_config.numCascades; ++i)
    {
        float splitDist = cascadeSplits[i];                     // ���݂̃X�v���b�g�̊����i0~1�j
        float splitNearW = nearZ + lastSplitDist * range;        // ���̃J�X�P�[�h�̋߃N���b�v
        float splitFarW = nearZ + splitDist * range;             // ���̃J�X�P�[�h�̉��N���b�v

        // NDC�ւ̐��K��(0~1)
        float splitNearNDC = (splitNearW - nearZ) / range;
        float splitFarNDC = (splitFarW - nearZ) / range;

        // ���݂̃J�X�P�[�h�ɑΉ�����V���h�E�s����v�Z
        ComputeCascadeMatrices(i, splitNearNDC, splitFarNDC, lightDir);

        // �V���h�E��r�p�̃X�v���b�g�[�x
        m_cascadeData[i].splitDepth = splitFarW;

        // ���� nearZ �p�ɍX�V
        lastSplitDist = splitDist;
    }

#ifdef _DEBUG
    AdjustCascadePadding();
#endif
}

void CascadedShadowMap::UpdateCascadeCBufferArray(void)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(m_cascadeArrayCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) return;

    CBCascadeDataArray* buffer = reinterpret_cast<CBCascadeDataArray*>(mapped.pData);

    float splits[4] = { 1e6f, 1e6f, 1e6f, 1e6f };
    for (int i = 0; i < m_config.numCascades; ++i)
    {
        buffer->cascadeArray[i] = m_cascadeData[i];

        splits[i] = m_cascadeData[i].splitDepth;
    }
    buffer->cascadeSplits = XMFLOAT4(splits[0], splits[1], splits[2], splits[3]);

    m_context->Unmap(m_cascadeArrayCBuffer, 0);

    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_CASCADE_DATA_ARRAY, m_cascadeArrayCBuffer);
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_CASCADE_DATA_ARRAY, m_cascadeArrayCBuffer);
}

void CascadedShadowMap::UpdateCascadeCBuffer(int cascadeIndex)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(m_cascadeSingleCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) return;

    memcpy(mapped.pData, &m_cascadeData[cascadeIndex], sizeof(CBCascadeData));
    m_context->Unmap(m_cascadeSingleCBuffer, 0);

    // VS�p�̏ꍇ
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_CASCADE_DATA, m_cascadeSingleCBuffer);

    // PS�ł��g�������ꍇ�͂�����ǉ�
    //m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_CASCADE_DATA, m_cascadeSingleCBuffer);
}

void CascadedShadowMap::VisualizeCascadeBounds(void)
{
    BOUNDING_BOX box = {};
    XMFLOAT3 minPt = { FLT_MAX, FLT_MAX, FLT_MAX };
    XMFLOAT3 maxPt = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (int cascadeIndex = 0; cascadeIndex < m_config.numCascades; ++cascadeIndex)
    {
        for (int i = 0; i < 8; ++i)
        {
            XMFLOAT3 pt;
            XMStoreFloat3(&pt, m_tempSliceCorners[cascadeIndex][i]);

            minPt.x = min(minPt.x, pt.x);
            minPt.y = min(minPt.y, pt.y);
            minPt.z = min(minPt.z, pt.z);

            maxPt.x = max(maxPt.x, pt.x);
            maxPt.y = max(maxPt.y, pt.y);
            maxPt.z = max(maxPt.z, pt.z);
        }

        box.minPoint = minPt;
        box.maxPoint = maxPt;

        XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

        if (cascadeIndex == 1) color = { 1.0f, 0.0f, 0.0f, 1.0f };
        else if (cascadeIndex == 2) color = { 0.0f, 1.0f, 0.0f, 1.0f };
        else if (cascadeIndex == 3) color = { 0.0f, 0.0f, 1.0f, 1.0f };

        if (m_showCascadeBox[cascadeIndex])
        {
            m_debugBoundingBoxRenderer[cascadeIndex].DrawBox(box, m_camera.GetViewProjMtx(), color);
        }
    }
}

void CascadedShadowMap::ComputeCascadeMatrices(int cascadeIndex, float splitNear, float splitFar,
                                               const XMVECTOR& lightDir) 
{

    // �J�����̃r���[���e�s��̋t�s����v�Z�iNDC -> ���[���h���W�ϊ��p�j
    XMMATRIX CSMViewProjMtx = ComputeCSMViewProjMatrixWithMaxDistance(m_config.maxShadowDistance);
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, CSMViewProjMtx);

    // ������S��(NDC��Ԃ�8�̃R�[�i�[(0~1�̐[�x)���`
    XMVECTOR ndcCorners[8] =
    {
        XMVectorSet(-1,  1, 0, 1),  // near ����
        XMVectorSet( 1,  1, 0, 1),  // near �E��
        XMVectorSet( 1, -1, 0, 1),  // near �E��
        XMVectorSet(-1, -1, 0, 1),  // near ����
        XMVectorSet(-1,  1, 1, 1),  // far ����
        XMVectorSet( 1,  1, 1, 1),  // far �E��
        XMVectorSet( 1, -1, 1, 1),  // far �E��
        XMVectorSet(-1, -1, 1, 1),  // far ����
    };

    // ���[���h��Ԃɕϊ�
    XMVECTOR frustumCornersWS[8];
    for (int i = 0; i < 8; ++i) 
    {
        frustumCornersWS[i] = XMVector3TransformCoord(ndcCorners[i], invViewProj);
    }


    // �������ꂽ������̃X���C�X���\�z(splitNear/splitFar �͈̔�)
    XMVECTOR sliceCorners[8];
    for (int i = 0; i < 4; ++i) 
    {
        // near ��(0~3) splitNear ����
        sliceCorners[i] = XMVectorLerp(frustumCornersWS[i], frustumCornersWS[i + 4], splitNear);
        // far ��(4~7) splitFar ����
        sliceCorners[i + 4] = XMVectorLerp(frustumCornersWS[i], frustumCornersWS[i + 4], splitFar);
    }

    // �����X���C�X�̒��S���v�Z
    XMVECTOR frustumCenter = XMVectorZero();
    for (int i = 0; i < 8; ++i) 
    {
        frustumCenter += sliceCorners[i];
    }
    frustumCenter /= 8.0f;

    // ���[�J���X�y�[�X�ɒ��S��
    XMVECTOR localCorners[8];
    for (int i = 0; i < 8; ++i)
        localCorners[i] = sliceCorners[i] - frustumCenter;

    // �J�X�P�[�h�X���C�X�̒��S�_�ƃR�[�i�[�z�񂪌v�Z�ς݂ł��邱�Ƃ�O��Ƃ���
    float radius = ComputeCascadeRadius(sliceCorners, frustumCenter, m_config.radiusStrategy, m_config.padding[cascadeIndex]);
    float radiusLocal = ComputeCascadeRadius(localCorners, XMVectorZero(), m_config.radiusStrategy, m_config.padding[cascadeIndex]);

    // ���C�g�r���[�s��(������������X���C�X���S�������낷)
    XMVECTOR lightPos = frustumCenter - lightDir * radius * 2.0f;
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, frustumCenter, XMVectorSet(0, 1, 0, 0));
    XMVECTOR lightPosLocal = XMVectorZero() - lightDir * radiusLocal * 2.0f;
    XMMATRIX lightViewLocal = XMMatrixLookAtLH(lightPosLocal, XMVectorZero(), XMVectorSet(0, 1, 0, 0));

    // ���ˉe�s��(�X���C�X�S�̂��J�o�[)
    XMMATRIX lightProj = ComputeLightProjMatrixFromSliceCorners(sliceCorners, lightView, m_config.padding[cascadeIndex]);
    XMMATRIX lightProjLocal = ComputeLightProjMatrixFromSliceCorners(localCorners, lightViewLocal, m_config.padding[cascadeIndex]);
    
    XMMATRIX lightViewProjLocal = lightViewLocal * lightProjLocal;
    XMMATRIX lightViewProj = lightView * lightProj;
    m_cascadePrivateData[cascadeIndex].lightViewProjLocal = lightViewProjLocal;
    m_cascadePrivateData[cascadeIndex].viewFrustumCenter = frustumCenter;
    m_lastCascadeRadius = radiusLocal;
    m_lastLightDir = lightDir;

    // �ŏI�r���[�s���ۑ�����O�ɁA�K�v�ɉ����ăX�i�b�v
    if (m_config.enableTexelSnap)
    {
        lightViewProj = SnapShadowMatrixToTexelGrid(lightView, lightProj, frustumCenter);
    }

    // �ŏI�V���h�E�s���ۑ�
	m_cascadeData[cascadeIndex].lightViewProj = XMMatrixTranspose(lightViewProj);
    m_cascadeData[cascadeIndex].lightView = XMMatrixTranspose(lightView);
    m_cascadeData[cascadeIndex].lightProj = XMMatrixTranspose(lightProj);

    // splitDepth(�J������view��ԂȂǂŎg�p����[�x臒l)
    m_cascadeData[cascadeIndex].splitDepth = splitFar; // �ʏ��splitFar���g��


#if DEBUG_CASCADE_BOUND
    m_tempFrustumCenter[cascadeIndex] = frustumCenter;
    m_tempInvViewProj = invViewProj;
    for (int i = 0; i < 8; ++i)
    {
        m_tempSliceCorners[cascadeIndex][i] = sliceCorners[i];
    }
#endif // DEBUG

}

void CascadedShadowMap::InitCascadeCBuffer(void)
{
    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(CBCascadeDataArray);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;

    m_device->CreateBuffer(&cbd, nullptr, &m_cascadeArrayCBuffer);

    cbd.ByteWidth = sizeof(CBCascadeData);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;

    m_device->CreateBuffer(&cbd, nullptr, &m_cascadeSingleCBuffer);
}

XMMATRIX CascadedShadowMap::SnapShadowMatrixToTexelGrid(const XMMATRIX& lightViewProjLocal)
{
    // �V���h�E�}�b�v�̃T�C�Y���擾
    float shadowMapSize = static_cast<float>(m_config.shadowMapSize);

    // �e�N�Z���T�C�Y���v�Z
    float texelSize = (2.0f * m_lastCascadeRadius) / shadowMapSize;

    // XY �̈ʒu�𒊏�
    float offsetX = lightViewProjLocal.r[3].m128_f32[0];
    float offsetY = lightViewProjLocal.r[3].m128_f32[1];

    float snappedX = roundf(offsetX / texelSize) * texelSize - offsetX;
    float snappedY = roundf(offsetY / texelSize) * texelSize - offsetY;

    XMMATRIX snapOffset = XMMatrixTranslation(snappedX, snappedY, 0.0f);
    return snapOffset * lightViewProjLocal;
}

XMMATRIX CascadedShadowMap::SnapShadowMatrixToTexelGrid(const XMMATRIX& lightView, const XMMATRIX& lightProj, XMVECTOR center)
{
    // �r���[�ˉe�s��̍���
    XMMATRIX lightViewProj = lightView * lightProj;

    // ���S�_�����C�g�r���[��Ԃɕϊ�
    XMVECTOR lightSpaceCenter = XMVector3TransformCoord(center, lightView);

    // �V���h�E�}�b�v�̃T�C�Y���擾
    float shadowMapSize = static_cast<float>(m_config.shadowMapSize);

    // �e�N�Z���T�C�Y���v�Z
    float texelSize = (2.0f * m_lastCascadeRadius) / shadowMapSize;

    // �e�N�Z���P�ʂɃX�i�b�v�i�i�q�ɍ��킹��j
    XMVECTOR snapped = XMVectorFloor(lightSpaceCenter / texelSize) * texelSize;

    // �X�i�b�v��̒��S�_�����[���h��Ԃɖ߂�
    XMMATRIX invLightView = XMMatrixInverse(nullptr, lightView);
    XMVECTOR newCenter = XMVector3TransformCoord(snapped, invLightView);

    // �V�����r���[�s����Čv�Z
    XMVECTOR lightPos = newCenter - m_lastLightDir * m_lastCascadeRadius * 2.0f;
    return XMMatrixLookAtLH(lightPos, newCenter, XMVectorSet(0, 1, 0, 0)) * lightProj;
}

float CascadedShadowMap::ComputeCascadeRadius(XMVECTOR* corners, XMVECTOR center, ShadowRadiusStrategy strategy, float padding)
{
    switch (strategy)
    {
    case ShadowRadiusStrategy::FitBoundingSphere:
    {
        // ���ׂẴR�[�i�[�_�ƒ��S�_�̋����̍ő�l�����߂�i���ŕ�ށj
        float r = 0.0f;
        // �ő唼�a�����߂ăV���h�E�͈͂��m��
        for (int i = 0; i < 8; ++i)
        {
            float d = XMVectorGetX(XMVector3Length(corners[i] - center));
            r = max(r, d);
        }
        // ���艻�̂��߂Ƀp�f�B���O�ƃX�i�b�v�P�ʂŐ؂�グ
        return ceilf((r + padding) * 16.0f) / 16.0f;
    }

    case ShadowRadiusStrategy::FitBoundingBox:
    {
        // AABB�i���ɉ������o�E���f�B���O�{�b�N�X�j���쐬
        XMVECTOR minPt = corners[0];
        XMVECTOR maxPt = corners[0];
        for (int i = 1; i < 8; ++i)
        {
            minPt = XMVectorMin(minPt, corners[i]);
            maxPt = XMVectorMax(maxPt, corners[i]);
        }

        XMVECTOR extent = (maxPt - minPt) * 0.5f;
        float r = max(max(XMVectorGetX(extent), XMVectorGetY(extent)), XMVectorGetZ(extent));
        return ceilf((r + padding) * 16.0f) / 16.0f;
    }

    case ShadowRadiusStrategy::PaddingBuffer:
        // �Œ�̃o�b�t�@���g���i�V���v������������j
        return padding;

    default:
        return 1.0f;
    }
}

bool CascadedShadowMap::IsAABBInsideLightFrustum(const BOUNDING_BOX& worldAABB, int cascadeIndex)
{
    // AABB��8�̒��_���擾
    XMVECTOR cornerWS[8];
    worldAABB.GetCorners(cornerWS);

    for (int i = 0; i < 8; ++i)
    {
        // �J�X�P�[�h�X���C�X�̒��S�����_�Ƃ������[�J����Ԃ֕ϊ�
        XMVECTOR localCorner = XMVectorSubtract(cornerWS[i], m_cascadePrivateData[cascadeIndex].viewFrustumCenter);

        // ���[�J����Ԃ̃��C�g�r���[�ˉe�s��ŕϊ�
        XMVECTOR clipPos = XMVector3TransformCoord(localCorner, m_cascadePrivateData[cascadeIndex].lightViewProjLocal);

        // ���K���f�o�C�X���W�iNDC�j�ɕϊ��iw�Ŋ���j
        XMVECTOR ndcPos = clipPos / XMVectorGetW(clipPos);

        float x = XMVectorGetX(ndcPos);
        float y = XMVectorGetY(ndcPos);
        float z = XMVectorGetZ(ndcPos);

        // �ЂƂł�������ɓ����Ă���΍̗p
        if (x >= -1.0f && x <= 1.0f &&
            y >= -1.0f && y <= 1.0f &&
            z >= 0.0f && z <= 1.0f)
        {
            return true;
        }
    }

    // �S�Ă̓_������O
    return false;
}


XMMATRIX CascadedShadowMap::ComputeCSMViewProjMatrixWithMaxDistance(float maxDistance)
{
    float fovY = m_camera.GetFov();
    float aspect = m_camera.GetAspectRatio();
    float nearZ = m_camera.GetNearZ();

    // ��`���ꂽmaxDistance�ŉ�����v��
    float farZ = maxDistance;

    XMMATRIX view = XMLoadFloat4x4(&m_camera.GetViewMatrix());
    XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);

    return view * proj; // ������t�ϊ����Ďg���܂�
}

//=============================================================================
// ���C�g�r���[�v���W�F�N�V�����s������Z����֐�
// �X���C�X�̃R�[�i�[�����C�g�r���[��Ԃɕϊ�����
// �ŏ��l�ƍő�l�ŕ�񂾐��K���e�s����\�z����
//=============================================================================
XMMATRIX CascadedShadowMap::ComputeLightProjMatrixFromSliceCorners(const XMVECTOR sliceCorners[8], const XMMATRIX& lightView, float padding)
{
    // ������Ԃɕϊ�
    XMVECTOR minPtLS = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
    XMVECTOR maxPtLS = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

    for (int i = 0; i < 8; ++i)
    {
        XMVECTOR cornerLS = XMVector3TransformCoord(sliceCorners[i], lightView);
        minPtLS = XMVectorMin(minPtLS, cornerLS);
        maxPtLS = XMVectorMax(maxPtLS, cornerLS);
    }

    // �p�f�B���O��K�p
    XMVECTOR padVec = XMVectorReplicate(padding);
    minPtLS = XMVectorSubtract(minPtLS, padVec);
    maxPtLS = XMVectorAdd(maxPtLS, padVec);

    // OrthographicOffCenterLH ���\�z
    float minX = XMVectorGetX(minPtLS);
    float maxX = XMVectorGetX(maxPtLS);
    float minY = XMVectorGetY(minPtLS);
    float maxY = XMVectorGetY(maxPtLS);
    float minZ = XMVectorGetZ(minPtLS);
    float maxZ = XMVectorGetZ(maxPtLS);

    return XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
}

void CascadedShadowMap::AdjustCascadePadding(void)
{
    // ��{�̃x�[�X�̃p�f�B���O�l
    float basePadding = m_config.maxShadowDistance * 0.01f; // ��: �ŉ���40,000�̏ꍇ��400.0f

    // �X���C�X�̈悪�傫���Ȃ�ɂ�Ďw���I�Ɋg��
    for (int i = 0; i < m_config.numCascades; ++i)
    {
        float factor = powf(2.0f, static_cast<float>(i));
        m_config.padding[i] = basePadding * factor;
    }
}
