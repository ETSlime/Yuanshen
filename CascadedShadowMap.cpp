//=============================================================================
//
// �����_�����O���� [CascadedShadowMap.cpp]
// Author : 
//
//=============================================================================
#include "CascadedShadowMap.h"

CascadedShadowMap::CascadedShadowMap() {}
CascadedShadowMap::~CascadedShadowMap() { Shutdown(); }

void CascadedShadowMap::Initialize(int shadowMapSize, int numCascades) 
{
    m_device = Renderer::get_instance().GetDevice();
    m_context = Renderer::get_instance().GetDeviceContext();

    assert(numCascades <= MaxCascades);

    m_shadowMapSize = shadowMapSize;
    m_numCascades = max(1, numCascades); // 1�ȏ�ɐ���

    m_shadowViewport = {};
    m_shadowViewport.TopLeftX = 0.0f;
    m_shadowViewport.TopLeftY = 0.0f;
    m_shadowViewport.Width = static_cast<float>(shadowMapSize);
    m_shadowViewport.Height = static_cast<float>(shadowMapSize);
    m_shadowViewport.MinDepth = 0.0f;
    m_shadowViewport.MaxDepth = 1.0f;

    for (int i = 0; i < numCascades; ++i) 
    {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = shadowMapSize;
        texDesc.Height = shadowMapSize;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        m_device->CreateTexture2D(&texDesc, nullptr, &m_shadowMaps[i]);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        m_device->CreateDepthStencilView(m_shadowMaps[i], &dsvDesc, &m_dsvs[i]);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(m_shadowMaps[i], &srvDesc, &m_srvs[i]);
    }
}

void CascadedShadowMap::Shutdown() 
{
    for (int i = 0; i < MaxCascades; ++i) 
    {
        SafeRelease(&m_shadowMaps[i]);
        SafeRelease(&m_dsvs[i]);
        SafeRelease(&m_srvs[i]);
    }
}

void CascadedShadowMap::SetViewport(void) 
{
    m_context->RSSetViewports(1, &m_shadowViewport);
}

void CascadedShadowMap::BeginCascadeRender(int cascadeIndex) 
{
    m_context->OMGetRenderTargets(1, &m_prevRTV, &m_prevDSV);
    m_context->OMSetRenderTargets(0, nullptr, m_dsvs[cascadeIndex]);
    m_currentCascadeIndex = cascadeIndex;

}

void CascadedShadowMap::EndCascadeRender(void) 
{
    m_context->OMSetRenderTargets(1, &m_prevRTV, m_prevDSV);
    if (m_prevRTV) m_prevRTV->Release();
    if (m_prevDSV) m_prevDSV->Release();
    m_prevRTV = nullptr;
    m_prevDSV = nullptr;
}

void CascadedShadowMap::UpdateCascades(const XMMATRIX& camView, const XMMATRIX& camProj, const XMVECTOR& lightDir, float nearZ, float farZ) 
{
    // �e�J�X�P�[�h�̕��������i0.0�`1.0�j���i�[����z��
    float cascadeSplits[MaxCascades] = {};

    // �����̃^�C�v�𐧌䂷��W���i0.0 = ���`�A1.0 = �ΐ��A0.95�Ȃ�قڑΐ��j
    float lambda = 0.95f;
    float range = farZ - nearZ; // ������̉��s���͈̔�
    float ratio = farZ / nearZ; // �ΐ��X�P�[���̌v�Z�Ɏg�p

    if (m_config.useManualSplits)
    {
        for (int i = 0; i < m_numCascades; ++i)
            cascadeSplits[i] = m_config.manualSplits[i];
    }
    else
    {
        // �J�X�P�[�h���ƂɃX�v���b�g�ʒu���v�Z
        for (int i = 0; i < m_numCascades; ++i)
        {
            float p = (i + 1) / static_cast<float>(m_numCascades);          // �����̈ʒu�i���K���j
            float logSplit = nearZ * powf(ratio, p);                        // �ΐ��X�v���b�g
            float uniSplit = nearZ + range * p;                             // ���`�X�v���b�g
            float split = lambda * logSplit + (1.0f - lambda) * uniSplit;   // �n�C�u���b�h����
            cascadeSplits[i] = (split - nearZ) / range;                     // 0�`1 �ɐ��K�����ĕۑ�
        }
    }

    float lastSplitDist = 0.0f;

    // �e�J�X�P�[�h�ɑΉ�����t���X�^���͈͂�����
    for (int i = 0; i < m_numCascades; ++i) 
    {
        float splitDist = cascadeSplits[i];                     // ���݂̃X�v���b�g�̊����i0�`1�j
        float splitNear = nearZ + lastSplitDist * range;        // ���̃J�X�P�[�h�̋߃N���b�v
        float splitFar = nearZ + splitDist * range;             // ���̃J�X�P�[�h�̉��N���b�v

        // ���݂̃J�X�P�[�h�ɑΉ�����V���h�E�s����v�Z
        ComputeCascadeMatrices(i, nearZ + lastSplitDist * range, splitFar, camView, camProj, lightDir);
        // �V���h�E��r�p�̃X�v���b�g�[�x
        m_cascadeData[i].splitDepth = nearZ + splitDist * range;
        // ���� nearZ �p�ɍX�V
        lastSplitDist = splitDist;
    }
}

