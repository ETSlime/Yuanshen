//=============================================================================
//
// エネミーモデル処理 [Enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "Enemy.h"
#include "camera.h"
#include "MapEditor.h"
#include "score.h"
#include "Hilichurl.h"
#include "Player.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define	VALUE_MOVE			(3.5f)							// 移動量
#define VALUE_RUN			(120.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量

#define ENEMY_SHADOW_SIZE	(1.1f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる
#define ENEMY_SCALE			(1.8f)

#define	TEXTURE_MAX			(2)

#define ROTATION_SPEED				(0.18f)
#define FALLING_SPEED		(5.0f)
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


//void Enemy::UpdateEditorSelect(int sx, int sy)
//{
//	if (MapEditor::get_instance().GetOnEditorCursor() == TRUE)
//	{
//		return;
//	}
//		
//	this->SetIsCursorIn(FALSE);
//
//	CAMERA* camera = GetCamera();
//	XMMATRIX P = XMLoadFloat4x4(&camera->mtxProjection);
//
//	// Compute picking ray in view space.
//	float vx = (+2.0f * sx / SCREEN_WIDTH - 1.0f) / P.r[0].m128_f32[0];
//	float vy = (-2.0f * sy / SCREEN_HEIGHT + 1.0f) / P.r[1].m128_f32[1];
//
//	// Ray definition in view space.
//	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
//	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);
//
//	// Tranform ray to local space of Mesh.
//	XMMATRIX V = XMLoadFloat4x4(&camera->mtxView);
//	XMMATRIX invView = XMLoadFloat4x4(&camera->mtxInvView);
//
//	XMMATRIX W = instance.transform.mtxWorld;
//	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
//
//	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);
//
//	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
//	rayDir = XMVector3TransformNormal(rayDir, toLocal);
//
//	// Make the ray direction unit length for the intersection tests.
//	rayDir = XMVector3Normalize(rayDir);
//	
//	XMVECTOR minPoint = XMLoadFloat3(&instance.pModel->GetBoundingBox().minPoint);
//	XMVECTOR maxPoint = XMLoadFloat3(&instance.pModel->GetBoundingBox().maxPoint);
//
//
//	float tMin = 0.0f;
//	float tMax = FLT_MAX;
//	BOOL intersect = FALSE;
//	for (int i = 0; i < 3; ++i) 
//	{
//		float rayOriginComponent = XMVectorGetByIndex(rayOrigin, i);
//		float rayDirComponent = XMVectorGetByIndex(rayDir, i);
//		float minComponent = XMVectorGetByIndex(minPoint, i);
//		float maxComponent = XMVectorGetByIndex(maxPoint, i);
//
//		if (fabs(rayDirComponent) < 1e-8f) {
//			if (rayOriginComponent < minComponent || rayOriginComponent > maxComponent) 
//			{
//				intersect =  FALSE;
//				return;
//			}
//		}
//		else 
//		{
//			float t1 = (minComponent - rayOriginComponent) / rayDirComponent;
//			float t2 = (maxComponent - rayOriginComponent) / rayDirComponent;
//
//			if (t1 > t2) 
//			{
//				float temp = t1;
//				t1 = t2;
//				t2 = temp;
//			}
//
//			tMin = max(tMin, t1);
//			tMax = min(tMax, t2);
//
//			if (tMin > tMax) 
//			{
//				intersect = FALSE;
//				return;
//			}
//		}
//	}
//
//	this->SetIsCursorIn(TRUE);
//
//
//	this->UpdateModelEditor();
//
//	PrintDebugProc("Enemy:X:%f Y:%f Z:%f\n", instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);
//
//}

