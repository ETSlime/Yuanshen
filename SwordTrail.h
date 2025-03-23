#pragma once
//=============================================================================
//
// SwordTrail処理 [SwordTrail.h]
// Author : 
//
//=============================================================================
#include "GameObject.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define SWORD_TRAIL_TEXTURE_PATH	"data/TEXTURE/SwordTrail/SwordTrail.png"

//*****************************************************************************
// 構造体定義
//*****************************************************************************

enum class SwordTrailType
{
	Default,
	Normal,
	Flame,
	Ice,
	Lightning,
};

class SwordTrail
{
public:
	SwordTrail(const GameObject<SkinnedMeshModelInstance>* weapon);
	~SwordTrail();
	void Update(void);
	void Draw(void);
	void GenerateSwordTrailMesh(void);
	bool SetTrailType(SwordTrailType type);
	void SetTrailFrames(int frames);
	void GetSwordTrailPoints(XMVECTOR& hiltWorld, XMVECTOR& tipWorld);
private:

	bool LoadTrailTexture(char* path);

	const GameObject<SkinnedMeshModelInstance>* weapon;

	SwordTrailType trailType;

	int maxTrailFrames;

	ID3D11Buffer* vertexBuffer;
	ID3D11ShaderResourceView* swordTrailTexture;
	SimpleArray<XMVECTOR> hiltTrail;
	SimpleArray<XMVECTOR> tipTrail;
	SimpleArray<VFXVertex> swordTrailVertices;

	Renderer& renderer = Renderer::get_instance();
};