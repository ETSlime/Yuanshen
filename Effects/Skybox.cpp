//=============================================================================
//
// Skybox���� [Skybox.cpp]
// Author : 
//
//=============================================================================
#include "Effects/Skybox.h"
#include "Core/TextureMgr.h"
#include "Core/Graphics/Renderer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
constexpr float SKYBOX_SIZE = 100.0f;
constexpr float CYCLE_DURATION = 3000.0f;

float Skybox::s_blendFactor = 0.0f;

static char* dayTextures[6] =
{
    "data/TEXTURE/SkyBox/LS cartoon 05_out_0.png",
    "data/TEXTURE/SkyBox/LS cartoon 05_out_1.png",
    "data/TEXTURE/SkyBox/LS cartoon 05_out_2.png",
    "data/TEXTURE/SkyBox/LS cartoon 05_out_3.png",
    "data/TEXTURE/SkyBox/LS cartoon 05_out_4.png",
    "data/TEXTURE/SkyBox/LS cartoon 05_out_5.png",
};
static char* nightTextures[6] =
{
    "data/TEXTURE/SkyBox/LS nigth 02_out_0.png",
    "data/TEXTURE/SkyBox/LS nigth 02_out_1.png",
    "data/TEXTURE/SkyBox/LS nigth 02_out_2.png",
    "data/TEXTURE/SkyBox/LS nigth 02_out_3.png",
    "data/TEXTURE/SkyBox/LS nigth 02_out_4.png",
    "data/TEXTURE/SkyBox/LS nigth 02_out_5.png",
};

Skybox::Skybox() 
	: m_device(Renderer::get_instance().GetDevice()), 
    m_context(Renderer::get_instance().GetDeviceContext())
{
    m_dayToNight = true;
    m_timeOfDay = 0.0f;
    Initialize();
}

Skybox::~Skybox() 
{
    SafeRelease(&m_vertexBuffer);
    SafeRelease(&m_skyboxBuffer);

    for (auto& srv : m_skyboxDaySRVs)
    {
        SafeRelease(&srv);
    }
    for (auto& srv : m_skyboxNightSRVs)
    {
        SafeRelease(&srv);
    }

}

bool Skybox::Initialize() 
{
    CreateCube();
    LoadShaders();

    for (int i = 0; i < 6; ++i)
    {
        m_skyboxDaySRVs[i] = TextureMgr::get_instance().CreateTexture(dayTextures[i]);
        if (!m_skyboxDaySRVs[i])
            return false;
    }

    for (int i = 0; i < 6; ++i)
    {
        m_skyboxNightSRVs[i] = TextureMgr::get_instance().CreateTexture(nightTextures[i]);
        if (!m_skyboxNightSRVs[i])
            return false;
    }

    D3D11_BUFFER_DESC matrixBufferDesc = {};
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(SkyBoxBuffer);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_device->CreateBuffer(&matrixBufferDesc, nullptr, &m_skyboxBuffer);

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);

    return true;
}

void Skybox::Update(void)
{
    if (m_dayToNight)
    {
        m_timeOfDay++;
        if (m_timeOfDay >= CYCLE_DURATION)
        {
            m_timeOfDay = CYCLE_DURATION;
            m_dayToNight = false;
        }
    }
    else
    {
        m_timeOfDay--;
        if (m_timeOfDay <= 0.0f)
        {
            m_timeOfDay = 0.0f;
            m_dayToNight = true;
        }
    }

}