//void UpdateHPGauge(int idx)
//{
//	g_HPGauge[idx].pos = g_Enemy[idx].pos;
//	g_HPGauge[idx].pos.y += ENEMY_SIZE;
//	g_HPGauge[idx].pos.x -= 15.0f;
//}

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
//HRESULT MakeVertexHPGauge(int width, int height)
//{
//	// 頂点バッファ生成
//	D3D11_BUFFER_DESC bd;
//	ZeroMemory(&bd, sizeof(bd));
//	bd.Usage = D3D11_USAGE_DYNAMIC;
//	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
//	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//	renderer.GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);
//
//	// 頂点バッファに値をセットする
//	D3D11_MAPPED_SUBRESOURCE msr;
//	renderer.GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
//
//	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;
//
//	float fWidth = width;
//	float fHeight = height;
//
//	// 頂点座標の設定
//	//vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
//	//vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
//	//vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
//	//vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);
//	vertex[0].Position = XMFLOAT3(0.0f, fHeight, 0.0f);
//	vertex[1].Position = XMFLOAT3(fWidth, fHeight, 0.0f);
//	vertex[2].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
//	vertex[3].Position = XMFLOAT3(fWidth, 0.0f, 0.0f);
//
//	// 法線の設定
//	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
//	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
//	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
//	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
//
//	// 拡散光の設定
//	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//
//	// テクスチャ座標の設定
//	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
//	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
//	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
//	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
//
//	renderer.GetDeviceContext()->Unmap(g_VertexBuffer, 0);
//
//	return S_OK;
//}

//=============================================================================
// 初期化処理
//=============================================================================
Enemy::Enemy(EnemyType enemyType, Transform trans)
{

	//MapEditor::get_instance().AddToList(this);
	enemyAttr.enemyType = enemyType;

	instance.load = true;



	instance.attributes.spd = VALUE_MOVE;
	switch (enemyAttr.enemyType)
	{
	case EnemyType::Hilichurl:
		enemyAttr.maxHP = HILI_MAX_HP;
		enemyAttr.HP = HILI_MAX_HP;
		enemyAttr.viewAngle = HILI_VIEW_ANGLE;
		enemyAttr.viewDistance = HILI_VIEW_DISTANCE;
		enemyAttr.chaseRange = HILI_CHASING_RANGE;
		enemyAttr.attackRange = HILI_ATTACK_RANGE;
		break;
	default:
		break;
	}

	instance.transform.pos = trans.pos;
	instance.transform.rot = trans.rot;
	instance.transform.scl = trans.scl;
	enemyAttr.initTrans.pos = trans.pos;
	enemyAttr.initTrans.rot = trans.rot;
	enemyAttr.initTrans.scl = trans.scl;

	enemyAttr.moveTbl = nullptr;


	enemyAttr.move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

	enemyAttr.time = 0.0f;			// 線形補間用のタイマーをクリア
	enemyAttr.tblMax = 0;			// 再生する行動データテーブルのレコード数をセット


	instance.use = true;		// true:生きてる
}

