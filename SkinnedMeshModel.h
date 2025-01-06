#pragma once

#include "FBXLoader.h"

#define TEXTURE_NAME_LENGTH		256

enum VertexDataLocation
{
	Index,
	Vertex,
};

struct IndexData
{
	int Index;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};


struct ModelData
{
	SimpleArray<VERTEX_3D> Vertices;
	SimpleArray<VERTEX_3D> VerticesTemp;
	SimpleArray<IndexData> IndicesData;
	SimpleArray<Subset> Subsets;

	VERTEX_3D* VertexArray;
	unsigned short	VertexNum;
	unsigned short* IndexArray;
	unsigned short	IndexNum;
};

class SkinnedMeshModel
{
public:
	friend class FBXLoader;

	void Draw();

private:
	UINT SubsetCount;

	SimpleArray<MATERIAL> Mat;
	SimpleArray<ID3D11ShaderResourceView*> DiffuseMapSRV;
	SimpleArray<ID3D11ShaderResourceView*> NormalMapSRV;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

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
};