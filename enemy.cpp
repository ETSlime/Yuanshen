//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "enemy.h"
#include "shadow.h"
#include "camera.h"
#include "MapEditor.h"
#include "score.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/neko.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define ENEMY_SHADOW_SIZE	(1.1f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる
#define ENEMY_SCALE			(1.8f)

#define	TEXTURE_MAX			(2)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static UISprite			g_HPGauge[MAX_ENEMY];

static ID3D11Buffer* g_VertexBuffer = NULL;	// 頂点バッファ
static char* g_TextureName[] = {
	"data/TEXTURE/EnemyHPGauge.png",
	"data/TEXTURE/EnemyHPGauge_bg.png",
};
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報


//void UpdateEnemy(void)
//{
//	// エネミーを動かす場合は、影も合わせて動かす事を忘れないようにね！
//	for (int i = 0; i < MAX_ENEMY; i++)
//	{
//		
//
//		if (g_Enemy[i].use == true)		// このエネミーが使われている？
//		{								// Yes
//			UpdateEditorSelect(&g_Enemy[i], GetMousePosX(), GetMousePosY());
//				// 移動処理
//			if (g_Enemy[i].tblMax > 0 && g_Enemy[i].pModel->GetIsModelSelected() == FALSE)	// 線形補間を実行する？
//			{	// 線形補間の処理
//				int nowNo = (int)g_Enemy[i].time;			// 整数分であるテーブル番号を取り出している
//				int maxNo = g_Enemy[i].tblMax;				// 登録テーブル数を数えている
//				int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
//				INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Enemy[i].tblNo];	// 行動テーブルのアドレスを取得
//
//				XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
//				XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
//				XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換
//
//				XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
//				XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
//				XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している
//
//				float nowTime = g_Enemy[i].time - nowNo;	// 時間部分である少数を取り出している
//
//				Pos *= nowTime;								// 現在の移動量を計算している
//				Rot *= nowTime;								// 現在の回転量を計算している
//				Scl *= nowTime;								// 現在の拡大率を計算している
//
//				// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
//				float oldY = g_Enemy[i].pos.y;
//				XMStoreFloat3(&g_Enemy[i].pos, nowPos + Pos);
//				g_Enemy[i].pos.y += oldY;
//
//				// 計算して求めた回転量を現在の移動テーブルに足している
//				XMStoreFloat3(&g_Enemy[i].rot, nowRot + Rot);
//
//				// 計算して求めた拡大率を現在の移動テーブルに足している
//				XMStoreFloat3(&g_Enemy[i].scl, nowScl + Scl);
//
//				XMVECTOR direction = XMVectorSubtract(nowPos, Pos);
//				direction = XMVector3Normalize(direction);
//				g_Enemy[i].targetDir = atan2(XMVectorGetZ(direction), XMVectorGetX(direction));
//				g_Enemy[i].targetDir += XM_PI / 2;
//				// frameを使て時間経過処理をする
//				g_Enemy[i].time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
//				if ((int)g_Enemy[i].time >= maxNo)			// 登録テーブル最後まで移動したか？
//				{
//					g_Enemy[i].time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
//				}
//			}
//
//			float deltaDir = g_Enemy[i].targetDir - g_Enemy[i].dir;
//			if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
//			if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
//			g_Enemy[i].dir += deltaDir * ROTATION_SPEED;
//			if (i != 4)
//				g_Enemy[i].rot.y = g_Enemy[i].dir;
//
//			PlayEnemyWalkAnim(&g_Enemy[i]);
//			if (g_Enemy[i].state == ENEMY_WALK)
//			{
//				PlayEnemyWalkAnim(&g_Enemy[i]);
//			}
//
//			XMFLOAT3 pos = g_Enemy[i].pos;
//			pos.y = (-40.0f - ENEMY_OFFSET_Y - 0.1f);
//			SetPositionShadow(g_Enemy[i].shadowIdx, pos);
//
//			UpdateHPGauge(i);
//
//			if (g_Enemy[i].HP <= 0.0f)
//			{
//				g_Enemy[i].use = FALSE;
//				AddScore(100);
//			}
//				
//
//		}
//	}
//
//
//
//
//#ifdef _DEBUG
//
//	//if (GetKeyboardTrigger(DIK_P))
//	//{
//	//	// モデルの色を変更できるよ！半透明にもできるよ。
//	//	for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
//	//	{
//	//		SetModelDiffuse(&g_Enemy[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
//	//	}
//	//}
//
//	//if (GetKeyboardTrigger(DIK_L))
//	//{
//	//	// モデルの色を元に戻している
//	//	for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
//	//	{
//	//		SetModelDiffuse(&g_Enemy[0].model, j, g_Enemy[0].diffuse[j]);
//	//	}
//	//}
//
//	if (GetKeyboardTrigger(DIK_P))
//	{
//		g_Enemy[1].state = ENEMY_WALK;
//	}
//#endif
//
//	
//
//}

