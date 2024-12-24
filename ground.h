//=============================================================================
//
// エネミーモデル処理 [ground.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_GROUND		(15)					// エネミーの数
#define	GROUND_SIZE		(5.0f)				// 当たり判定の大きさ
#define ROTATION_SPEED				(0.18f)

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
	SimpleArray<GameObject<ModelInstance>*> groundGO;
};