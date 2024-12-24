//=============================================================================
//
// エネミーモデル処理 [Ground.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "ground.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_GROUND		"data/MODEL/tree.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define GROUND_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define GROUND_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる


//=============================================================================
// 初期化処理
//=============================================================================
Ground::Ground()
{
	for (int i = 0; i < MAX_GROUND; i++)
	{
		GameObject<ModelInstance>* GO = new GameObject<ModelInstance>();
		GO->Instantiate(MODEL_GROUND);

		GO->SetPosition(XMFLOAT3(-50.0f + i * 130.0f, -70.0f, -1720.0f - i * 130.0f));
		GO->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
		GO->SetScale(XMFLOAT3(12.5f, 12.5f, 12.5f));

		// モデルのディフューズを保存しておく。色変え対応の為。
		Model* pModel = GO->GetModel();
		pModel->GetModelDiffuse(&GO->GetDiffuse()[0]);

		groundGO.push_back(GO);

	}
}

//=============================================================================
// 終了処理
//=============================================================================
Ground::~Ground()
{
	int size = groundGO.getSize();
	for (int i = 0; i < size; i++)
	{
		if (groundGO[i])
			delete groundGO[i];
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void Ground::Update(void)
{
	int size = groundGO.getSize();

	for (int i = 0; i < size; i++)
	{
		if (groundGO[i]->GetUse() == false) continue;

		groundGO[i]->Update();
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void Ground::Draw(void)
{
	int size = groundGO.getSize();

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < size; i++)
	{
		if (groundGO[i]->GetUse() == false) continue;

		groundGO[i]->Draw();
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}