void Enemy::UpdateEditorSelect(int sx, int sy)
{
	if (MapEditor::get_instance().GetOnEditorCursor() == TRUE)
	{
		return;
	}
		
	this->SetIsCursorIn(FALSE);

	CAMERA* camera = GetCamera();
	XMMATRIX P = XMLoadFloat4x4(&camera->mtxProjection);

	// Compute picking ray in view space.
	float vx = (+2.0f * sx / SCREEN_WIDTH - 1.0f) / P.r[0].m128_f32[0];
	float vy = (-2.0f * sy / SCREEN_HEIGHT + 1.0f) / P.r[1].m128_f32[1];

	// Ray definition in view space.
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	// Tranform ray to local space of Mesh.
	XMMATRIX V = XMLoadFloat4x4(&camera->mtxView);
	XMMATRIX invView = XMLoadFloat4x4(&camera->mtxInvView);

	XMMATRIX W = instance.transform.mtxWorld;
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// Make the ray direction unit length for the intersection tests.
	rayDir = XMVector3Normalize(rayDir);
	
	XMVECTOR minPoint = XMLoadFloat3(&instance.pModel->GetBondingBox().minPoint);
	XMVECTOR maxPoint = XMLoadFloat3(&instance.pModel->GetBondingBox().maxPoint);


	float tMin = 0.0f;
	float tMax = FLT_MAX;
	BOOL intersect = FALSE;
	for (int i = 0; i < 3; ++i) 
	{
		float rayOriginComponent = XMVectorGetByIndex(rayOrigin, i);
		float rayDirComponent = XMVectorGetByIndex(rayDir, i);
		float minComponent = XMVectorGetByIndex(minPoint, i);
		float maxComponent = XMVectorGetByIndex(maxPoint, i);

		if (fabs(rayDirComponent) < 1e-8f) {
			if (rayOriginComponent < minComponent || rayOriginComponent > maxComponent) 
			{
				intersect =  FALSE;
				return;
			}
		}
		else 
		{
			float t1 = (minComponent - rayOriginComponent) / rayDirComponent;
			float t2 = (maxComponent - rayOriginComponent) / rayDirComponent;

			if (t1 > t2) 
			{
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			tMin = max(tMin, t1);
			tMax = min(tMax, t2);

			if (tMin > tMax) 
			{
				intersect = FALSE;
				return;
			}
		}
	}

	this->SetIsCursorIn(TRUE);


	this->UpdateModelEditor();

	PrintDebugProc("Enemy:X:%f Y:%f Z:%f\n", instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);

}

//void UpdateHPGauge(int idx)
//{
//	g_HPGauge[idx].pos = g_Enemy[idx].pos;
//	g_HPGauge[idx].pos.y += ENEMY_SIZE;
//	g_HPGauge[idx].pos.x -= 15.0f;
//}

void Enemy::PlayEnemyWalkAnim(void)
{
	if (this->GetIsModelSelected() == TRUE) return;

	float angle = (XM_PI / JUMP_CNT_MAX) * instance.jumpCnt;
	instance.jumpCnt++;

	if (instance.jumpUp == FALSE)
	{
		instance.transform.scl.y += 0.008f;
		instance.transform.pos.y += instance.jumpYMax * cosf(angle);
		//SetShadowSize(enemy->shadowIdx, enemy->shadowSize, enemy->shadowSize);
	}
	else
	{
		instance.transform.scl.y -= 0.008f;
		instance.transform.pos.y -= instance.jumpYMax * cosf(angle);
		//SetShadowSize(enemy->shadowIdx, enemy->shadowSize, enemy->shadowSize);
	}

	if (instance.jumpCnt > JUMP_CNT_MAX * 0.5f)
	{
		instance.jumpUp = !instance.jumpUp;
		instance.jumpCnt = 0;
	}
}

//void DrawHPGauge(int idx)
//{
//	// ライティングを無効
//	SetLightEnable(FALSE);
//
//	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
//	CAMERA* cam = GetCamera();
//
//	// 頂点バッファ設定
//	UINT stride = sizeof(VERTEX_3D);
//	UINT offset = 0;
//	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
//
//	// プリミティブトポロジ設定
//	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	if (g_HPGauge[idx].bUse)
//	{
//		// ワールドマトリックスの初期化
//		mtxWorld = XMMatrixIdentity();
//
//		// ビューマトリックスを取得
//		mtxView = XMLoadFloat4x4(&cam->mtxView);
//
//
//		// なにかするところ
//		// 正方行列（直交行列）を転置行列させて逆行列を作ってる版(速い)
//		mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
//		mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
//		mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];
//
//		mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
//		mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
//		mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];
//
//		mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
//		mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
//		mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];
//
//		int ratio = g_Enemy[idx].HP / g_Enemy[idx].maxHP;
//		//MakeVertexHPGauge(15.0f * ratio, 5.0f);
//
//		// スケールを反映
//		mtxScl = XMMatrixScaling(g_HPGauge[idx].scl.x, g_HPGauge[idx].scl.y, g_HPGauge[idx].scl.z);
//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);
//
//		// 移動を反映
//		mtxTranslate = XMMatrixTranslation(g_HPGauge[idx].pos.x, g_HPGauge[idx].pos.y, g_HPGauge[idx].pos.z);
//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);
//
//		// ワールドマトリックスの設定
//		SetCurrentWorldMatrix(&mtxWorld);
//
//
//		// マテリアル設定
//		SetMaterial(g_HPGauge[idx].material);
//
//		// テクスチャ設定
//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
//
//		// ポリゴンの描画
//		GetDeviceContext()->Draw(4, 0);
//	}
//
//	// ライティングを有効に
//	SetLightEnable(TRUE);
//}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexHPGauge(int width, int height)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = width;
	float fHeight = height;

	// 頂点座標の設定
	//vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	//vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	//vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	//vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);
	vertex[0].Position = XMFLOAT3(0.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth, 0.0f, 0.0f);

	// 法線の設定
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

//=============================================================================
// 初期化処理
//=============================================================================
Enemy::Enemy(char* enemyPath, Transform trans)
{
	Instantiate(enemyPath);
	MapEditor::get_instance().AddToList(this);

	instance.load = true;

	instance.attribute.maxHP = 20.0f;
	instance.attribute.HP = 20.0f;

	instance.attribute.spd = 0.0f;			// 移動スピードクリア
	instance.attribute.size = ENEMY_SIZE;	// 当たり判定の大きさ

	instance.transform.pos = trans.pos;
	instance.transform.rot = trans.rot;
	instance.transform.scl = trans.scl;

	instance.moveTbl = nullptr;


	// モデルのディフューズを保存しておく。色変え対応の為。
	instance.pModel->GetModelDiffuse(&instance.diffuse[0]);


	instance.jumpUp = FALSE;
	instance.jumpCnt = 0;
	instance.jumpYMax = 0.9f;

	instance.move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

	instance.time = 0.0f;			// 線形補間用のタイマーをクリア
	instance.tblMax = 0;			// 再生する行動データテーブルのレコード数をセット


	instance.use = true;		// true:生きてる
}

//=============================================================================
// 更新処理
//=============================================================================
void Enemy::Update(void)
{
	if (instance.use == true)		// このエネミーが使われている？
	{								// Yes
		GameObject::Update();

		UpdateEditorSelect(GetMousePosX(), GetMousePosY());
		// 移動処理
		BOOL isSelected = this->GetIsModelSelected();
		if (instance.tblMax > 0 && isSelected == FALSE)	// 線形補間を実行する？
		{	// 線形補間の処理
			int nowNo = (int)instance.time;			// 整数分であるテーブル番号を取り出している
			int maxNo = instance.tblMax;				// 登録テーブル数を数えている
			int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
			INTERPOLATION_DATA* tbl = instance.moveTbl;//g_MoveTblAdr[instance.tblNo];	// 行動テーブルのアドレスを取得

			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

			XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
			XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
			XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

			float nowTime = instance.time - nowNo;	// 時間部分である少数を取り出している

			Pos *= nowTime;								// 現在の移動量を計算している
			Rot *= nowTime;								// 現在の回転量を計算している
			Scl *= nowTime;								// 現在の拡大率を計算している

			// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
			float oldY = instance.transform.pos.y;
			XMStoreFloat3(&instance.transform.pos, nowPos + Pos);
			instance.transform.pos.y += oldY;

			// 計算して求めた回転量を現在の移動テーブルに足している
			XMStoreFloat3(&instance.transform.rot, nowRot + Rot);

			// 計算して求めた拡大率を現在の移動テーブルに足している
			XMStoreFloat3(&instance.transform.scl, nowScl + Scl);

			XMVECTOR direction = XMVectorSubtract(nowPos, Pos);
			direction = XMVector3Normalize(direction);
			instance.targetDir = atan2(XMVectorGetZ(direction), XMVectorGetX(direction));
			instance.targetDir += XM_PI / 2;
			// frameを使て時間経過処理をする
			instance.time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
			if ((int)instance.time >= maxNo)			// 登録テーブル最後まで移動したか？
			{
				instance.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
			}
		}

		float deltaDir = instance.targetDir - instance.dir;
		if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
		if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
		instance.dir += deltaDir * ROTATION_SPEED;
		//if (i != 4)
		//	instance.rot.y = g_Enemy[i].dir;

		PlayEnemyWalkAnim();
		if (instance.state == ENEMY_WALK)
		{
			PlayEnemyWalkAnim();
		}

		XMFLOAT3 pos = instance.transform.pos;
		pos.y = (-40.0f - ENEMY_OFFSET_Y - 0.1f);
		//SetPositionShadow(g_Enemy[i].shadowIdx, pos);

		//UpdateHPGauge(i);

		if (instance.attribute.HP <= 0.0f)
		{
			instance.use = FALSE;
			AddScore(100);
		}

	}
}

//=============================================================================
// 描画処理
//=============================================================================
void Enemy::Draw(void)
{
	// ワールドマトリックスの設定
	SetCurrentWorldMatrix(&instance.transform.mtxWorld);

	// モデル描画
	this->DrawModelEditor();
	instance.pModel->DrawBoundingBox();
}

void Enemy::SetMoveTbl(MoveTable moveTbl)
{
	instance.moveTbl = moveTbl.moveTblAddr;
	instance.tblMax = moveTbl.size;
}

EnemyManager::EnemyManager()
{

}

void EnemyManager::Init()
{
	InitializeMoveTbl();
	for (int i = 0; i < 4; i++)
	{
		Transform trans;
		trans.pos = XMFLOAT3(-50.0f + i * 30.0f, -70.0f, 20.0f);
		trans.scl = XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE);
		SpawnEnemy(MODEL_ENEMY, trans, moveTbls[i]);
	}
}



