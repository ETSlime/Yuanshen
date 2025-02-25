//=============================================================================
//
// Skyboxˆ— [Skybox.cpp]
// Author : 
//
//=============================================================================
#include "Skybox.h"
#include "TextureMgr.h"
#include "renderer.h"
//*****************************************************************************
// ƒ}ƒNƒ’è‹`
//*****************************************************************************
constexpr float SKYBOX_SIZE = 100.0f;
constexpr float CYCLE_DURATION = 500.0f;
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

Skybox::Skybox(ID3D11Device* device, ID3D11DeviceContext* context) 
	: m_device(device), m_context(context) 
{
    Initialize();
}

Skybox::~Skybox() 
{
    if (m_vertexBuffer) m_vertexBuffer->Release();
    if (m_inputLayout) m_inputLayout->Release();
    if (m_vertexShader) m_vertexShader->Release();
    if (m_pixelShader) m_pixelShader->Release();
    if (m_samplerState) m_samplerState->Release();
    for (auto& srv : skyboxDaySRVs)
    {
        if (srv) srv->Release();
    }
    for (auto& srv : skyboxNightSRVs)
    {
        if (srv) srv->Release();
    }
}

bool Skybox::Initialize() 
{
    CreateCube();
    LoadShaders();

    for (int i = 0; i < 6; ++i)
    {
        skyboxDaySRVs[i] = TextureMgr::get_instance().CreateTexture(dayTextures[i]);
        if (!skyboxDaySRVs[i])
            return false;
    }

    for (int i = 0; i < 6; ++i)
    {
        skyboxNightSRVs[i] = TextureMgr::get_instance().CreateTexture(nightTextures[i]);
        if (!skyboxNightSRVs[i])
            return false;
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    m_device->CreateSamplerState(&samplerDesc, &m_samplerState);

    D3D11_BUFFER_DESC matrixBufferDesc = {};
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(SkyBoxBuffer);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_device->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);

    return true;
}

void Skybox::Update(void)
{
    m_timeOfDay++;
    if (m_timeOfDay > CYCLE_DURATION)
        m_timeOfDay -= CYCLE_DURATION;
}

void Skybox::CreateCube() 
{
    const float size = SKYBOX_SIZE;

    SkyBoxVertex vertices[] = 
    {
        // +X –Ê (faceIndex = 0)
        {{ size,  size, -size}, {1.0f, 0.0f}, 0},
        {{ size, -size, -size}, {1.0f, 1.0f}, 0},
        {{ size, -size,  size}, {0.0f, 1.0f}, 0},
        {{ size, -size,  size}, {0.0f, 1.0f}, 0},
        {{ size,  size,  size}, {0.0f, 0.0f}, 0},
        {{ size,  size, -size}, {1.0f, 0.0f}, 0},

        // -X –Ê (faceIndex = 1)
        {{-size,  size,  size}, {1.0f, 0.0f}, 1},
        {{-size, -size,  size}, {1.0f, 1.0f}, 1},
        {{-size, -size, -size}, {0.0f, 1.0f}, 1},
        {{-size, -size, -size}, {0.0f, 1.0f}, 1},
        {{-size,  size, -size}, {0.0f, 0.0f}, 1},
        {{-size,  size,  size}, {1.0f, 0.0f}, 1},

        // +Y –Ê (faceIndex = 2)
        {{-size,  size, -size}, {0.0f, 1.0f}, 2},
        {{-size,  size,  size}, {0.0f, 0.0f}, 2},
        {{ size,  size,  size}, {1.0f, 0.0f}, 2},
        {{ size,  size,  size}, {1.0f, 0.0f}, 2},
        {{ size,  size, -size}, {1.0f, 1.0f}, 2},
        {{-size,  size, -size}, {0.0f, 1.0f}, 2},

        // -Y –Ê (faceIndex = 3)
        {{-size, -size,  size}, {0.0f, 0.0f}, 3},
        {{-size, -size, -size}, {0.0f, 1.0f}, 3},
        {{ size, -size, -size}, {1.0f, 1.0f}, 3},
        {{ size, -size, -size}, {1.0f, 1.0f}, 3},
        {{ size, -size,  size}, {1.0f, 0.0f}, 3},
        {{-size, -size,  size}, {0.0f, 0.0f}, 3},

        // +Z –Ê (faceIndex = 4)
        {{-size,  size,  size}, {1.0f, 0.0f}, 4},
        {{-size, -size,  size}, {1.0f, 1.0f}, 4},
        {{ size, -size,  size}, {0.0f, 1.0f}, 4},
        {{ size, -size,  size}, {0.0f, 1.0f}, 4},
        {{ size,  size,  size}, {0.0f, 0.0f}, 4},
        {{-size,  size,  size}, {1.0f, 0.0f}, 4},

        // -Z –Ê (faceIndex = 5)
        {{ size,  size, -size}, {1.0f, 0.0f}, 5},
        {{ size, -size, -size}, {1.0f, 1.0f}, 5},
        {{-size, -size, -size}, {0.0f, 1.0f}, 5},
        {{-size, -size, -size}, {0.0f, 1.0f}, 5},
        {{-size,  size, -size}, {0.0f, 0.0f}, 5},
        {{ size,  size, -size}, {1.0f, 0.0f}, 5},
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
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlob, * pPSBlob;
    ID3DBlob* pErrorBlob = NULL;
    hr = D3DX11CompileFromFile("Skybox.hlsl", NULL, NULL, "VS", "vs_4_0", 0, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
    }

    hr = D3DX11CompileFromFile("Skybox.hlsl", NULL, NULL, "PS", "ps_4_0", 0, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
    }


    m_device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_vertexShader);
    m_device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = { 
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}, 
    };
    m_device->CreateInputLayout(layout, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_inputLayout);

    pVSBlob->Release();
    pPSBlob->Release();
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
    m_context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    SkyBoxBuffer* dataPtr = (SkyBoxBuffer*)mappedResource.pData;
    dataPtr->view = XMMatrixTranspose(view);
    dataPtr->projection = XMMatrixTranspose(projectionMatrix);
    float blendFactor = abs(sin(m_timeOfDay / CYCLE_DURATION * DirectX::XM_PI));
    //dataPtr->blendFactor = blendFactor;
    m_context->Unmap(m_matrixBuffer, 0);

    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    m_context->VSSetShader(m_vertexShader, nullptr, 0);
    m_context->VSSetConstantBuffers(12, 1, &m_matrixBuffer);
    m_context->PSSetShader(m_pixelShader, nullptr, 0);
    m_context->PSSetShaderResources(0, 6, skyboxDaySRVs);
    m_context->PSSetShaderResources(6, 6, skyboxNightSRVs);
    m_context->PSSetSamplers(2, 1, &m_samplerState);

    m_context->OMSetDepthStencilState(m_depthStencilState, 0);
    m_context->Draw(36, 0);
    Renderer::get_instance().SetDepthEnable(TRUE);
    Renderer::get_instance().SetCullingMode(CULL_MODE_BACK);
}