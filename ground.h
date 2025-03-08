//=============================================================================
//
// エネミーモデル処理 [Ground.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "Grass.h"
#include "Town.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
// 読み込むモデル名
#define	MODEL_ENVIRONMENT_PATH		"data/MODEL/Environment/"
#define	MODEL_TOWN_PATH				"data/MODEL/Environment/Knight"
#define	MODEL_FIELD					"data/MODEL/Environment/Land.obj"
#define	MODEL_TREE_NAME				"Tree.fbx"
#define	MODEL_FIELD_NAME			"Land.fbx"
#define	MODEL_BONFIRE_NAME			"bonfire.fbx"
#define	MODEL_TOWN_NAME				"kt.fbx"

#define WORLD_MAX			(XMFLOAT3(200000.0f, 200000.0f, 200000.0f))
#define WORLD_MIN			(XMFLOAT3(-200000.0f, -200000.0f, -200000.0f))
//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct GROUND
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	int					state;
	bool				use;
	bool				load;
	Model*				pModel;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号

};



class Ground
{
public:
	Ground();
	~Ground();
	void Update(void);
	void Draw(void);
private:
	SimpleArray<GameObject<SkinnedMeshModelInstance>*> skinnedMeshGroundGO;
	SimpleArray<GameObject<ModelInstance>*>	groundGO;
	Town* town;
	Grass* grass;
	Renderer& renderer = Renderer::get_instance();
};