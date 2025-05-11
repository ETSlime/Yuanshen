//=============================================================================
//
// �����_�����O���� [Renderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"
#include "ShaderManager.h"
#include "ShaderResourceBinder.h"

//*********************************************************
// �}�N����`
//*********************************************************
#define LIGHT_MAX			(5)
#define BONE_MAX			(512)
#define MAX_BONE_INDICES	(4)
#define SHADOWMAP_SIZE		(SCREEN_WIDTH * 3.5f)
#define DEPTHBIAS_LAYER_0	(-25)
#define DEPTHBIAS_LAYER_1	(-50)
#define DEPTHBIAS_LAYER_2	(-100)
#define DEPTHBIAS_LAYER_3	(-200)
//*********************************************************

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//�u�����h����
	BLEND_MODE_ALPHABLEND,	//���u�����h
	BLEND_MODE_ADD,			//���Z�u�����h
	BLEND_MODE_SUBTRACT,	//���Z�u�����h
	BLEND_MODE_SWORDTRAIL,

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//�J�����O����
	CULL_MODE_FRONT,		//�\�̃|���S����`�悵�Ȃ�(CW)
	CULL_MODE_BACK,			//���̃|���S����`�悵�Ȃ�(CCW)

	CULL_MODE_NUM
};

enum class DepthMode
{
	Enable,			// �[�x�e�X�gON + ��������ON�i�ʏ�̃��f���p�j
	Particle,		// �[�x�e�X�gON + ��������OFF�i�p�[�e�B�N���p�j
	Disable			// �[�x�e�X�gOFF�iUI�ȂǂɎg�p�j
};

enum class RenderMode
{
	OBJ,
	SKINNED_MESH,
	INSTANCE,
	UI,
	VFX,
};

enum class RenderLayer
{
	DEFAULT,
	LAYER_0,
	LAYER_1,
	LAYER_2,
	LAYER_3,
};

//*********************************************************
// �\����
//*********************************************************

// ���_�\����
struct VERTEX_3D
{
    XMFLOAT3	Position;
    XMFLOAT3	Normal;
    XMFLOAT4	Diffuse;
    XMFLOAT2	TexCoord;
	XMFLOAT3	Tangent;

	VERTEX_3D()
	{
		Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		TexCoord = XMFLOAT2(0.0f, 0.0f);
	}

	VERTEX_3D(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT3 tangent, XMFLOAT4 dif, XMFLOAT2 tex)
	{
		Position = pos;
		Normal = norm;
		Tangent = tangent;
		Diffuse = dif;
		TexCoord = tex;
	}
};

struct SKINNED_VERTEX_3D
{
	XMFLOAT3	Position;
	XMFLOAT3	Normal;
	XMFLOAT3	Tangent;
	XMFLOAT3	Bitangent;
	XMFLOAT4	Diffuse;
	XMFLOAT2	TexCoord;
	XMFLOAT4	Weights;// [MAX_BONE_INDICES] ;
	XMFLOAT4	BoneIndices;// [MAX_BONE_INDICES] ;

	SKINNED_VERTEX_3D()
	{
		Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		TexCoord = XMFLOAT2(0.0f, 0.0f);
		Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Bitangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
		//for (int i = 0; i < MAX_BONE_INDICES; i++)
		//{
		//	Weights[i] = 0.0f;
		//	BoneIndices[i] = 0;
		//}
		Weights = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		BoneIndices = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	SKINNED_VERTEX_3D(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT3 tangent, XMFLOAT4 dif, XMFLOAT2 tex)
	{
		Position = pos;
		Normal = norm;
		Tangent = tangent;
		Diffuse = dif;
		TexCoord = tex;
		//for (int i = 0; i < MAX_BONE_INDICES; i++)
		//{
		//	Weights[i] = 0.0f;
		//	BoneIndices[i] = 0;
		//}
		Bitangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Weights = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		BoneIndices = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
};

// UI�`��p�̒��_�\����
struct UIVertex
{
	XMFLOAT2 Position; // �ʒu�i�X�N���[�����W�j
	XMFLOAT2 TexCoord; // �e�N�X�`�����W
	XMFLOAT4 Color;
};


struct VFXVertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT4 color;
};

struct WorldMatrixBuffer
{
	XMMATRIX world;
	XMMATRIX invWorld;
};

// �}�e���A���\����
struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
	int			normalMapSampling;
	int			bumpMapSampling;
	int			opacityMapSampling;
	int			lightMapSampling;
	int			reflectMapSampling;
	int			translucencyMapSampling;
	BOOL		LoadMaterial;

