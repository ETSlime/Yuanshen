//=============================================================================
//
// エネミーモデル処理 [Enemy.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "DoubleLinkedList.h"
#include "SingletonBase.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ENEMY		(5)					// エネミーの数
#define JUMP_CNT_MAX	(120)
#define	ENEMY_SIZE		(65.0f)				// 当たり判定の大きさ
#define ROTATION_SPEED				(0.18f)
#define ENEMY_HIT_WINDOW			(60)
#define ENEMY_RESPAWON_TIME			(2000.0f)

enum
{
	ENEMY_IDLE,
	ENEMY_WALK,
};

enum class EnemyType
{
	Hilichurl,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
//struct ENEMY
//{
//
//	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
//	XMFLOAT3			pos;				// モデルの位置
//	XMFLOAT3			rot;				// モデルの向き(回転)
//	XMFLOAT3			scl;				// モデルの大きさ(スケール)
//
//
//	bool				use;
//	bool				load;
//	Model*				pModel;				// モデル情報
//	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色
//
//
//	int					shadowIdx;			// 影のインデックス番号
//	float				shadowSize;
//};

struct EnemyAttribute
{
	float				HP;
	float				maxHP;

	float				timer;
	float				nextChangeDirTime;
	float				moveDistance;
	float				distPlayer;

	bool				isWaiting;
	bool				isSurprised;
	bool				isChasingPlayer;
	bool				isSitting;
	bool				isInCooldown;

	float				viewAngle;     // 視野角 (例: 60° = 60 * (PI / 180))
	float				viewDistance;  // 視認範囲
	float				chaseRange;    // 追跡範囲
	float				attackRange;   // 攻撃範囲
	float				attackCooldownTimer;

	EnemyState			initState;
	Transform			initTrans;

	float				cooldownMoveDirection;
	float				cooldownWaitTime;
	float				cooldownMoveDistance;
	
	float				waitTime;
	EnemyType			enemyType;

	bool				die;
	bool				isDead;
	bool				startFadeOut;
	bool				randomMove;
	float				respawnTimer;


	INTERPOLATION_DATA* moveTbl;
	XMFLOAT3	move;			// 移動速度
	float		time;			// 線形補間用
	int			tblMax;			// そのテーブルのデータ数

	void Initialize()
	{
		isWaiting = false;
		isSurprised = false;
		isChasingPlayer = false;
		isSitting = false;
		isInCooldown = false;
		die = false;
		isDead = false;
		startFadeOut = false;
		randomMove = true;

		timer = 0.0f;
		nextChangeDirTime = 0.0f;
		moveDistance = 0.0f;
		distPlayer = 0.0f;
		waitTime = 0.0f;
		cooldownMoveDirection = 0.0f;
		cooldownWaitTime = 0.0f;
		cooldownMoveDistance = 0.0f;
	}
};

struct UISprite
{
	XMFLOAT3	pos;			// 位置
	XMFLOAT3	scl;			// スケール
	MATERIAL	material;		// マテリアル
	float		fWidth;			// 幅
	float		fHeight;		// 高さ
	BOOL		bUse;			// 使用しているかどうか

};

struct MoveTable
{
	INTERPOLATION_DATA* moveTblAddr;
	int size;
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexHPGauge(int w, int h);

class Player;

class Enemy : public GameObject<SkinnedMeshModelInstance>
{
public:
	Enemy(EnemyType enemyType, Transform trans);
	inline EnemyAttribute* GetEnemyAttribute(void) { return &enemyAttr; }
	void Update(void) override;
	void Draw(void) override;
	inline INTERPOLATION_DATA* GetMoveTbl() { return enemyAttr.moveTbl; }
	void SetMoveTbl(const MoveTable* moveTbl);

	void SetPlayer(const Player* player) { this->player = player; }

	void ReduceHP(float amount) { enemyAttr.HP -= amount; }

	void SetRandomMove(bool random) { enemyAttr.randomMove = random; }

protected:
	EnemyAttribute enemyAttr;
	const Player* player;
	virtual void Initialize(void);
private:
	void SetNewPosTarget(void);
	void StartWaiting(void);
	bool DetectPlayer(void);
	void ChasePlayer(void);
	void AttackPlayer(float dist);
	void CooldownMovement(void);
	//void UpdateEditorSelect(int sx, int sy);
};

class EnemyManager : public SingletonBase<EnemyManager>
{
public:
	EnemyManager();
	void Init(const Player* player = nullptr);
	void SpawnEnemy(EnemyType enemType, Transform trans, EnemyState initState, MoveTable* moveTbl = nullptr);
	void Draw(void);
	void Update(void);
	const DoubleLinkedList<Enemy*>* GetEnemy() { return &enemyList; }
	Renderer& renderer = Renderer::get_instance();
private:
	void InitializeMoveTbl();

	DoubleLinkedList<Enemy*> enemyList;
	SimpleArray<MoveTable> moveTbls;
	const Player* player;
};