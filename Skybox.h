//=============================================================================
//
// Skybox [Skybox.h]
// Author : 
//
//=============================================================================
#pragma once
#include "renderer.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	TEXTURE_SKYBOX_PATH		"data/TEXTURE/BrightSky.dds"


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct SkyBoxVertex 
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
    int      faceIndex;
};

struct SkyBoxBuffer
{
    XMMATRIX view;
    XMMATRIX projection;
    float blendFactor;
};

class Skybox 
{
public:
    Skybox(ID3D11Device* device, ID3D11DeviceContext* context);
    Skybox::~Skybox();

    bool Initialize(void);
    void Update(void);
    void Draw(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix);

private:
    void CreateCube();
    void LoadShaders();

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

    ID3D11Buffer* m_vertexBuffer;
    ID3D11InputLayout* m_inputLayout;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;

    ID3D11SamplerState* m_samplerState;
    ID3D11Buffer* m_matrixBuffer;
    ID3D11DepthStencilState* m_depthStencilState;

    ID3D11ShaderResourceView* skyboxDaySRVs[6];
    ID3D11ShaderResourceView* skyboxNightSRVs[6];
    float m_timeOfDay;
};