	MATERIAL()
	{
		noTexSampling = 1;
		lightMapSampling = 0;
		normalMapSampling = 0;
		bumpMapSampling = 0;
		opacityMapSampling = 0;
		reflectMapSampling = 0;
		translucencyMapSampling = 0;
		LoadMaterial = FALSE;
		Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Shininess = 0.0f;
	}
};

// ���C�g�\����
struct LightData
{
	XMFLOAT3	Direction;	// ���C�g�̕���
	XMFLOAT3	Position;	// ���C�g�̈ʒu
	XMFLOAT4	Diffuse;	// �g�U���̐F
	XMFLOAT4	Ambient;	// �����̐F
	float		Attenuation;// ������
	int			Type;		// ���C�g��ʁE�L���t���O
	int			Enable;		// ���C�g��ʁE�L���t���O

	XMFLOAT4X4	LightViewProj;
};

// �t�H�O�\����
struct FOG 
{
	float		FogStart;	// �t�H�O�̊J�n����
	float		FogEnd;		// �t�H�O�̍ő勗��
	XMFLOAT4	FogColor;	// �t�H�O�̐F
};

// �}�e���A���p�萔�o�b�t�@�\����
struct MATERIAL_CBUFFER
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
	int			lightMapSampling;
	int			normalMapSampling;
	int			bumpMapSampling;
	int			opacityMapSampling;
	int			reflectMapSampling;
	int			translucencyMapSampling;
	//float		Dummy[1];				// 16byte���E�p
};

// ���C�g�p�t���O�\����
struct LIGHTFLAGS
{
	int			Type;		//���C�g�^�C�v�ienum LIGHT_TYPE�j
	int         OnOff;		//���C�g�̃I��or�I�t�X�C�b�`
	int			Dummy[2];
};

// ���C�g�p�萔�o�b�t�@�\����
struct LIGHT_CBUFFER
{
	XMFLOAT4	Direction[LIGHT_MAX];	// ���C�g�̕���
	XMFLOAT4	Position[LIGHT_MAX];	// ���C�g�̈ʒu
	XMFLOAT4	Diffuse[LIGHT_MAX];		// �g�U���̐F
	XMFLOAT4	Ambient[LIGHT_MAX];		// �����̐F
	XMFLOAT4	Attenuation[LIGHT_MAX];	// ������
	LIGHTFLAGS	Flags[LIGHT_MAX];		// ���C�g���
	XMFLOAT4X4 	LightViewProj[LIGHT_MAX];
	int			Enable;					// ���C�e�B���O�L���E�����t���O
	int			Dummy[3];				// 16byte���E�p
};

// �t�H�O�p�萔�o�b�t�@�\����
struct FOG_CBUFFER
{
	XMFLOAT4	Fog;					// �t�H�O��
	XMFLOAT4	FogColor;				// �t�H�O�̐F
	int			Enable;					// �t�H�O�L���E�����t���O
	float		Dummy[3];				// 16byte���E�p
};

// �����p�o�b�t�@
struct FUCHI
{
	int			fuchi;
	int			fill[3];
};

struct LIGHTMODE_CBUFFER
{
	int			mode;
	int			padding[3];
};

struct BoneMatrices
{
	XMMATRIX bones[BONE_MAX];
};

struct RenderProgressBuffer
{
	float progress = 0.0f;
	int isRandomFade = true;
	XMFLOAT2 padding;
};

struct MeshRenderData 
{
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	UINT stride;
	UINT indexCount;
	DXGI_FORMAT indexFormat;
	XMMATRIX worldMatrix;

	ID3D11ShaderResourceView* opacityMapSRV = nullptr;
	bool enableAlphaTest = false;
};

struct StaticRenderData : public MeshRenderData
{
	UINT startIndexLocation = 0;
};

struct SkinnedRenderData : public MeshRenderData
{
	const XMMATRIX* pBoneMatrices = nullptr;
};

struct InstancedRenderData : public MeshRenderData
{
	ID3D11Buffer* instanceBuffer = nullptr;
	UINT instanceCount = 0;
	UINT startIndexLocation = 0;
	UINT instanceStride = 0; // �C���X�^���X�X�g���C�h�ǉ�
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
class Renderer : public SingletonBase<Renderer>
{
public:

	Renderer();

	HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
	void Uninit(void);

	void Clear(void);
	void Present(void);

	ID3D11Device* GetDevice(void);
	ID3D11DeviceContext* GetDeviceContext(void);

	void SetDepthEnable(BOOL Enable);
	void SetDepthForParticle(void);
	void SetDepthMode(DepthMode mode);
	void SetBlendState(BLEND_MODE bm);
	void SetCullingMode(CULL_MODE cm);
	void SetAlphaTestEnable(BOOL flag);

