#pragma once
//=============================================================================
//
// SkinnedMeshModel [SkinnedMeshModel.h]
// Author : 
//
//=============================================================================
#include "FBXLoader.h"
#include "HashMap.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_NAME_LENGTH		256

enum VertexDataLocation
{
	Index,
	Vertex,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct IndexData
{
	int Index;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

struct PosNormalTexTanSkinned
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 TangentU;
};


struct SKINNED_VERTEX_3D_TEMP
{
	XMFLOAT3	Position;
	XMFLOAT3	Normal;
	XMFLOAT4	Diffuse;
	XMFLOAT2	TexCoord;
	SimpleArray<float>	Weights;
	SimpleArray<int>	BoneIndices;
};

struct ModelData
{
	SimpleArray<SKINNED_VERTEX_3D> Vertices;
	SimpleArray<SKINNED_VERTEX_3D_TEMP> VerticesTemp;
	SimpleArray<IndexData> IndicesData;
	SimpleArray<Subset> Subsets;

	SKINNED_VERTEX_3D* VertexArray;
	unsigned short	VertexNum;
	unsigned short* IndexArray;
	unsigned short	IndexNum;
};

class SkinnedMeshModel
{
public:
	friend class FBXLoader;

	void Draw();
	void Update();

	SkinnedMeshModel();
	~SkinnedMeshModel();

private:

	void UpdateLimbGlobalTransform(FbxNode* node, int& curIdx, int prevIdx, uint64_t time);
	void UpdateBoneTransform();
	XMFLOAT3 GetAnimationValue(FbxNode** ppAnimationCurve, XMFLOAT3 defaultValue, uint64_t time);
	float GetAnimationCurveValue(FbxNode** ppAnimationCurveNode, uint64_t time, float defaultValue);

	UINT SubsetCount;
	UINT ModelCount;
	SimpleArray<MATERIAL> Mat;
	SimpleArray<ID3D11ShaderResourceView*> DiffuseMapSRV;
	SimpleArray<ID3D11ShaderResourceView*> NormalMapSRV;
	SimpleArray<int> mBoneHierarchy;
	SimpleArray<int> mModelHierarchy;
	SimpleArray<XMFLOAT4X4> mModelGlobalRot;
	SimpleArray<XMFLOAT4X4> mModelLocalRot;
	SimpleArray<XMFLOAT4X4> mModelGlobalScl;
	SimpleArray<XMFLOAT4X4> mModelLocalScl;
	SimpleArray<XMFLOAT4X4> mModelTranslate;
	SimpleArray<XMFLOAT4X4> mModelLocalTranslate;
	SimpleArray<XMFLOAT4X4> mModelGlobalTrans;
	SimpleArray<XMFLOAT4X4> mModelLocalTrans;
	SimpleArray<XMFLOAT4X4> mBoneOffsets;
	SimpleArray<XMFLOAT4X4> mBoneToParentTransforms;
	SimpleArray<XMFLOAT4X4> mBoneFinalTransforms;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	uint64_t animationTime;
	FbxNode* armatureNode;

	// Keep CPU copies of the mesh data to read from.  
	ModelData* modelData = new ModelData();
	ModelProperty modelProperty;
	ModelProperty globalModelProperty;
	Material material;
	Material globalMaterial;
	CULL_MODE cullMode;
	ID3D11ShaderResourceView* Texture;
	VertexDataLocation normalLoc;
	VertexDataLocation texLoc;
	char textureName[TEXTURE_NAME_LENGTH];
	BindPose bindPose;
	HashMap<uint64_t, FbxNode*, HashUInt64, EqualUInt64> fbxNodes = HashMap<uint64_t, FbxNode*, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<int, uint64_t, HashUInt64, EqualUInt64> deformerHashMap = HashMap<int, uint64_t, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, int, HashUInt64, EqualUInt64> limbHashMap = HashMap<uint64_t, int, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, uint64_t, HashUInt64, EqualUInt64> deformerToLimb = HashMap<uint64_t, uint64_t, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	Renderer& renderer = Renderer::get_instance();
};