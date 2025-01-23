//=============================================================================
//
// モデルの処理 [model.h]
// Author :
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "HashMap.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MODEL_MAX_MATERIAL		(16)		// １モデルのMaxマテリアル数

//*****************************************************************************
// 構造体定義
//*****************************************************************************

class Model;

// マテリアル構造体
struct MODEL_MATERIAL
{
	char						Name[256];
	MATERIAL					MaterialData;
	char						TextureName[256];
};

// 描画サブセット構造体
struct SUBSET
{
	unsigned int	StartIndex;
	unsigned int	IndexNum;
	MODEL_MATERIAL	Material;
	ID3D11ShaderResourceView* Texture;

	SUBSET()
	{
		Texture = nullptr;
		Material = MODEL_MATERIAL();
		StartIndex = 0;
		IndexNum = 0;
	}

	~SUBSET()
	{
		if (Texture)
			Texture->Release();
	}
};

// モデル構造体
struct MODEL_DATA
{
	VERTEX_3D* VertexArray;
	unsigned int	VertexNum;
	unsigned int* IndexArray;
	unsigned int	IndexNum;

	MODEL_DATA()
	{
		VertexArray = nullptr;
		IndexArray = nullptr;
		VertexNum = 0;
		IndexNum = 0;
	}

	~MODEL_DATA()
	{
		delete[] VertexArray;
		delete[] IndexArray;
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


struct BOUNDING_BOX
{
	XMFLOAT3 minPoint;
	XMFLOAT3 maxPoint;
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

	BOUNDING_BOX GetBondingBox() { return boundingBox; }

	static Model* StoreModel(char* modelPath);
	static MODEL_POOL* GetModel(char* modelPath);
	static void RemoveModel(char* modelPath);

private:
	void LoadObj(char* FileName, MODEL_DATA* Model);
	void LoadMaterial(char* FileName, MODEL_MATERIAL** MaterialArray, unsigned int* MaterialNum);
	void CreateBoundingBoxVertex();

	static HashMap<char*, MODEL_POOL, CharPtrHash, CharPtrEquals> modelHashMap;

	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;

	SUBSET*			SubsetArray;
	unsigned int	SubsetNum;

	MODEL_DATA*		modelData;

	BOUNDING_BOX	boundingBox;
	ID3D11Buffer* BBVertexBuffer;

	Renderer& renderer = Renderer::get_instance();
};