//=============================================================================
//
// ���̕`�� [Environment.h]
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
#define BUSH_MODEL_PATH    "data/MODEL/Environment/bush.obj"
#define BUSH_TEX_PATH      "data/MODEL/Environment/branch-01.png"
#define FLOWER1_MODEL_PATH    "data/MODEL/Environment/Flower_1.obj"
#define FLOWER1_TEX_PATH      "data/MODEL/Environment/Flower_1.png"
#define SHRUBBERY_1_MODEL_PATH    "data/MODEL/Environment/Shrubbery_1.obj"
#define SHRUBBERY_1_TEX_PATH      "data/MODEL/Environment/Shrubbery_1.png"
//*****************************************************************************
// �\���̒�`
//*****************************************************************************

enum class EnvironmentObjectType 
{
    Grass,
    Bush,
    Flower_1,
    Shrubbery_1,
    Clover_1,
    Clover_2,
    Clover_3,
    Clover_4,
    Clover_5,
    Tree,
    Rock,
    FallenTree,
    Other
};

struct InstanceVertex
{
    XMFLOAT3 Position;   // ���_�ʒu
    XMFLOAT3 Normal;     // �@��
    XMFLOAT2 TexCoord;   // �e�N�X�`�����W
    float Weight;        // ���̉e���x (�d��)
};

struct InstanceData
{
    XMFLOAT3 OffsetPosition; // �C���X�^���X�ʒu�I�t�Z�b�g (���[���h���W)
    XMFLOAT4 Rotation;        // �C���X�^���X�̉�] (�l����)
    XMFLOAT4 initialBillboardRot; // �����r���{�[�h��]�p�x
    float Scale;             // �C���X�^���X�X�P�[��
    float Type;              // �C���X�^���X�̎��

    InstanceData()
    {
        OffsetPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
        Rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        initialBillboardRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        Scale = 1.0f;
        Type = 0.0f;
    }

    InstanceData(XMFLOAT3 offset, XMFLOAT4 rot, XMFLOAT4 billboardRot, float scl, float type):
        OffsetPosition(offset), Rotation(rot), initialBillboardRot(billboardRot), Scale(scl), Type(type) {}
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

struct EnvironmentObjAttributes
{
    bool use;
    bool load;
    bool affectedByWind;
    bool castShadow;
    bool collision;
    bool adaptedToTerrain;
    bool billboard;
};

struct EnvironmentObject
{
    EnvironmentObjectType type;
    EnvironmentObjAttributes attributes;
	GameObject<ModelInstance>* model;
	Transform transform;

    char* modelPath;
    char* texturePath;
    int indexCount;
    int instanceCount;

    ID3D11ShaderResourceView* diffuseTextureSRV;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* instanceBuffer;
    InstanceData* instanceData;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;

    ~EnvironmentObject()
	{
		SAFE_DELETE_ARRAY(modelPath);
		SAFE_DELETE_ARRAY(texturePath);
        SAFE_DELETE(model);
        SAFE_DELETE(instanceData);

        SafeRelease(&vertexBuffer);
        SafeRelease(&indexBuffer);
        SafeRelease(&instanceBuffer);
        SafeRelease(&diffuseTextureSRV);
	}
};

struct EnvironmentConfig
{
    bool loadGrass;
    bool loadBush;
    bool loadFlower;
    bool loadTree;
    bool loadRock;

};

class Environment
{
public:
    Environment();
    Environment(EnvironmentConfig config);
    ~Environment();

    void Update(void);
    void Draw(void);
    bool GenerateInstances(EnvironmentObjectType type, const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox, int clusterCount = 45);

private:

    bool Initialize(void);
    EnvironmentObject* InitializeEnvironmentObj(EnvironmentObjectType type);

    void RenderEnvironmentObj(EnvironmentObject* obj);

    // ���I�������v�Z
    XMFLOAT3 CalculateDynamicWindDirection(float time);

    bool GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV);
    float CalculateWeight(float vertexY, float minY, float maxY);
    float RayIntersectTriangle(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, const Triangle* tri);
    bool LoadEnvironmentObj(EnvironmentObject* obj);
    void LoadShaders(EnvironmentObject* obj);

    void CompileShaders(void);

    OctreeNode* GenerateOctree(const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox);

    SimpleArray<EnvironmentObject*> environmentObjects;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

    ID3D11Buffer* perFrameBuffer;

    ID3D11ShaderResourceView* shadowMapSRV;
    ID3D11ShaderResourceView* noiseTextureSRV;

    ID3D11InputLayout* instanceInputLayout;
    ID3D11VertexShader* grassVertexShader;
    ID3D11PixelShader* grassPixelShader;
    ID3D11VertexShader* treeVertexShader;
    ID3D11PixelShader* treePixelShader;
    ID3D11VertexShader* shadowVertexShader;

    ID3D11SamplerState* samplerState;
    ID3D11SamplerState* shadowSamplerState;
    XMFLOAT4X4 lightViewProjectionMatrix;
    float time;
};