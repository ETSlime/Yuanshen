//=============================================================================
//
// レンダリング処理 [renderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"
//*********************************************************
// マクロ定義
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
enum LIGHT_TYPE
{
	LIGHT_TYPE_NONE,		//ライト無し
	LIGHT_TYPE_DIRECTIONAL,	//ディレクショナルライト
	LIGHT_TYPE_POINT,		//ポイントライト

	LIGHT_TYPE_NUM
};

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//ブレンド無し
	BLEND_MODE_ALPHABLEND,	//αブレンド
	BLEND_MODE_ADD,			//加算ブレンド
	BLEND_MODE_SUBTRACT,	//減算ブレンド
	BLEND_MODE_SWORDTRAIL,

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//カリング無し
	CULL_MODE_FRONT,		//表のポリゴンを描画しない(CW)
	CULL_MODE_BACK,			//裏のポリゴンを描画しない(CCW)

	CULL_MODE_NUM
};

enum class RenderMode
{
	OBJ,
	SKINNED_MESH,
	OBJ_SHADOW,
	SKINNED_MESH_SHADOW,
	INSTANCE,
	INSTANCE_SHADOW,
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
// 構造体
//*********************************************************

// 頂点構造体
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

// マテリアル構造体
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

// ライト構造体
struct LIGHT 
{
	XMFLOAT3	Direction;	// ライトの方向
	XMFLOAT3	Position;	// ライトの位置
	XMFLOAT4	Diffuse;	// 拡散光の色
	XMFLOAT4	Ambient;	// 環境光の色
	float		Attenuation;// 減衰率
	int			Type;		// ライト種別・有効フラグ
	int			Enable;		// ライト種別・有効フラグ

	XMFLOAT4X4	LightViewProj;
};

// フォグ構造体
struct FOG 
{
	float		FogStart;	// フォグの開始距離
	float		FogEnd;		// フォグの最大距離
	XMFLOAT4	FogColor;	// フォグの色
};

struct LightViewProjBuffer
{
	XMMATRIX ProjView[LIGHT_MAX];
	int LightIndex;
	int padding[3];
};

// マテリアル用定数バッファ構造体
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
	//float		Dummy[1];				// 16byte境界用
};

// ライト用フラグ構造体
struct LIGHTFLAGS
{
	int			Type;		//ライトタイプ（enum LIGHT_TYPE）
	int         OnOff;		//ライトのオンorオフスイッチ
	int			Dummy[2];
};

// ライト用定数バッファ構造体
struct LIGHT_CBUFFER
{
	XMFLOAT4	Direction[LIGHT_MAX];	// ライトの方向
	XMFLOAT4	Position[LIGHT_MAX];	// ライトの位置
	XMFLOAT4	Diffuse[LIGHT_MAX];		// 拡散光の色
	XMFLOAT4	Ambient[LIGHT_MAX];		// 環境光の色
	XMFLOAT4	Attenuation[LIGHT_MAX];	// 減衰率
	LIGHTFLAGS	Flags[LIGHT_MAX];		// ライト種別
	int			Enable;					// ライティング有効・無効フラグ
	int			Dummy[3];				// 16byte境界用

	XMFLOAT4X4	LightViewProj;
};

// フォグ用定数バッファ構造体
struct FOG_CBUFFER
{
	XMFLOAT4	Fog;					// フォグ量
	XMFLOAT4	FogColor;				// フォグの色
	int			Enable;					// フォグ有効・無効フラグ
	float		Dummy[3];				// 16byte境界用
};

// 縁取り用バッファ
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
	float progress;
	int isRandomFade;
	XMFLOAT2 padding;
};