void CascadedShadowMap::ComputeCascadeMatrices(int index, float splitNear, float splitFar,
                                               const XMMATRIX& camView, const XMMATRIX& camProj,
                                               const XMVECTOR& lightDir) 
{
    // �J�����̃r���[���e�s��̋t�s����v�Z�iNDC -> ���[���h���W�ϊ��p�j
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, camView * camProj);

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
        sliceCorners[i] = XMVectorLerp(frustumCornersWS[i], frustumCornersWS[i + 4], (splitNear - 0.0f) / (1.0f - 0.0f));
        // far ��(4~7) splitFar ����
        sliceCorners[i + 4] = XMVectorLerp(frustumCornersWS[i], frustumCornersWS[i + 4], (splitFar - 0.0f) / (1.0f - 0.0f));
    }

    // �����X���C�X�̒��S���v�Z
    XMVECTOR frustumCenter = XMVectorZero();
    for (int i = 0; i < 8; ++i) 
    {
        frustumCenter += sliceCorners[i];
    }
    frustumCenter /= 8.0f;


    // �J�X�P�[�h�X���C�X�̒��S�_�ƃR�[�i�[�z�񂪌v�Z�ς݂ł��邱�Ƃ�O��Ƃ���
    float radius = ComputeCascadeRadius(sliceCorners, frustumCenter, m_config.radiusStrategy, m_config.padding);

    // ���C�g�r���[�s��(������������X���C�X���S�������낷)
    XMVECTOR lightPos = frustumCenter - lightDir * radius * 2.0f;
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, frustumCenter, XMVectorSet(0, 1, 0, 0));

    // ���ˉe�s��(�X���C�X�S�̂��J�o�[)
    XMMATRIX lightProj = XMMatrixOrthographicLH(radius * 2.0f, radius * 2.0f, 0.0f, radius * 4.0f);

    // �ŏI�r���[�s���ۑ�����O�ɁA�K�v�ɉ����ăX�i�b�v
    m_lastCascadeRadius = radius;
    m_lastLightDir = lightDir;

    // �ŏI�V���h�E�s���ۑ�
    if (m_config.enableTexelSnap)
    {
        m_cascadeData[index].lightViewProj = SnapShadowMatrixToTexelGrid(lightView, lightProj, frustumCenter);
    }
    else
    {
        m_cascadeData[index].lightViewProj = lightView * lightProj;
    }
    m_cascadeData[index].lightViewProj = lightView * lightProj;
}

XMMATRIX CascadedShadowMap::SnapShadowMatrixToTexelGrid(const XMMATRIX& lightView, const XMMATRIX& lightProj, XMVECTOR center)
{
    // �r���[�ˉe�s��̍���
    XMMATRIX lightViewProj = lightView * lightProj;

    // ���S�_�����C�g�r���[��Ԃɕϊ�
    XMVECTOR lightSpaceCenter = XMVector3TransformCoord(center, lightView);

    // �V���h�E�}�b�v�̃T�C�Y���擾
    float shadowMapSize = static_cast<float>(m_shadowMapSize);

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
