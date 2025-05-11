//=============================================================================
//
// Skybox [Skybox.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	TEXTURE_SKYBOX_PATH		"data/TEXTURE/BrightSky.dds"

//*****************************************************************************
// �\���̒�`
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
    Skybox();
    Skybox::~Skybox();

    void Update(void);
    void Draw(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix);

    static float GetCurrentDaytime(void) { return blendFactor; }

private:

    bool Initialize(void);
    void CreateCube();
    void LoadShaders();
    float AdjustBlendFactor(float time);

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;


    ShaderSet m_shaderSet;

    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_skyboxBuffer;
    ID3D11DepthStencilState* m_depthStencilState;

    ID3D11ShaderResourceView* skyboxDaySRVs[6];
    ID3D11ShaderResourceView* skyboxNightSRVs[6];

    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();

    float m_timeOfDay;
    bool dayToNight;
    static float blendFactor;
};