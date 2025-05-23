#pragma once
//=============================================================================
//
// ソードトレイル [SwordTrail.h]
// Author : 
// - 武器の動きをエモく彩る残像エフェクトっ
// - 柄と刃先の履歴を保持して、空間に幻想を残す
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

	// hiltTrail, tipTrail の履歴から VFXVertex を生成して vertexBuffer を更新
	void GenerateSwordTrailMesh(void);
	// 軌跡の見た目タイプ（通常、炎、氷、雷…）を切り替える
	bool SetTrailType(SwordTrailType type);
	// 最大フレーム数（履歴数）を指定、つまり「残像の長さ」を制御できる
	void SetTrailFrames(int frames);
	// 現在の武器から柄・刃先の3D位置を計算して取得
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