//=============================================================================
// 更新処理
//=============================================================================
void Enemy::Update(void)
{
	if (instance.use == true)		// このエネミーが使われている？
	{								// Yes

		if (enemyAttr.isDead == true)
		{
			enemyAttr.respawnTimer--;
			if (enemyAttr.respawnTimer <= 0.0f)
			{
				enemyAttr.respawnTimer = 0.0f;
				Initialize();
			}
			else
				return;
		}

		if (instance.renderProgress.progress < 1.0f && !enemyAttr.startFadeOut)
		{
			instance.renderProgress.isRandomFade = true;
			instance.renderProgress.progress += 0.01f;
			if (instance.renderProgress.progress > 1.0f)
			{
				instance.renderProgress.isRandomFade = false;
				instance.renderProgress.progress = 1.0f;
			}
		}
		else if (enemyAttr.startFadeOut == true)
		{
			instance.renderProgress.isRandomFade = true;
			instance.renderProgress.progress -= 0.01f;
			if (instance.renderProgress.progress <= 0.0f)
			{
				instance.renderProgress.isRandomFade = false;
				instance.renderProgress.progress = 0.0f;
				enemyAttr.isDead = true;
			}
			return;
		}

		GameObject::Update();

		//UpdateHPGauge(i);

		if (enemyAttr.HP <= 0.0f)
		{
			enemyAttr.respawnTimer = ENEMY_RESPAWON_TIME;
			enemyAttr.die = true;
			return;
		}

		enemyAttr.timer++;

		if (instance.attributes.isHit1 ||
			instance.attributes.isHit2)
		{
			instance.attributes.hitTimer--;
			if (instance.attributes.hitTimer < 0)
			{
				instance.attributes.isHit1 = false;
				instance.attributes.isHit2 = false;
			}
		}

		if (instance.attributes.isGrounded == false)
		{
			instance.transform.pos.y -= FALLING_SPEED;
		}

		if (instance.attributes.isHit1 ||
			instance.attributes.isHit2 ||
			instance.attributes.isAttacking)
			return;

		if (enemyAttr.isInCooldown) 
		{
			enemyAttr.attackCooldownTimer--;
			if (enemyAttr.attackCooldownTimer <= 0.0f)
			{
				enemyAttr.isInCooldown = false;
			}
			else 
			{
				CooldownMovement();
				return;
			}
		}

		// プレイヤーを発見するかチェック
		if (DetectPlayer()) 
		{
			if (enemyAttr.isSitting == false)
			{
				if (enemyAttr.isChasingPlayer == false)
					enemyAttr.isSurprised = true;
				if (enemyAttr.isSurprised == false)
					enemyAttr.isChasingPlayer = true;
			}

		}

		// プレイヤーを追跡する状態
		if (enemyAttr.isChasingPlayer)
		{
			ChasePlayer();
		}
		else if (enemyAttr.randomMove == true)
		{
			// 通常の移動ロジック
			if (enemyAttr.isWaiting)
			{
				if (enemyAttr.timer >= enemyAttr.waitTime)
				{
					enemyAttr.isWaiting = false;
					SetNewPosTarget();
				}
			}
			else
			{
				// 移動処理
				float moveStep = instance.attributes.spd;
				enemyAttr.moveDistance -= moveStep;

				// 目標距離に達した場合、待機モードに入る
				if (enemyAttr.moveDistance <= 0.0f)
				{
					instance.attributes.isMoving = false;
					StartWaiting();
				}
				else
				{
					// 入力のあった方向へ向かせて移動させる
					instance.attributes.isMoving = true;
					instance.transform.pos.x += sinf(instance.transform.rot.y) * instance.attributes.spd;
					instance.transform.pos.z += cosf(instance.transform.rot.y) * instance.attributes.spd;
				}
			}
		}

		{
			//UpdateEditorSelect(GetMousePosX(), GetMousePosY());


//BOOL isSelected = this->GetIsModelSelected();
//if (attribute.tblMax > 0 && isSelected == FALSE)	// 線形補間を実行する？
//{	// 線形補間の処理
//	int nowNo = (int)attribute.time;			// 整数分であるテーブル番号を取り出している
//	int maxNo = attribute.tblMax;				// 登録テーブル数を数えている
//	int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
//	INTERPOLATION_DATA* tbl = attribute.moveTbl;//g_MoveTblAdr[instance.tblNo];	// 行動テーブルのアドレスを取得

//	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
//	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
//	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

//	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
//	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
//	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

//	float nowTime = attribute.time - nowNo;	// 時間部分である少数を取り出している

//	Pos *= nowTime;								// 現在の移動量を計算している
//	Rot *= nowTime;								// 現在の回転量を計算している
//	Scl *= nowTime;								// 現在の拡大率を計算している

//	// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
//	float oldY = instance.transform.pos.y;
//	XMStoreFloat3(&instance.transform.pos, nowPos + Pos);
//	instance.transform.pos.y += oldY;

//	// 計算して求めた回転量を現在の移動テーブルに足している
//	XMStoreFloat3(&instance.transform.rot, nowRot + Rot);

//	// 計算して求めた拡大率を現在の移動テーブルに足している
//	XMStoreFloat3(&instance.transform.scl, nowScl + Scl);

//	XMVECTOR direction = XMVectorSubtract(nowPos, Pos);
//	direction = XMVector3Normalize(direction);
//	attribute.targetDir = atan2(XMVectorGetZ(direction), XMVectorGetX(direction));
//	attribute.targetDir += XM_PI / 2;
//	// frameを使て時間経過処理をする
//	attribute.time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
//	if ((int)attribute.time >= maxNo)			// 登録テーブル最後まで移動したか？
//	{
//		attribute.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
//	}
//}

//float deltaDir = attribute.targetDir - attribute.dir;
//if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
//if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
//attribute.dir += deltaDir * ROTATION_SPEED;
////if (i != 4)
////	instance.rot.y = g_Enemy[i].dir;


//XMFLOAT3 pos = instance.transform.pos;
//pos.y = (-40.0f - ENEMY_OFFSET_Y - 0.1f);
//SetPositionShadow(g_Enemy[i].shadowIdx, pos);
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void Enemy::Draw(void)
{

}

void Enemy::SetMoveTbl(const MoveTable* moveTbl)
{
	if (moveTbl)
	{
		enemyAttr.moveTbl = moveTbl->moveTblAddr;
		enemyAttr.tblMax = moveTbl->size;
	}
}

void Enemy::Initialize(void)
{
	enemyAttr.Initialize();

	instance.transform.pos = enemyAttr.initTrans.pos;
	instance.transform.rot = enemyAttr.initTrans.rot;
	instance.transform.scl = enemyAttr.initTrans.scl;

	switch (enemyAttr.enemyType)
	{
	case EnemyType::Hilichurl:
		enemyAttr.maxHP = HILI_MAX_HP;
		enemyAttr.HP = HILI_MAX_HP;
		break;
	default:
		break;
	}

	instance.renderProgress.isRandomFade = true;
	instance.renderProgress.progress = 0.0f;
}

void Enemy::SetNewPosTarget()
{
	instance.transform.rot.y = GetRandFloat(0.0f, XM_2PI);
	enemyAttr.moveDistance = GetRandFloat(800.0f, 4500.0f);
	enemyAttr.nextChangeDirTime = GetRandFloat(1.0f, 3.0f);
	enemyAttr.timer = 0.0f;
}

void  Enemy::StartWaiting() 
{
	enemyAttr.isWaiting = true;
	enemyAttr.waitTime = GetRandFloat(400.0f, 2000.0f);
	enemyAttr.timer = 0.0f;
}

bool Enemy::DetectPlayer(void)
{
	XMVECTOR enemyPosVec = XMLoadFloat3(&instance.transform.pos);
	XMVECTOR playerPosVec = XMLoadFloat3(&player->GetTransform().pos);

	// プレイヤーへの方向ベクトル
	XMVECTOR diff = XMVectorSubtract(playerPosVec, enemyPosVec);
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));

	// 視認範囲外なら発見しない
	if (distSq > enemyAttr.viewDistance * enemyAttr.viewDistance) return false;

	// 正面方向ベクトル（Z軸方向）
	XMVECTOR forward = XMVectorSet(sinf(instance.transform.rot.y), 0.0f, cosf(instance.transform.rot.y), 0.0f);
	diff = XMVector3Normalize(diff); // 正規化

	// 内積で角度を求める
	float dot = XMVectorGetX(XMVector3Dot(forward, diff));
	float angle = acosf(dot);

	return angle <= enemyAttr.viewAngle * 0.5f;
}

void Enemy::ChasePlayer(void)
{
	if (player == nullptr) return;

	Transform playerTransform = player->GetTransform();
	// 敵からプレイヤーへの方向ベクトル
	XMVECTOR enemyPosVec = XMLoadFloat3(&instance.transform.pos);
	XMVECTOR playerPosVec = XMLoadFloat3(&playerTransform.pos);

	// プレイヤーまでの距離
	XMVECTOR diff = XMVectorSubtract(playerPosVec, enemyPosVec);
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));

	if (distSq < enemyAttr.attackRange * enemyAttr.attackRange * 1.1f)
	{
		// 攻撃範囲に入ったら攻撃モード
		AttackPlayer(distSq);
		return;
	}

	if (distSq > enemyAttr.chaseRange * enemyAttr.chaseRange)
	{
		// 追跡範囲を超えたらランダム移動に戻る
		enemyAttr.isChasingPlayer = false;
		SetNewPosTarget();
		return;
	}

	// 目標方向を計算
	float dx = instance.transform.pos.x - playerTransform.pos.x;
	float dz = instance.transform.pos.z - playerTransform.pos.z;

	instance.transform.rot.y = -atan2(dz, dx) - XM_PI * 0.5f;

	// プレイヤー方向に移動
	instance.attributes.isMoving = true;
	float moveStep = instance.attributes.spd;
	instance.transform.pos.x += sinf(instance.transform.rot.y) * moveStep;
	instance.transform.pos.z += cosf(instance.transform.rot.y) * moveStep;
}