void Skybox::CreateCube() 
{
    const float size = SKYBOX_SIZE;

    SkyBoxVertex vertices[] = 
    {
        // +X �� (faceIndex = 0)
        {{ size,  size, -size}, {1.0f, 0.0f}, 0},
        {{ size, -size, -size}, {1.0f, 1.0f}, 0},
        {{ size, -size,  size}, {0.0f, 1.0f}, 0},
        {{ size, -size,  size}, {0.0f, 1.0f}, 0},
        {{ size,  size,  size}, {0.0f, 0.0f}, 0},
        {{ size,  size, -size}, {1.0f, 0.0f}, 0},

        // -X �� (faceIndex = 2)
        {{-size,  size,  size}, {1.0f, 0.0f}, 2},
        {{-size, -size,  size}, {1.0f, 1.0f}, 2},
        {{-size, -size, -size}, {0.0f, 1.0f}, 2},
        {{-size, -size, -size}, {0.0f, 1.0f}, 2},
        {{-size,  size, -size}, {0.0f, 0.0f}, 2},
        {{-size,  size,  size}, {1.0f, 0.0f}, 2},

        // +Y �� (faceIndex = 4)
        {{-size,  size, -size}, {0.0f, 1.0f}, 4},
        {{-size,  size,  size}, {0.0f, 0.0f}, 4},
        {{ size,  size,  size}, {1.0f, 0.0f}, 4},
        {{ size,  size,  size}, {1.0f, 0.0f}, 4},
        {{ size,  size, -size}, {1.0f, 1.0f}, 4},
        {{-size,  size, -size}, {0.0f, 1.0f}, 4},

        // -Y �� (faceIndex = 5)
        {{-size, -size,  size}, {0.0f, 0.0f}, 5},
        {{-size, -size, -size}, {0.0f, 1.0f}, 5},
        {{ size, -size, -size}, {1.0f, 1.0f}, 5},
        {{ size, -size, -size}, {1.0f, 1.0f}, 5},
        {{ size, -size,  size}, {1.0f, 0.0f}, 5},
        {{-size, -size,  size}, {0.0f, 0.0f}, 5},

        // +Z �� (faceIndex = 3)
        {{-size,  size,  size}, {0.0f, 0.0f}, 3},
        {{-size, -size,  size}, {0.0f, 1.0f}, 3},
        {{ size, -size,  size}, {1.0f, 1.0f}, 3},
        {{ size, -size,  size}, {1.0f, 1.0f}, 3},
        {{ size,  size,  size}, {1.0f, 0.0f}, 3},
        {{-size,  size,  size}, {0.0f, 0.0f}, 3},

        // -Z �� (faceIndex = 1)
        {{ size,  size, -size}, {0.0f, 0.0f}, 1},
        {{ size, -size, -size}, {0.0f, 1.0f}, 1},
        {{-size, -size, -size}, {1.0f, 1.0f}, 1},
        {{-size, -size, -size}, {1.0f, 1.0f}, 1},
        {{-size,  size, -size}, {1.0f, 0.0f}, 1},
        {{ size,  size, -size}, {0.0f, 0.0f}, 1},
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(vertices);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
}

void Skybox::LoadShaders() 
{
    m_shaderSet = ShaderManager::get_instance().GetShaderSet(ShaderSetID::Skybox);
}

float Skybox::AdjustBlendFactor(float time)
{
    if (time < 0.5f) 
    {
        // ������[���i�x�߂̕ω��j
        return 8.0f * powf(time, 4.0f);
    }
    else 
    {
        // �[�������i���߂̕ω��j
        float t = (time - 0.5f) * 2.0f; // 0.0�`1.0�ɐ��K��
        return 1.0f - powf(1.0f - t, 4.0f) * 0.5f;
    }
}

void Skybox::Draw(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
{
    Renderer::get_instance().ResetRenderTarget();
    Renderer::get_instance().SetCullingMode(CULL_MODE_NONE);

    UINT stride = sizeof(SkyBoxVertex);
    UINT offset = 0;

    XMMATRIX view = viewMatrix;
    view.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // Remove translation

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(m_skyboxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    SkyBoxBuffer* dataPtr = (SkyBoxBuffer*)mappedResource.pData;
    dataPtr->view = XMMatrixTranspose(view);
    dataPtr->projection = XMMatrixTranspose(projectionMatrix);
    s_blendFactor = AdjustBlendFactor(m_timeOfDay / CYCLE_DURATION);
    dataPtr->blendFactor = s_blendFactor;
    m_context->Unmap(m_skyboxBuffer, 0);

    m_context->IASetInputLayout(m_shaderSet.inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    m_context->VSSetShader(m_shaderSet.vs, nullptr, 0);
    m_context->PSSetShader(m_shaderSet.ps, nullptr, 0);
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_SKYBOX, m_skyboxBuffer);
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_SKYBOX, m_skyboxBuffer);
    m_ShaderResourceBinder.BindShaderResources(ShaderStage::PS, SLOT_TEX_SKYBOX_DAY, 6, m_skyboxDaySRVs);
    m_ShaderResourceBinder.BindShaderResources(ShaderStage::PS, SLOT_TEX_SKYBOX_NIGHT, 6, m_skyboxNightSRVs);


    m_context->OMSetDepthStencilState(m_depthStencilState, 0);
    m_context->Draw(36, 0);
    Renderer::get_instance().SetDepthEnable(TRUE);
    Renderer::get_instance().SetCullingMode(CULL_MODE_BACK);
}