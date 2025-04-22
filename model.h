//=============================================================================
//
// モデルの処理 [Model.h]
// Author :
//
//=============================================================================
#pragma once

#include "main.h"
#include "Renderer.h"
#include "HashMap.h"
#include "OctreeNode.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MODEL_MAX_MATERIAL		(16)		// １モデルのMaxマテリアル数

enum TextureType
{
	Diffuse,
	Normal,
	Bump,
	Opacity,
	Reflect,
	Translucency
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

class Model;

// マテリアル構造体
struct MODEL_MATERIAL
{
	char						Name[256] = {};
	MATERIAL					MaterialData;
	char						DiffuseTextureName[256] = {};
	char						NormalTextureName[256] = {};
	char						BumpTextureName[256] = {};
	char						OpacityTextureName[256] = {};
	char						ReflectTextureName[256] = {};
	char						TranslucencyTextureName[256] = {};
};

// 描画サブセット構造体
struct SUBSET
{
	unsigned int	StartIndex;
	unsigned int	IndexNum;
	MODEL_MATERIAL	Material;
	ID3D11ShaderResourceView* diffuseTexture;
	ID3D11ShaderResourceView* normalTexture;
	ID3D11ShaderResourceView* bumpTexture;
	ID3D11ShaderResourceView* opacityTexture;
	ID3D11ShaderResourceView* reflectTexture;
	ID3D11ShaderResourceView* translucencyTexture;

	SUBSET()
	{
		diffuseTexture = nullptr;
		normalTexture = nullptr;
		bumpTexture = nullptr;
		opacityTexture = nullptr;
		reflectTexture = nullptr;
		translucencyTexture = nullptr;
		Material = MODEL_MATERIAL();
		StartIndex = 0;
		IndexNum = 0;
	}

	~SUBSET()
	{
		SafeRelease(&diffuseTexture);
		SafeRelease(&normalTexture);
		SafeRelease(&bumpTexture);
		SafeRelease(&opacityTexture);
		SafeRelease(&reflectTexture);
		SafeRelease(&translucencyTexture);
	}
};

// モデル構造体
struct MODEL_DATA
{
	VERTEX_3D*		VertexArray;
	unsigned int	VertexNum;
	unsigned int*	IndexArray;
	unsigned int	IndexNum;

	SUBSET*			SubsetArray;
	unsigned int	SubsetNum;
	BOUNDING_BOX	boundingBox;

	SimpleArray<Triangle*> triangles;

	MODEL_DATA()
	{
		VertexArray = nullptr;
		IndexArray = nullptr;
		SubsetArray = nullptr;
		VertexNum = 0;
		IndexNum = 0;
		SubsetNum = 0;
	}

	~MODEL_DATA()
	{
		SAFE_DELETE_ARRAY(VertexArray);
		SAFE_DELETE_ARRAY(IndexArray);
		SAFE_DELETE_ARRAY(SubsetArray);
	}
};

struct MODEL_POOL
{
	Model* pModel;
	unsigned int count;

	MODEL_POOL()
	{
		pModel = nullptr;
		count = 0;
	}

	void AddRef() { count++; }
};

struct StaticMeshPart
{
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	UINT IndexNum = 0;
	UINT StartIndex = 0;
};


class Model
{
public:
	Model(char* FileName);
	Model() = delete;

	~Model();
	void DrawModel();
	void DrawBoundingBox();

	// モデルのマテリアルのディフューズを取得する。Max16個分にしてある
	void GetModelDiffuse(XMFLOAT4* diffuse);

	// モデルの指定マテリアルのディフューズをセットする。
	void SetModelDiffuse(int mno, XMFLOAT4 diffuse);

	void BuildTrianglesByBoundingBox(BOUNDING_BOX box);
	bool BuildOctree(void);

	const MODEL_DATA* GetModelData(void) { return modelData; }
	const SUBSET* GetSubset(void) { return modelData->SubsetArray; }
	unsigned int GetSubNum(void) { return modelData->SubsetNum; }
	BOUNDING_BOX GetBoundingBox(void) { return modelData->boundingBox; }
	void SetDrawBoundingBox(bool draw) { drawBoundingBox = draw; }

	const SimpleArray<StaticMeshPart>& GetMeshParts(void) const;

	static Model* StoreModel(char* modelPath);
	static MODEL_POOL* GetModel(char* modelPath);
	static void RemoveModel(char* modelPath);

private:
	void LoadObj(char* FileName, MODEL_DATA* Model);
	void LoadMaterial(char* FileName, MODEL_MATERIAL** MaterialArray, unsigned int* MaterialNum);
	void LoadTextureName(char*FileName, FILE* fp, MODEL_MATERIAL* Material, int mc, TextureType type);
	void CreateBoundingBoxVertex(void);
	void BuildMeshParts(void);

	mutable SimpleArray<StaticMeshPart> m_meshParts;
	static HashMap<char*, MODEL_POOL, CharPtrHash, CharPtrEquals> modelHashMap;

	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;

	//SUBSET*			SubsetArray;
	//unsigned int	SubsetNum;

	MODEL_DATA*		modelData;
	bool			loadTangent = false;
	bool			drawBoundingBox = false;

	//BOUNDING_BOX	boundingBox;
	ID3D11Buffer* BBVertexBuffer;

	Renderer& renderer = Renderer::get_instance();
};