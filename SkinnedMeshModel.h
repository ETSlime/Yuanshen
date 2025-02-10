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
#define MAX_MESH_NUM			64
#define MODEL_NAME_LENGTH		64
#define MODEL_PATH_LENGTH		128
enum VertexDataLocation
{
	Index,
	Vertex,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PosNormalTexTanSkinned
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 TangentU;
};

struct ModelData
{
	ModelData()
	{
		mesh = new MeshData();
		shapeCnt = 0;
		limbCnt = 0;
		VertexArray = nullptr;
		VertexNum = 0;
		IndexArray = nullptr;
		IndexNum = 0;

		armatureNode = nullptr;

		VertexBuffer = nullptr;
		IndexBuffer = nullptr;

		material = nullptr;

		diffuseTexture = nullptr;
		emissiveTexture = nullptr;

		modelID = modelCnt;
		modelCnt++;

	}

	MeshData*				mesh;
	SimpleArray<MeshData>	shapes;
	SimpleArray<Subset> Subsets;

	int shapeCnt;
	int limbCnt;
	int modelID;

	static int modelCnt;

	SKINNED_VERTEX_3D* VertexArray;
	unsigned int	VertexNum;
	unsigned int* IndexArray;
	unsigned int	IndexNum;

	FbxNode* armatureNode;
	SimpleArray<int> mBoneHierarchy;
	SimpleArray<XMFLOAT4X4> mModelGlobalRot;
	SimpleArray<XMFLOAT4X4> mModelGlobalScl;
	SimpleArray<XMFLOAT4X4> mModelTranslate;
	SimpleArray<XMFLOAT4X4> mModelGlobalTrans;
	SimpleArray<XMFLOAT4X4> mModelLocalTrans;
	SimpleArray<XMFLOAT4X4> mBoneOffsets;
	SimpleArray<XMFLOAT4X4> mBoneToParentTransforms;
	SimpleArray<XMFLOAT4X4> mBoneFinalTransforms;
	VertexDataLocation normalLoc;
	VertexDataLocation texLoc;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	XMFLOAT4X4	boneMatrices[BONE_MAX];

	FbxMaterial* material;

	ID3D11ShaderResourceView* diffuseTexture;
	ID3D11ShaderResourceView* emissiveTexture;

	HashMap<int, uint64_t, HashUInt64, EqualUInt64> deformerHashMap = HashMap<int, uint64_t, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, int, HashUInt64, EqualUInt64> deformerIdxHashMap = HashMap<uint64_t, int, HashUInt64, EqualUInt64>(
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
};

class SkinnedMeshModel
{
public:
	friend class FBXLoader;

	void Draw();
	void Update();

	SkinnedMeshModel();
	~SkinnedMeshModel();

	void SetBodyDiffuseTexture(TextureMgr& texMgr, char* texturePath);
	void SetHairDiffuseTexture(TextureMgr& texMgr, char* texturePath);
	void SetFaceDiffuseTexture(TextureMgr& texMgr, char* texturePath);

	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)
	
	int cnt = 0;

private:

	void UpdateLimbGlobalTransform(FbxNode* node, int& curIdx, int prevIdx, uint64_t time, ModelData* modelData);
	void UpdateBoneTransform(ModelData* modelData);
	XMFLOAT3 GetAnimationValue(FbxNode** ppAnimationCurve, XMFLOAT3 defaultValue, uint64_t time);
	float GetAnimationCurveValue(FbxNode** ppAnimationCurveNode, uint64_t time, float defaultValue);
	void CalculateDrawParameters(ModelData* modelData, float startPercentage, float endPercentage, int& IndexNum, int& StartIndexLocation);

	char modelPath[MODEL_PATH_LENGTH];
	char modelName[MODEL_NAME_LENGTH];

	UINT SubsetCount;
	UINT ModelCount;

	uint64_t animationTime;
	FbxNode* armatureNode;

	ID3D11ShaderResourceView* bodyDiffuseTexture;
	ID3D11ShaderResourceView* hairDiffuseTexture;
	ID3D11ShaderResourceView* faceDiffuseTexture;


	ModelProperty modelProperty;
	ModelProperty globalModelProperty;
	FbxMaterial globalMaterial;
	CULL_MODE cullMode;

	BindPose bindPose;

	HashMap<uint64_t, FbxNode*, HashUInt64, EqualUInt64> fbxNodes = HashMap<uint64_t, FbxNode*, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, ModelData*, HashUInt64, EqualUInt64> meshDataMap = HashMap<uint64_t, ModelData*, HashUInt64, EqualUInt64>(
		MAX_MESH_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	Renderer& renderer = Renderer::get_instance();
};