void Enemy::AttackPlayer(float dist)
{
	instance.attributes.isAttacking = true;
	enemyAttr.distPlayer = dist;
}

void Enemy::CooldownMovement(void)
{
	if (enemyAttr.cooldownMoveDistance > 0.0f) 
	{
		float moveStep = instance.attributes.spd * 0.3f;
		enemyAttr.cooldownMoveDistance -= moveStep;
		instance.attributes.isMoving = true;
		instance.transform.pos.x += cosf(instance.transform.rot.y) * enemyAttr.cooldownMoveDirection * moveStep;
		instance.transform.pos.z -= sinf(instance.transform.rot.y) * enemyAttr.cooldownMoveDirection * moveStep;

		if (enemyAttr.cooldownMoveDistance <= 0.0f) 
		{
			enemyAttr.cooldownMoveDistance = 0.0f;
			enemyAttr.timer = 0.0f;
		}
	}
	else if (enemyAttr.timer >= enemyAttr.cooldownWaitTime) 
	{
		enemyAttr.cooldownMoveDirection = (GetRand(0, 1) == 0) ? -1.0f : 1.0f;
		enemyAttr.cooldownMoveDistance = enemyAttr.attackCooldownTimer * GetRandFloat(HILI_MIN_COOLDOWN_WAIT_TIME, HILI_MAX_COOLDOWN_WAIT_TIME);
		enemyAttr.cooldownWaitTime = GetRandFloat(0.5f, 1.5f);
	}
	else
		instance.attributes.isMoving = false;

	enemyAttr.attackCooldownTimer--;
	if (enemyAttr.attackCooldownTimer <= 0.0f)
	{
		enemyAttr.isInCooldown = false;
	}
}

