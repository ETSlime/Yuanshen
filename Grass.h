//=============================================================================
//
// ���̕`�� [Grass.h]
// Author : 
//
//=============================================================================
#pragma once
#include "renderer.h"
#include "GameObject.h"
#include "OctreeNode.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define GRASS_MODEL_PATH    "data/MODEL/Environment/Grass_1.obj"
#define GRASS_TEX_PATH      "data/MODEL/Environment/Grass.png"
//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct GrassVertex
{
    XMFLOAT3 Position;   // ���_�ʒu
    XMFLOAT3 Normal;     // �@��
    XMFLOAT2 TexCoord;   // �e�N�X�`�����W
    float Weight;        // ���̉e���x (�d��)
};

struct InstanceData
{
    XMFLOAT3 OffsetPosition; // �C���X�^���X�ʒu�I�t�Z�b�g (���[���h���W)
    XMFLOAT4 Rotation;        // ���̉�] (�l����)
    float Scale;             // �C���X�^���X�X�P�[��

    InstanceData()
    {
        OffsetPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
        Rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        Scale = 1.0f;
    }

    InstanceData(XMFLOAT3 offset, XMFLOAT4 rot, float scl):
        OffsetPosition(offset), Rotation(rot), Scale(scl) {}
};

struct alignas(16) PerFrameBuffer 
{
    float Time;                     // �o�ߎ��� (4 bytes)
    XMFLOAT3 WindDirection;         // ������ (12 bytes)
    float WindStrength;             // ���̋��� (4 bytes)
    float padding1;                 // �p�f�B���O (4 bytes) - 16�o�C�g����p
    XMFLOAT2 NoiseTextureResolution; // �m�C�Y�e�N�X�`���̉𑜓x (8 bytes)
    XMFLOAT2 padding2;              // �p�f�B���O (8 bytes) - 16�o�C�g����p
};



class Grass
{
public:
    Grass();
    ~Grass();

    void Update(void);
    void Draw(void);
    bool CreateInstanceBuffer(void);
    bool GenerateGrassInstances(const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox, int grassClusterCount = 50);

private:

    bool Initialize(void);
    // ���I�������v�Z
    void RenderShadowPass(void);

    XMFLOAT3 CalculateDynamicWindDirection(float time);

    bool GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV);
    float CalculateWeight(float vertexY, float minY, float maxY);
    float RayIntersectTriangle(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, const Triangle* tri);
    void LoadModel(void);
    void LoadShaders(void);

    OctreeNode* GenerateOctree(const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox);


    GameObject<ModelInstance>* model;
    Transform transform;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* instanceBuffer;
    int indexCount;
    ID3D11Buffer* perFrameBuffer;
    ID3D11ShaderResourceView* diffuseTextureSRV;
    ID3D11ShaderResourceView* shadowMapSRV;
    ID3D11ShaderResourceView* noiseTextureSRV;

    InstanceData* instanceData;
    int instanceCount;

    ID3D11InputLayout* m_inputLayout;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11VertexShader* m_shadowVertexShader;

    ID3D11SamplerState* samplerState;
    ID3D11SamplerState* shadowSamplerState;
    XMFLOAT4X4 lightViewProjectionMatrix;
    float time;
};