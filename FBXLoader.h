#pragma once

#include "main.h"
#include "renderer.h"
#include "HashMap.h"
#include "TextureMgr.h"
#include "SimpleArray.h"
#include "SingletonBase.h"
//*****************************************************************************
// ç\ë¢ëÃíËã`
//*****************************************************************************
struct PosNormalTexTanSkinned
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 TangentU;
	XMFLOAT3 Weights;
	BYTE BoneIndices[4];
};

struct ModelProperty
{
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

enum MODEL_PROPERTY
{
	QuaternionInterpolate,
	RotationOffset,
	RotationPivot,
	ScalingOffset,
	ScalingPivot,
	TranslationActive,
	TranslationMin,
	TranslationMax,
	TranslationMinX,
	TranslationMinY,
	TranslationMinZ,
	TranslationMaxX,
	TranslationMaxY,
	TranslationMaxZ,
	RotationOrder,
	RotationSpaceForLimitOnly,
	RotationStiffnessX,
	RotationStiffnessY,
	RotationStiffnessZ,
	AxisLen,
	PreRotation,
	PostRotation,
	RotationActive,
	RotationMin,
	RotationMax,
	RotationMinX,
	RotationMinY,
	RotationMinZ,
	RotationMaxX,
	RotationMaxY,
	RotationMaxZ,
	InheritType,
	ScalingActive,
	ScalingMin,
	ScalingMax,
	ScalingMinX,
	ScalingMinY,
	ScalingMinZ,
	ScalingMaxX,
	ScalingMaxY,
	ScalingMaxZ,
	GeometricTranslation,
	GeometricRotation,
	GeometricScaling,
	MinDampRangeX,
	MinDampRangeY,
	MinDampRangeZ,
	MaxDampRangeX,
	MaxDampRangeY,
	MaxDampRangeZ,
	MinDampStrengthX,
	MinDampStrengthY,
	MinDampStrengthZ,
	MaxDampStrengthX,
	MaxDampStrengthY,
	MaxDampStrengthZ,
	PreferedAngleX,
	PreferedAngleY,
	PreferedAngleZ,
	LookAtProperty,
	UpVectorProperty,
	Show,
	NegativePercentShapeSupport,
	DefaultAttributeIndex,
	Freeze,
	LODBox,
	Lcl_Translation,
	Lcl_Rotation,
	Lcl_Scaling,
	Visibility,
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

class FBXLoader : public SingletonBase<FBXLoader>
{
public:
	FBXLoader() {};
	void LoadModel(ID3D11Device* device, TextureMgr& texMgr, SkinnedMeshModel& model, const char* modelFilename, const char* texturePath);

private:
	bool ParseObjectDefinitions(FILE* file, SkinnedMeshModel& model);
	bool ParseObjectProperties(FILE* file, SkinnedMeshModel& model);
	bool ParseGeometry(FILE* file, SkinnedMeshModel& model);
	bool ParseModel(FILE* file, SkinnedMeshModel& model);
	bool ParseMaterial(FILE* file, SkinnedMeshModel& model);
	bool ParseMaterialProperty(FILE* file, Material& globalMaterial);
	bool ParseModelProperty(FILE* file, ModelProperty& modelProperty);
	bool ParseVertexData(FILE* file, SkinnedMeshModel& model);
	bool ParseIndexData(FILE* file, SkinnedMeshModel& model);
	bool ParseNormal(FILE* file, SkinnedMeshModel& model);
	bool ParseUV(FILE* file, SkinnedMeshModel& model);
	bool ParseTexture(FILE* file, SkinnedMeshModel& model);
	bool ParseObjectConnections(FILE* file, SkinnedMeshModel& model);
	void SkipNode(FILE* file);
};