EnemyManager::EnemyManager()
{
	player = nullptr;
}

void EnemyManager::Init(const Player* player)
{
	this->player = player;
	Transform trans;
	trans.pos = XMFLOAT3(12937.0f, -2384.0f, -19485.0f);
	trans.scl = HILICHURL_SIZE;
	InitializeMoveTbl();
	for (int i = 0; i < 25; i++)
	{
		float radius = 20000.f;
		float r = GetRandFloat(2000.0f, radius);
		float angle = GetRandFloat(0.0f, 2.0f * 3.14159265359f);
		float x = trans.pos.x + r * cos(angle);
		float z = trans.pos.z + r * sin(angle);
		Transform t = trans;
		t.pos.x = x;
		t.pos.z = z;

		SpawnEnemy(EnemyType::Hilichurl, t, EnemyState::IDLE);
	}


	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::IDLE);

	trans.pos = XMFLOAT3(12759.5f, -2384.0f, -19177.56f);
	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	trans.pos = XMFLOAT3(12404.5f, -2384.0f, -19177.56f);
	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	trans.pos = XMFLOAT3(12227.0f, -2384.0f, -19485.0f);
	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	trans.pos = XMFLOAT3(12404.5f, -2384.0f, -19792.43f);
	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	trans.pos = XMFLOAT3(12759.5f, -2384.0f, -19792.43f);
	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);
}



void EnemyManager::SpawnEnemy(EnemyType enemyType, Transform trans, EnemyState initState, MoveTable* moveTbl)
{
	Enemy* enemy;
	switch (enemyType)
	{
	case EnemyType::Hilichurl:
		enemy = new Hilichurl(trans, initState);
		break;
	default:
		return;
	}
	 
	if (initState == EnemyState::IDLE)
		enemy->SetRandomMove(true);
	else
		enemy->SetRandomMove(false);

	enemy->SetMoveTbl(moveTbl);
	enemy->SetPlayer(player);
	enemyList.push_back(enemy);
}

void EnemyManager::Draw(void)
{
	// カリング無効
	renderer.SetCullingMode(CULL_MODE_NONE);

	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		// モデル描画
		if (cur->data->GetInstance()->renderProgress.progress < 1.0f)
			renderer.SetRenderProgress(cur->data->GetInstance()->renderProgress);

		if (!cur->data->GetEnemyAttribute()->isDead)
			cur->data->Draw();

		if (cur->data->GetInstance()->renderProgress.progress < 1.0f)
		{
			RenderProgressBuffer defaultRenderProgress;
			defaultRenderProgress.isRandomFade = false;
			defaultRenderProgress.progress = 1.0f;
			renderer.SetRenderProgress(defaultRenderProgress);
		}

		cur = cur->next;
	}

	// カリング設定を戻す
	renderer.SetCullingMode(CULL_MODE_BACK);
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