void EnemyManager::SpawnEnemy(char* enemyPath, Transform trans, MoveTable moveTbl)
{
	Enemy* enemy = new Enemy(enemyPath, trans);
	enemy->SetMoveTbl(moveTbl);
	enemyList.push_back(enemy);
}

void EnemyManager::Draw(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		// モデル描画
		cur->data->Draw();
		cur = cur->next;
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

void EnemyManager::Update(void)
{
	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		cur->data->Update();
		if (cur->data->GetUse() == FALSE)
		{
			Node<Enemy*>* toDelete = cur;
			cur = cur->next;
			delete toDelete->data;
			enemyList.remove(toDelete);
		}
		else
			cur = cur->next;
	}
}

void EnemyManager::InitializeMoveTbl()
{
	static INTERPOLATION_DATA moveTbl[] = {
		//座標									回転率							拡大率					時間
		{ XMFLOAT3(50.0f,  0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(250.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 }
	};
	moveTbls.push_back({ moveTbl , sizeof(moveTbl) / sizeof(INTERPOLATION_DATA) });

	static INTERPOLATION_DATA moveTbl2[] = {
		//座標									回転率							拡大率							時間
		{ XMFLOAT3(450.0f,  0.0f, 55.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(250.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
	};

	moveTbls.push_back({ moveTbl2 , sizeof(moveTbl2) / sizeof(INTERPOLATION_DATA) });

	static INTERPOLATION_DATA moveTbl3[] = {
		//座標									回転率							拡大率							時間
		{ XMFLOAT3(50.0f,  0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(250.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
	};

	moveTbls.push_back({ moveTbl3 , sizeof(moveTbl3) / sizeof(INTERPOLATION_DATA) });

	static INTERPOLATION_DATA moveTbl4[] = {
		//座標									回転率							拡大率							時間
		{ XMFLOAT3(471.0f,  0.0f, -437.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(752.0f, 0.0f, 742.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
	};
	moveTbls.push_back({ moveTbl4 , sizeof(moveTbl4) / sizeof(INTERPOLATION_DATA) });
}