	void SetWorldViewProjection2D(void);
	void SetCurrentWorldMatrix(const XMMATRIX* WorldMatrix) const;
	void SetViewMatrix(const XMMATRIX* ViewMatrix) const;
	void SetProjectionMatrix(const XMMATRIX* ProjectionMatrix) const;

	void SetMaterial(MATERIAL material);

	void SetFogEnable(BOOL flag);
	void SetFog(FOG* fog);

	void SetRenderProgress(RenderProgressBuffer renderProgress);

	void DebugTextOut(char* text, int x, int y);

	void SetFuchi(int flag);
	void SetShaderCamera(XMFLOAT3 pos);
	void SetFillMode(D3D11_FILL_MODE mode);
	void SetBoneMatrix(const XMMATRIX matrices[BONE_MAX]) const;
	void SetClearColor(float* color4);
	void SetRenderLayer(RenderLayer layer);
	void SetRenderObject(void);
	void SetRenderSkinnedMeshModel(void);
	void SetRenderInstance(void);
	void SetRenderVFX(void);
	void SetRenderUI(void);
	void SetStaticModelInputLayout(void);
	void SetUIInputLayout(void);
	void SetSkinnedMeshInputLayout(void);
	void SetVFXInputLayout(void);
	void ResetRenderTarget(void);
	void SetLightModeBuffer(int mode);
	void SetLightBuffer(const LIGHT_CBUFFER& lightBuffer);

	void SetMainPassViewport(void);
	void SetShadersets(void);

	RenderMode GetRenderMode(void);
	void SetRenderMode(RenderMode mode);

	void BindViewBuffer(ShaderStage stage);


private:

	void SetFogBuffer(void);

	D3D_FEATURE_LEVEL       g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device* g_D3DDevice = NULL;
	ID3D11DeviceContext* g_ImmediateContext = NULL;
	IDXGISwapChain* g_SwapChain = NULL;

	ID3D11RenderTargetView* g_RenderTargetView = NULL;
	ID3D11DepthStencilView* g_SceneDepthStencilView = NULL;

	ID3D11Buffer* g_WorldBuffer = NULL;
	ID3D11Buffer* g_ViewBuffer = NULL;
	ID3D11Buffer* g_ProjectionBuffer = NULL;
	ID3D11Buffer* g_MaterialBuffer = NULL;
	ID3D11Buffer* g_LightBuffer = NULL;
	ID3D11Buffer* g_FogBuffer = NULL;
	ID3D11Buffer* g_FuchiBuffer = NULL;
	ID3D11Buffer* g_CameraPosBuffer = NULL;
	ID3D11Buffer* g_LightProjViewBuffer = NULL;
	ID3D11Buffer* g_BoneMatrixBuffer = NULL;
	ID3D11Buffer* g_LightModeBuffer = NULL;
	ID3D11Buffer* g_RenderProgressBuffer = NULL;


	ID3D11DepthStencilState* g_DepthStateEnable = NULL;
	ID3D11DepthStencilState* g_DepthStateDisable = NULL;
	ID3D11DepthStencilState* g_DepthStateParticle = NULL;

	ID3D11BlendState* g_BlendStateNone = NULL;
	ID3D11BlendState* g_BlendStateAlphaBlend = NULL;
	ID3D11BlendState* g_BlendStateAdd = NULL;
	ID3D11BlendState* g_BlendStateSubtract = NULL;
	ID3D11BlendState* g_BlendStateSwordTrail = NULL;
	BLEND_MODE				g_BlendStateParam;

	ID3D11SamplerState* g_SamplerState = NULL;
	ID3D11SamplerState* g_SamplerStateShadow = NULL;
	ID3D11SamplerState* g_SamplerStateOpacity = NULL;

	ID3D11RasterizerState* g_RasterStateCullOff = NULL;
	ID3D11RasterizerState* g_RasterStateCullCW = NULL;
	ID3D11RasterizerState* g_RasterStateCullCCW = NULL;
	ID3D11RasterizerState* g_RasterizerLayer0 = NULL;
	ID3D11RasterizerState* g_RasterizerLayer1 = NULL;
	ID3D11RasterizerState* g_RasterizerLayer2 = NULL;
	ID3D11RasterizerState* g_RasterizerLayer3 = NULL;

	ShaderManager& m_ShaderManager = ShaderManager::get_instance();
	ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();

	ShaderSet m_StaticModelShaderSet;
	ShaderSet m_SkinnedModelShaderSet;
	ShaderSet m_InstanceModelShaderSet;
	ShaderSet m_VFXShaderSet;
	ShaderSet m_UIShaderSet;

	MATERIAL_CBUFFER	g_Material;
	LIGHT_CBUFFER	g_Light;
	FOG_CBUFFER		g_Fog;

	FUCHI			g_Fuchi;
	RenderMode		g_RenderMode;

	float g_ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };	// �w�i�F
};