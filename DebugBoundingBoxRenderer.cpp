//=============================================================================
//
// ƒŒƒ“ƒ_ƒŠƒ“ƒOˆ— [DebugBoundingBoxRenderer.cpp]
// Author : 
//
//=============================================================================
#include "DebugBoundingBoxRenderer.h"

//=============================================================================
// ‰Šú‰»ˆ—
//=============================================================================
void DebugBoundingBoxRenderer::Initialize(void)
{
    // 8 corner vertices
    Vertex vertices[] = 
    {
        {{-0.5f, -0.5f, -0.5f}},
        {{-0.5f, +0.5f, -0.5f}},
        {{+0.5f, +0.5f, -0.5f}},
        {{+0.5f, -0.5f, -0.5f}},
        {{-0.5f, -0.5f, +0.5f}},
        {{-0.5f, +0.5f, +0.5f}},
        {{+0.5f, +0.5f, +0.5f}},
        {{+0.5f, -0.5f, +0.5f}},
    };

    uint16_t indices[] = 
    {
        0,1, 1,2, 2,3, 3,0, // bottom
        4,5, 5,6, 6,7, 7,4, // top
        0,4, 1,5, 2,6, 3,7  // sides
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    m_device->CreateBuffer(&vbDesc, &vbData, &m_vertexBuffer);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    m_device->CreateBuffer(&ibDesc, &ibData, &m_indexBuffer);

    // Compile and create shaders
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlob, * pPSBlob;
    ID3DBlob* pErrorBlob = NULL;
    hr = D3DX11CompileFromFile("DebugBox.hlsl", NULL, NULL, "VS", "vs_4_0", 0, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
    }

    hr = D3DX11CompileFromFile("DebugBox.hlsl", NULL, NULL, "PS", "ps_4_0", 0, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
    }
    m_device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_vertexShader);
    m_device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pixelShader);

    // Input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    m_device->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_inputLayout);

    // Constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    pVSBlob->Release();
    pPSBlob->Release();
}

void DebugBoundingBoxRenderer::DrawBox(const BOUNDING_BOX& box, const XMMATRIX& viewProj, const XMFLOAT4& color)
{
    XMVECTOR minPt = XMLoadFloat3(&box.minPoint);
    XMVECTOR maxPt = XMLoadFloat3(&box.maxPoint);
    XMVECTOR center = (minPt + maxPt) * 0.5f;
    XMVECTOR size = (maxPt - minPt);

    XMMATRIX world = XMMatrixScalingFromVector(size) * XMMatrixTranslationFromVector(center);
    XMMATRIX wvp = XMMatrixTranspose(world * viewProj);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    ConstantBuffer* cb = reinterpret_cast<ConstantBuffer*>(mapped.pData);
    cb->worldViewProj = wvp;
    cb->color = color;
    m_context->Unmap(m_constantBuffer, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    m_context->VSSetShader(m_vertexShader, nullptr, 0);
    m_context->VSSetConstantBuffers(9, 1, &m_constantBuffer);
    m_context->PSSetShader(m_pixelShader, nullptr, 0);
    m_context->PSSetConstantBuffers(9, 1, &m_constantBuffer);

    m_context->DrawIndexed(24, 0, 0);
}

DebugBoundingBoxRenderer::~DebugBoundingBoxRenderer()
{
    SafeRelease(&m_vertexBuffer);
    SafeRelease(&m_indexBuffer);
    SafeRelease(&m_constantBuffer);
    SafeRelease(&m_vertexShader);
    SafeRelease(&m_pixelShader);
    SafeRelease(&m_inputLayout);
}