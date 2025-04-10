//=============================================================================
//
// レンダリング処理 [DebugBoundingBoxRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"
#include "AABBUtils.h"

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct Vertex
{
    XMFLOAT3 position;
};

struct ConstantBuffer
{
    XMMATRIX worldViewProj;
    XMFLOAT4 color;
};

class DebugBoundingBoxRenderer
{
public:
    ~DebugBoundingBoxRenderer();

	void Initialize(void);
	void DrawBox(const BOUNDING_BOX& box, const XMMATRIX& viewProj, const XMFLOAT4& color);

private:
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_inputLayout;
    ID3D11Buffer* m_constantBuffer;

    ID3D11Device* m_device = Renderer::get_instance().GetDevice();
    ID3D11DeviceContext* m_context = Renderer::get_instance().GetDeviceContext();
};