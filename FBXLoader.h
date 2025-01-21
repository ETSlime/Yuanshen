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

enum class FbxNodeType
{
	LimbNode,
	Mesh,
	Skin,
	Cluster,
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

enum GeometryType
{
	Mesh,
	Shape,
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
	FbxNode* parentNode;
	SimpleArray<FbxNode*>	childNodes;
	void*	nodeData;
	void*	limbNodeAnimation;

	FbxNode() : nodeID(MAXUINT64), nodeName(""), nodeType(FbxNodeType::None), parentNode(nullptr), nodeData(nullptr), limbNodeAnimation(nullptr) {};

	FbxNode(uint64_t nodeID) : nodeName("")
	{
		this->nodeID = nodeID;
		parentNode = nullptr;
		nodeData = nullptr;
		limbNodeAnimation = nullptr;
		nodeType = FbxNodeType::None;
		childNodes = SimpleArray<FbxNode*>();
	}

	FbxNode(uint64_t nodeID, char* nodeName, FbxNodeType nodeType)
	{
		this->nodeID = nodeID;
		memset(nodeName, 0, sizeof(nodeName));
		strcpy(this->nodeName, nodeName);
		this->nodeType = nodeType;
		nodeData = nullptr;
		parentNode = nullptr;
		limbNodeAnimation = nullptr;
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
		if (childNodes.getSize() > 0)
			childNodes.clear();
	}
};

struct Material
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
};

class SkinnedMeshModel;
struct ModelData;

class FBXLoader : public SingletonBase<FBXLoader>
{
public:
	FBXLoader() {};
	void LoadModel(ID3D11Device* device, TextureMgr& texMgr, SkinnedMeshModel& model, const char* modelFilename, const char* texturePath);

private:
	bool ParseObjectDefinitions(FILE* file, SkinnedMeshModel& model);
	bool ParseObjectProperties(FILE* file, SkinnedMeshModel& model);
	bool ParseGeometry(FILE* file, SkinnedMeshModel& model);
	bool ParseModel(FILE* file, SkinnedMeshModel& model, FbxNode* node);
	bool ParseMaterial(FILE* file, SkinnedMeshModel& model);
	bool ParseDeformer(FILE* file, FbxNode* node);
	bool ParseMaterialProperty(FILE* file, Material& globalMaterial);
	bool ParseModelProperty(FILE* file, ModelProperty& modelProperty, FbxNode* node);
	bool ParseVertexData(FILE* file, SkinnedMeshModel& model);
	bool ParseIndexData(FILE* file, SkinnedMeshModel& model);
	bool ParseNormal(FILE* file, SkinnedMeshModel& model);
	bool ParseUV(FILE* file, SkinnedMeshModel& model);
	bool ParseTexture(FILE* file, SkinnedMeshModel& model);
	bool ParseObjectConnections(FILE* file, SkinnedMeshModel& model);
	bool ParseBindPose(FILE* file, SkinnedMeshModel& model, FbxNode* node);
	bool ParseAnimationCurve(FILE* file, FbxNode* node);
	bool CreateFbxNode(char* buffer, FbxNode* fbxNode);
	void SkipNode(FILE* file);

	void HandleDeformer(FbxNode* node, ModelData* data, SkinnedMeshModel& model, int boneCnt, int prevIdx);

	Renderer& renderer = Renderer::get_instance();
};
