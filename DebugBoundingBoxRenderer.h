//=============================================================================
//
// レンダリング処理 [DebugBoundingBoxRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"
#include "AABBUtils.h"
#include "Debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define DEBUG_BOUNDING_BOX_NAME_LENGTH 64

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

class DebugBoundingBoxRenderer : public IDebugUI
{
public:
    ~DebugBoundingBoxRenderer();

	void Initialize(char* name = "Debug BoundingBox");
	void DrawBox(const BOUNDING_BOX& box, const XMMATRIX& viewProj, const XMFLOAT4& color);

    virtual void RenderImGui(void) override;
    virtual const char* GetPanelName(void) const override { return m_boxName; };

private:
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    ID3D11Buffer* m_constantBuffer;

    XMFLOAT3 m_center = {};
    XMFLOAT3 m_size = {};
    char m_boxName[DEBUG_BOUNDING_BOX_NAME_LENGTH];

    ShaderSet debugShaderSet;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();
};