//*****************************************************************************
// プロトタイプ宣言
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
	void SetBlendState(BLEND_MODE bm);
	void SetCullingMode(CULL_MODE cm);
	void SetAlphaTestEnable(BOOL flag);

	void SetWorldViewProjection2D(void);
	void SetCurrentWorldMatrix(const XMMATRIX* WorldMatrix) const;
	void SetViewMatrix(const XMMATRIX* ViewMatrix) const;
	void SetProjectionMatrix(const XMMATRIX* ProjectionMatrix) const;

	void SetMaterial(MATERIAL material);

	void SetLightEnable(BOOL flag);
	void SetLight(int index, LIGHT* light);
	void SetLightProjView(LightViewProjBuffer* lightBuffer);

	void SetFogEnable(BOOL flag);
	void SetFog(FOG* fog);

	void SetRenderProgress(RenderProgressBuffer renderProgress);

	void DebugTextOut(char* text, int x, int y);

	void SetFuchi(int flag);
	void SetShaderCamera(XMFLOAT3 pos);
	void SetFillMode(D3D11_FILL_MODE mode);
	void SetBoneMatrix(XMMATRIX matrices[BONE_MAX]);
	void SetClearColor(float* color4);
	void SetRenderLayer(RenderLayer layer);
	void SetRenderShadowMap(int lightIdx);
	void SetRenderSkinnedMeshShadowMap(int lightIdx);
	void SetRenderInstanceShadowMap(int lightIdx);
	void SetRenderObject(void);
	void SetRenderSkinnedMeshModel(void);
	void SetRenderInstance(void);
	void SetRenderVFX(void);
	void SetRenderUI(void);
	void SetModelInputLayout(void);
	void SetSkinnedMeshInputLayout(void);
	void SetVFXInputLayout(void);
	void ResetRenderTarget(void);
	void SetLightModeBuffer(int mode);
	void ClearShadowDSV(int lightIdx);

	void SetMainPassViewport(void);
	void SetShadowPassViewport(void);

	RenderMode GetRenderMode(void);
	void SetRenderMode(RenderMode mode);


private:

	void SetLightBuffer(void);
	void SetFogBuffer(void);

	D3D_FEATURE_LEVEL       g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device* g_D3DDevice = NULL;
	ID3D11DeviceContext* g_ImmediateContext = NULL;
	IDXGISwapChain* g_SwapChain = NULL;

	ID3D11RenderTargetView* g_RenderTargetView = NULL;

	ID3D11DepthStencilView* g_ShadowDSV[LIGHT_MAX];
	ID3D11DepthStencilView* g_SceneDepthStencilView = NULL;

	ID3D11InputLayout* g_VertexLayout = NULL;
	ID3D11VertexShader* g_VertexShader = NULL;
	ID3D11VertexShader* g_DepthVertexShader = NULL;
	ID3D11PixelShader* g_PixelShader = NULL;

	ID3D11InputLayout* g_SkinnedMeshVertexLayout = NULL;
	ID3D11VertexShader* g_SkinnedMeshVertexShader = NULL;
	ID3D11PixelShader* g_SkinnedMeshPixelShader = NULL;
	ID3D11VertexShader* g_DepthSkinnedMeshVertexShader = NULL;

	ID3D11VertexShader* g_VFXVertexShader = NULL;
	ID3D11PixelShader* g_VFXPixelShader = NULL;
	ID3D11InputLayout* g_VFXVertexLayout = NULL;

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

	ID3D11ShaderResourceView* g_ShadowMapSRV[LIGHT_MAX];

	ID3D11DepthStencilState* g_DepthStateEnable = NULL;
	ID3D11DepthStencilState* g_DepthStateDisable = NULL;

	ID3D11BlendState* g_BlendStateNone = NULL;
	ID3D11BlendState* g_BlendStateAlphaBlend = NULL;
	ID3D11BlendState* g_BlendStateAdd = NULL;
	ID3D11BlendState* g_BlendStateSubtract = NULL;
	ID3D11BlendState* g_BlendStateSwordTrail = NULL;
	BLEND_MODE				g_BlendStateParam;


	ID3D11RasterizerState* g_RasterStateCullOff;
	ID3D11RasterizerState* g_RasterStateCullCW;
	ID3D11RasterizerState* g_RasterStateCullCCW;
	ID3D11RasterizerState* g_RasterizerLayer0;
	ID3D11RasterizerState* g_RasterizerLayer1;
	ID3D11RasterizerState* g_RasterizerLayer2;
	ID3D11RasterizerState* g_RasterizerLayer3;


	MATERIAL_CBUFFER	g_Material;
	LIGHT_CBUFFER	g_Light;
	FOG_CBUFFER		g_Fog;

	FUCHI			g_Fuchi;
	RenderMode		g_RenderMode;

	float g_ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };	// 背景色
};