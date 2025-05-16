//=============================================================================
//
// ���R�����ӂ�ӂ�`���N���X����� [Environment.h]
// Author : 
// ����؁A�ԂȂǂ̃C���X�^���X�������_���z�u���āA���ɂ�������̌i�F��`�悵�܂�
// �����؁E�������E�m�C�Y�����E�V���h�E�Ή��܂ł������肵�Ă钴����΂艮�����
// 
//=============================================================================
#pragma once
#include "Renderer.h"
#include "GameObject.h"
#include "OctreeNode.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define GRASS_1_MODEL_PATH          "data/MODEL/Environment/Grass_1.obj"
#define GRASS_1_TEX_PATH            "data/MODEL/Environment/Grass_1.png"
#define GRASS_2_MODEL_PATH          "data/MODEL/Environment/Grass_2.obj"
#define GRASS_2_TEX_PATH            "data/MODEL/Environment/Grass_2.png"
#define CLOVER_1_MODEL_PATH         "data/MODEL/Environment/Clover_1.obj"
#define CLOVER_1_TEX_PATH           "data/MODEL/Environment/Clover_1.png"
#define BUSH_1_MODEL_PATH            "data/MODEL/Environment/Bush/Bush_1.obj"
#define BUSH_2_MODEL_PATH            "data/MODEL/Environment/Bush/Bush_2.obj"
#define FLOWER_1_MODEL_PATH         "data/MODEL/Environment/Flower_1.obj"
#define FLOWER_1_TEX_PATH           "data/MODEL/Environment/Flower_1.png"
#define SHRUBBERY_1_MODEL_PATH      "data/MODEL/Environment/Shrubbery_1.obj"
#define SHRUBBERY_1_TEX_PATH        "data/MODEL/Environment/Shrubbery_1.png"
#define SHRUBBERY_1_MODEL_PATH      "data/MODEL/Environment/Shrubbery_1.obj"
#define SHRUBBERY_1_TEX_PATH        "data/MODEL/Environment/Shrubbery_1.png"
#define TREE_1_MODEL_PATH           "data/MODEL/Environment/Tree.obj"
#define TREE_1_TEX_PATH             "data/MODEL/Environment/branch-01.png"

#define GRASS_MAX_SLOPE_ANGLE           (XM_PI * 0.33f)
#define CLOVER_MAX_SLOPE_ANGLE          (XM_PI * 0.33f)
#define BUSH_MAX_SLOPE_ANGLE            (XM_PI * 0.33f)
#define FLOWER_MAX_SLOPE_ANGLE          (XM_PI * 0.33f)
#define SHRUBBERY_MAX_SLOPE_ANGLE       (XM_PI * 0.33f)
#define TREE_MAX_SLOPE_ANGLE            (XM_PI * 0.33f)
//*****************************************************************************
// �\���̒�`
//*****************************************************************************


enum class EnvironmentObjectType 
{
    Grass_1,
    Grass_2,
    Bush_1,
    Bush_2,
    Flower_1,
    Shrubbery_1,
    Clover_1,
    Tree_1,
    Rock,
    FallenTree,
    Other
};

enum RotationAxis
{
    AXIS_X,
    AXIS_Y,
    AXIS_Z
};

struct InstanceVertex
{
    XMFLOAT3 Position;   // ���_�ʒu
    XMFLOAT3 Normal;     // �@��
    XMFLOAT2 TexCoord;   // �e�N�X�`�����W
    float Weight;        // ���̉e���x (�d��)
    XMFLOAT3 Tangent;    // �ؐ��i�@���}�b�v�p�j
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
    float maxSlopeAngle;
};

struct EnvironmentObject
{
    EnvironmentObjectType type;
    EnvironmentObjAttributes attributes;
	GameObject<ModelInstance>* modelGO = nullptr;
	Transform transform;

    char* modelPath = nullptr;
    char* extDiffuseTexPath = nullptr;
    int indexCount;
    int instanceCount;
    bool useExtDiffuseTex = false;

    ID3D11ShaderResourceView* externDiffuseTextureSRV = nullptr;
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    ID3D11Buffer* instanceBuffer = nullptr;
    InstanceData* instanceData = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;

    ~EnvironmentObject()
	{
		SAFE_DELETE_ARRAY(modelPath);
		SAFE_DELETE_ARRAY(extDiffuseTexPath);
        SAFE_DELETE(modelGO);
        SAFE_DELETE(instanceData);

        SafeRelease(&vertexBuffer);
        SafeRelease(&indexBuffer);
        SafeRelease(&instanceBuffer);
        SafeRelease(&externDiffuseTextureSRV);
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

struct InstanceTransformInfo
{
    float posX;
    float posZ;
    float rotY;
    float scl;

    InstanceTransformInfo()
    {
        posX = 0.0f;
        posZ = 0.0f;
        rotY = 0.0f;
        scl = 1.0f;
    }

    InstanceTransformInfo(float posX, float posZ, float rotY, float scl)
    {
        this->posX = posX;
        this->posZ = posZ;
        this->scl = scl;
        this->rotY = rotY;
    }
};

struct InstanceParams
{
    EnvironmentObjectType type;

    SimpleArray<InstanceTransformInfo> transformArray;

    bool randomScl = false;
    bool randomRotY = false;
};

class Environment
{
public:
    Environment();
    Environment(EnvironmentConfig config);
    ~Environment();

    void Update(void);
    void Draw(void);
    bool GenerateRandomInstances(EnvironmentObjectType type, const Model* fieldModel, int clusterCount = 45);
    bool GenerateInstanceByParams(const InstanceParams& params, const SkinnedMeshModel* fieldModel);

private:

    bool Initialize(void);
    void RenderEnvironmentObj(EnvironmentObject* obj);
    EnvironmentObject* InitializeEnvironmentObj(EnvironmentObjectType type);
    bool GetInitRotation(EnvironmentObject* obj, XMFLOAT4& rotation, const Triangle* triangle);
    float GetRandomScale(EnvironmentObjectType type);
    bool CreateInstanceBuffer(EnvironmentObject* obj, const SimpleArray<InstanceData>& instanceDataArray);
    // �w�肵�����𒆐S�Ɋp�x�i���W�A���j������]��ǉ�����
    XMVECTOR AddRotationToQuaternion(const XMVECTOR& baseRotation, RotationAxis axis, float angle);

    // ���I�������v�Z
    XMFLOAT3 CalculateDynamicWindDirection(float time);

    bool GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV);
    float CalculateWeight(float vertexY, float minY, float maxY);
    float RayIntersectTriangle(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, const Triangle* tri);
    bool LoadEnvironmentObj(EnvironmentObject* obj);
    void LoadShaders(EnvironmentObject* obj);

    OctreeNode* GenerateOctree(const SimpleArray<Triangle*>* triangles, BOUNDING_BOX boundingBox);

    SimpleArray<EnvironmentObject*> environmentObjects;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

    ID3D11Buffer* perFrameBuffer;

    ID3D11ShaderResourceView* shadowMapSRV;
    ID3D11ShaderResourceView* noiseTextureSRV;

    ShaderSet grassShaderSet;
    ShaderSet treeShaderSet;

    float time;

    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();
};