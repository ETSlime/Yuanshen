#pragma once
//=============================================================================
//
// SkinnedMeshModel [SkinnedMeshModel.h]
// Author : 
//
//=============================================================================
#include "FBXLoader.h"
#include "AnimStateMachine.h"
#include "OctreeNode.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_MESH_NUM			64
#define MAX_ANIM_NUM			32
#define MODEL_NAME_LENGTH		64
#define MODEL_PATH_LENGTH		128
#define ANIM_SPD				(1539538600 * 0.55f)
enum VertexDataLocation
{
	Index,
	Vertex,
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct PosNormalTexTanSkinned
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 TangentU;
};

struct SkinnedMeshModelPool
{
	SkinnedMeshModel* pModel;
	unsigned int count;

	SkinnedMeshModelPool()
	{
		pModel = nullptr;
		count = 0;
	}

	void AddRef() { count++; }
};

struct BoneTransformData
{
	SimpleArray<XMFLOAT4X4> mModelGlobalRot;
	SimpleArray<XMFLOAT4X4> mModelGlobalScl;
	SimpleArray<XMFLOAT4X4> mModelTranslate;
	SimpleArray<XMFLOAT4X4> mModelGlobalTrans;
	SimpleArray<XMFLOAT4X4> mModelLocalTrans;
	SimpleArray<XMFLOAT4X4> mBoneFinalTransforms;

	HashMap<uint64_t, int, HashUInt64, EqualUInt64> limbHashMap = HashMap<uint64_t, int, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	BoneTransformData(int numBones)
	{
		mModelGlobalRot.resize(numBones);
		mModelGlobalScl.resize(numBones);
		mModelTranslate.resize(numBones);
		mModelGlobalTrans.resize(numBones);
		mModelLocalTrans.resize(numBones);
		mBoneFinalTransforms.resize(numBones);
	}
};

