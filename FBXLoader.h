#pragma once

#include "main.h"
#include "renderer.h"
#include "HashMap.h"
#include "TextureMgr.h"
#include "SimpleArray.h"
#include "SingletonBase.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define MAX_NODE_NUM			8192
#define TEXTURE_NAME_LENGTH		256
#define SMALL_NUM_THRESHOLD		1e-6
enum class FbxNodeType
{
	LimbNode,
	Mesh,
	Material,
	Skin,
	Cluster,
	BlendShape,
	BlendShapeChannel,
	BindPose,
	AnimationCurve,
	AnimationCurveNode,
	None,
};

enum class ObjectType
{
	Model,
	Material,
	Texture,
	AnimationStack,
	AnimationLayer,
	Geometry,
	Implementation,
	BindingTable,
	NONE
};

enum MappingInformationType
{
	ByPolygon,
	ByPolygonVertex,
	ByVertice,
	ByEdge,
	AllSame,
	NoMappingInformation
};

enum ShadingModelEnum
{
	Phong,
};

enum ReferenceInformationType
{
	Direct,
	IndexToDirect
};

enum class GeometryType
{
	Mesh,
	Shape,
};

enum AnimationClipName
{
	ANIM_NONE,
	ANIM_IDLE,
	ANIM_BREAKDANCE_UPROCK,
	ANIM_STANDING_DRAW_ARROW,
	ANIM_STANDING_AIM_WALK_FORWARD,
	ANIM_STANDING_AIM_WALK_LEFT,
	ANIM_STANDING_AIM_OVERDRAW,
	ANIM_WALK,
	ANIM_RUN,
	ANIM_DASH,
};

enum class ModelType
{
	Default,
	Sigewinne,
	Klee,
	Lumine,
	Weapon,
	Hilichurl,
	Mitachurl,
	AbyssMage,
	Tree,
	Field,
	Town,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct ModelProperty
{
	int			modelIdx;
	bool		QuaternionInterpolate;
	bool		Visibility;
	XMFLOAT3	Translation;
	XMFLOAT3	Rotation;
	XMFLOAT3	Scaling;
	XMFLOAT3	RotationOffset;
	XMFLOAT3	RotationPivot;
	XMFLOAT3	ScalingOffset;
	XMFLOAT3	ScalingPivot;
	bool		TranslationActive;
	XMFLOAT3	TranslationMin;
	XMFLOAT3	TranslationMax;
	bool		TranslationMinX;
	bool		TranslationMinY;
	bool		TranslationMinZ;
	bool		TranslationMaxX;
	bool		TranslationMaxY;
	bool		TranslationMaxZ;
	UINT		RotationOrder;
	bool		RotationSpaceForLimitOnly;
	double		AxisLen;
	XMFLOAT3	PreRotation;
	XMFLOAT3	PostRotation;
	bool		RotationActive;
	XMFLOAT3	RotationMin;
	XMFLOAT3	RotationMax;
	bool		RotationMinX;
	bool		RotationMinY;
	bool		RotationMinZ;
	bool		RotationMaxX;
	bool		RotationMaxY;
	bool		RotationMaxZ;
	double		RotationStiffnessX;
	double		RotationStiffnessY;
	double		RotationStiffnessZ;
	double		MinDampRangeX;
	double		MinDampRangeY;
	double		MinDampRangeZ;
	double		MaxDampRangeX;
	double		MaxDampRangeY;
	double		MaxDampRangeZ;
	double		MinDampStrengthX;
	double		MinDampStrengthY;
	double		MinDampStrengthZ;
	double		MaxDampStrengthX;
	double		MaxDampStrengthY;
	double		MaxDampStrengthZ;
	double		PreferedAngleX;
	double		PreferedAngleY;
	double		PreferedAngleZ;
	UINT		InheritType;
	bool		ScalingActive;
	XMFLOAT3	ScalingMin;
	XMFLOAT3	ScalingMax;
	bool		ScalingMinX;
	bool		ScalingMinY;
	bool		ScalingMinZ;
	bool		ScalingMaxX;
	bool		ScalingMaxY;
	bool		ScalingMaxZ;
	XMFLOAT3	GeometricTranslation;
	XMFLOAT3	GeometricRotation;
	XMFLOAT3	GeometricScaling;
	void*		LookAtProperty;
	void*		UpVectorProperty;
	bool		Show;
	bool		NegativePercentShapeSupport;
	int			DefaultAttributeIndex;
	XMFLOAT3	Color;
	int			Camera_Index;
	double		Size;
	double		LimbLength;
	int			Look;
	bool		Freeze;
	bool		LODBox;
	bool		VisibilityInheritance;

	void Clear()
	{
		modelIdx = -1;
		Translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}

	ModelProperty()
	{
		Clear();
	}
};

struct Deformer
{
	SimpleArray<int> Index;
	SimpleArray<float> Weights;
	XMFLOAT4X4 Transform;
	XMFLOAT4X4 TransformLink;
	int	boneIdx;
};

struct AnimationCurve
{
	SimpleArray<uint64_t> KeyTime;
	SimpleArray<float> KeyValue;
	int KeyAttrRefCount;
	float DefaultValue;
};

struct AnimationCurveNode
{
	uint64_t dX;
	uint64_t dY;
	uint64_t dZ;
};

struct LimbNodeAnimation
{
	uint64_t LclTranslation;
	uint64_t LclRot;
	uint64_t LclScl;
};

struct BindPoseData
{
	uint64_t	ModelNodeID;
	XMFLOAT4X4	mtxBindPose;
};

struct BindPose
{
	int	numePoseNodes;
	SimpleArray<BindPoseData> bindPoseData;
	HashMap<uint64_t, XMFLOAT4X4, HashUInt64, EqualUInt64> mtxBindPoses = 
		HashMap<uint64_t, XMFLOAT4X4, HashUInt64, EqualUInt64>(
		MAX_NODE_NUM,
		HashUInt64(),
		EqualUInt64()
	);
};

struct IndexData
{
	int Index;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Bitangent;
	XMFLOAT2 TexCoord;
};

struct SKINNED_VERTEX_3D_TEMP
{
	XMFLOAT3	Position;
	XMFLOAT3	Normal;
	XMFLOAT3	Tangent;
	XMFLOAT3	Bitangent;
	XMFLOAT4	Diffuse;
	XMFLOAT2	TexCoord;
	SimpleArray<float>	Weights;
	SimpleArray<int>	BoneIndices;