struct ModelData
{
	ModelData()
	{
		mesh = new MeshData();
		boneTransformData = nullptr;
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

	~ModelData()
	{
		SAFE_DELETE(mesh);
		SAFE_DELETE(VertexArray);
		SAFE_DELETE(IndexArray);
		SAFE_DELETE(boneTransformData);

		SafeRelease(&VertexBuffer);
		SafeRelease(&IndexBuffer);

		for (int i = 0; i < boundingBoxes.getSize(); i++)
		{
			SAFE_DELETE(boundingBoxes[i]);
		}

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
	BoneTransformData* boneTransformData;

	SimpleArray<int> mBoneHierarchy;
	SimpleArray<XMFLOAT4X4> mBoneOffsets;
	SimpleArray<XMFLOAT4X4> mBoneToParentTransforms;
	SimpleArray<SKINNED_MESH_BOUNDING_BOX*> boundingBoxes;
	SimpleArray<Triangle*> triangles;
	VertexDataLocation normalLoc;
	VertexDataLocation texLoc;

	FbxMaterial* material;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	ID3D11ShaderResourceView* diffuseTexture;
	ID3D11ShaderResourceView* emissiveTexture;
};

class SkinnedMeshModel
{
public:
	friend class FBXLoader;

	void DrawModel();
	void UpdateBoneTransform(SimpleArray<XMFLOAT4X4>* boneTransforms);

	SkinnedMeshModel();
	~SkinnedMeshModel();

	void SetBodyDiffuseTexture(char* texturePath);
	void SetBodyLightMapTexture(char* texturePath);
	void SetBodyNormalMapTexture(char* texturePath);
	void SetHairDiffuseTexture(char* texturePath);
	void SetHairLightMapTexture(char* texturePath);
	void SetFaceDiffuseTexture(char* texturePath);
	void SetFaceLightMapTexture(char* texturePath);

	void LoadTownTexture(void);

	void GetBoneTransformByAnim(FbxNode* currentClipArmatureNode, uint64_t currentClipTime, SimpleArray<XMFLOAT4X4>* boneFinalTransform);

	void SetBoundingBoxLocationOffset(XMFLOAT3 offset, int boneIdx = 0);
	void SetBoundingBoxSize(XMFLOAT3 size, int boneIdx = 0);
	void SetDrawBoundingBox(bool draw) { drawBoundingBox = draw; }
	void BuildTrianglesByWorldMatrix(XMMATRIX worldMatrix);
	bool BuildOctree(void);
	UINT GetNumBones(void) { return numBones; }
	BOUNDING_BOX GetBoundingBox(void) { return boundingBox; }
	const SimpleArray<Triangle*>& GetTriangles(void) const;

	void SetCurrentAnim(AnimationClipName clipName, float startTime = 0);
	AnimationClipName GetCurrentAnim(void) { return currentAnimClip->name; }
	AnimationClip* GetAnimationClip(AnimationClipName clipName);
	void PlayCurrentAnim(float playSpeed = 1.0f);

	XMMATRIX GetWeaponTransformMtx(void);
	XMMATRIX GetBoneFinalTransform(int boneIdx = 0);

	UINT weaponTransformIdx;

	static SkinnedMeshModel* StoreModel(char* modelPath, char* modelName, char* modelFullPath, 
		ModelType modelType, AnimationClipName clipName);
	static SkinnedMeshModelPool* GetModel(char* modelFullPath);
	static void RemoveModel(char* modelPath);

private:

	static HashMap<char*, SkinnedMeshModelPool, CharPtrHash, CharPtrEquals> modelHashMap;

	void UpdateLimbGlobalTransform(FbxNode* node, FbxNode* deformNode, int& curIdx, int prevIdx, uint64_t time, BoneTransformData* boneTransformData, BOOL isLoop = TRUE);
	void GetBoneTransform(SimpleArray<XMFLOAT4X4>& boneFinalTransform, ModelData* modelData);
	XMFLOAT3 GetAnimationValue(FbxNode** ppAnimationCurve, XMFLOAT3 defaultValue, uint64_t time, BOOL isLoop);
	float GetAnimationCurveValue(FbxNode** ppAnimationCurveNode, uint64_t time, float defaultValue, BOOL isLoop);
	void CalculateDrawParameters(ModelData* modelData, float startPercentage, float endPercentage, int& IndexNum, int& StartIndexLocation);

	void DrawSigewinne(ModelData* modelData);
	void DrawKlee(ModelData* modelData);
	void DrawLumine(ModelData* modelData);
	void DrawTree(ModelData* modelData);
	void DrawField(ModelData* modelData);
	void DrawTown(ModelData* modelData);

	void DrawBoundingBox(ModelData* modelData);
	void CreateBoundingBoxVertex(SKINNED_MESH_BOUNDING_BOX* boundingBox) const;

	char modelPath[MODEL_PATH_LENGTH];
	char modelName[MODEL_NAME_LENGTH];

	UINT ModelCount;
	UINT currentRootNodeID;
	UINT numBones;
	ModelType modelType;
	bool drawBoundingBox;

	AnimationClip* currentAnimClip;
	FbxNode* armatureNode;

	ModelProperty modelProperty;
	ModelProperty globalModelProperty;
	FbxMaterial globalMaterial;
	CULL_MODE cullMode;

	BOUNDING_BOX boundingBox;
	BindPose bindPose;

	ID3D11ShaderResourceView* bodyDiffuseTexture;
	ID3D11ShaderResourceView* bodyLightMapTexture;
	ID3D11ShaderResourceView* bodyNormalMapTexture;
	ID3D11ShaderResourceView* hairDiffuseTexture;
	ID3D11ShaderResourceView* hairLightMapTexture;
	ID3D11ShaderResourceView* faceDiffuseTexture;
	ID3D11ShaderResourceView* faceLightMapTexture;

	ID3D11ShaderResourceView* Area_Mdcity_Lvy01_Diffuse;
	ID3D11ShaderResourceView* Area_MdCity_Plot02_Diffuse;
	ID3D11ShaderResourceView* Area_Mdbuild_Wall06_Diffuse;
	ID3D11ShaderResourceView* Indoor_OutDoor_MDSkyBox;

	HashMap<uint64_t, FbxNode*, HashUInt64, EqualUInt64> fbxNodes = 
		HashMap<uint64_t, FbxNode*, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, ModelData*, HashUInt64, EqualUInt64> meshDataMap = 
		HashMap<uint64_t, ModelData*, HashUInt64, EqualUInt64>(
		MAX_MESH_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<int, AnimationClip*, HashUInt64, EqualUInt64> animationClips = 
		HashMap<int, AnimationClip*, HashUInt64, EqualUInt64>(
		MAX_ANIM_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<int, uint64_t, HashUInt64, EqualUInt64> deformerHashMap = 
		HashMap<int, uint64_t, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, int, HashUInt64, EqualUInt64> deformerIdxHashMap = 
		HashMap<uint64_t, int, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);

	HashMap<uint64_t, uint64_t, HashUInt64, EqualUInt64> deformerToLimb = 
		HashMap<uint64_t, uint64_t, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);
};