	//SimpleArray<SimpleArray<float>>	ModelWeights;
	//SimpleArray<int>	BoneIndices;
};

struct MeshData
{
	SimpleArray<SKINNED_VERTEX_3D> Vertices;
	SimpleArray<SKINNED_VERTEX_3D_TEMP> VerticesTemp;
	SimpleArray<IndexData> IndicesData;
};

struct Subset
{
	Subset() :
		Id(-1),
		VertexStart(0), VertexCount(0),
		FaceStart(0), FaceCount(0)
	{
	}

	UINT Id;
	UINT VertexStart;
	UINT VertexCount;
	UINT FaceStart;
	UINT FaceCount;
};

struct FbxNode
{
	uint64_t	nodeID;
	char nodeName[128];
	FbxNodeType nodeType;
	SimpleArray<FbxNode*>	parentNodes;
	SimpleArray<FbxNode*>	childNodes;
	void*	nodeData;
	void*	limbNodeAnimation;

	FbxNode() : nodeID(MAXUINT64), nodeName(""), nodeType(FbxNodeType::None), parentNodes(SimpleArray<FbxNode*>()), nodeData(nullptr), limbNodeAnimation(nullptr) {};

	FbxNode(uint64_t nodeID) : nodeName("")
	{
		this->nodeID = nodeID;
		nodeData = nullptr;
		limbNodeAnimation = nullptr;
		nodeType = FbxNodeType::None;
		parentNodes = SimpleArray<FbxNode*>();
		childNodes = SimpleArray<FbxNode*>();
	}

	FbxNode(uint64_t nodeID, char* nodeName, FbxNodeType nodeType)
	{
		this->nodeID = nodeID;
		memset(nodeName, 0, sizeof(nodeName));
		strcpy(this->nodeName, nodeName);
		this->nodeType = nodeType;
		nodeData = nullptr;
		limbNodeAnimation = nullptr;
		parentNodes = SimpleArray<FbxNode*>();
		childNodes = SimpleArray<FbxNode*>();
	}

	~FbxNode()
	{
		//if (parentNode)
		//{
		//	delete parentNode;
		//	parentNode = nullptr;
		//}
		//	
		if (nodeData)
		{
			delete nodeData;
			nodeData = nullptr;
		}
		if (parentNodes.getSize() > 0)
			parentNodes.clear();
		if (childNodes.getSize() > 0)
			childNodes.clear();
	}
};

struct FbxMaterial
{
	ShadingModelEnum	ShadingModel;
	bool				MultiLayer;
	XMFLOAT3			EmissiveColor;
	float				EmissiveFactor;
	XMFLOAT3			AmbientColor;
	float				AmbientFactor;
	XMFLOAT3			DiffuseColor;
	float				DiffuseFactor;
	XMFLOAT3			Bump;
	XMFLOAT3			NormalMap;
	double				BumpFactor;
	XMFLOAT3			TransparentColor;
	XMFLOAT3			TransparencyFactor;
	XMFLOAT3			DisplacementColor;
	double				DisplacementFactor;
	XMFLOAT3			VectorDisplacementColor;
	double				VectorDisplacementFactor;
	XMFLOAT3			SpecularColor;
	float				SpecularFactor;
	float				ShininessExponent;
	XMFLOAT3			ReflectionColor;
	float				ReflectionFactor;

	char				DiffuseColorTexName[TEXTURE_NAME_LENGTH];
	char				EmissiveColorTexName[TEXTURE_NAME_LENGTH];
};

class SkinnedMeshModel;
struct ModelData;

class FBXLoader : public SingletonBase<FBXLoader>
{
public:
	FBXLoader() {};
	bool LoadModel(ID3D11Device* device, TextureMgr& texMgr, SkinnedMeshModel& model, 
		const char* modelPath, const char* modelName, const char* texturePath, AnimationClipName name = AnimationClipName::ANIM_NONE, ModelType modelType = ModelType::Default);

	bool LoadAnimation(ID3D11Device* device, SkinnedMeshModel& model,
		const char* modelPath, const char* modelName, AnimationClipName name);

private:
	bool ParseObjectDefinitions(FILE* file, SkinnedMeshModel& model);
	bool ParseObjectProperties(FILE* file, SkinnedMeshModel& model, bool loadAnimation = false);
	bool ParseGeometry(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, FbxNode* node);
	bool ParseModel(FILE* file, SkinnedMeshModel& model, FbxNode* node);
	bool ParseMaterial(FILE* file, SkinnedMeshModel& model, FbxNode* node);
	bool ParseDeformer(FILE* file, FbxNode* node);
	bool ParseMaterialProperty(FILE* file, FbxMaterial& globalMaterial);
	bool ParseModelProperty(FILE* file, ModelProperty& modelProperty, FbxNode* node);
	bool ParseVertexData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, FbxNode* node, MeshData* &meshData);
	bool ParseIndexData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, FbxNode* node, MeshData* &meshData);
	bool ParseNormal(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, FbxNode* node, MeshData* &meshData);
	bool ParseNormalData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, 
		MappingInformationType mappingInformationType, ReferenceInformationType referenceInformationType, FbxNode* node, MeshData* &meshData, SimpleArray<float>& normals);
	bool ParseNormalIndexData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, FbxNode* node, MeshData*& meshData, SimpleArray<float>& normals);
	bool ParseUV(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, FbxNode* node, MeshData* &meshData);
	bool ParseTexture(FILE* file, SkinnedMeshModel& model, FbxNode* node);
	bool ParseObjectConnections(FILE* file, SkinnedMeshModel& model);
	bool ParseBindPose(FILE* file, SkinnedMeshModel& model, FbxNode* node);
	bool ParseAnimationStackCurve(FILE* file, SkinnedMeshModel& model);
	bool ParseAnimationCurve(FILE* file, FbxNode* node);
	bool CreateFbxNode(char* buffer, FbxNode* fbxNode);
	void SkipNode(FILE* file);

	void HandleDeformer(FbxNode* node, ModelData* data, SkinnedMeshModel& model, int boneCnt, int prevIdx);
	void HandleMeshNode(FbxNode* node, SkinnedMeshModel& model);

	void BuildLimbHierarchy(FbxNode* armatureNode, ModelData* modelData, int curIdx, int prevIdx);

	FbxNode* GetGeometryNodeByLimbNode(FbxNode* limbNode);
	FbxNode* GetModelArmatureNodeByDeformer(FbxNode* deformerNode);
	FbxNode* GetModelArmatureNodeByModel(FbxNode* modelNode);

	char* GetTextureName(char* modelPath, char* relativeTexturePath);

	void CalculateTangentAndBitangent(
		const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2,
		const XMFLOAT2& uv0, const XMFLOAT2& uv1, const XMFLOAT2& uv2,
		const XMFLOAT3& n0, const XMFLOAT3& n1, const XMFLOAT3& n2,
		XMFLOAT3& tangent0, XMFLOAT3& tangent1, XMFLOAT3& tangent2, 
		XMFLOAT3& bitangent0, XMFLOAT3& bitangent1, XMFLOAT3& bitangent2);

	Renderer& renderer = Renderer::get_instance();
};
