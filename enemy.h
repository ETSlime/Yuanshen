//=============================================================================
//
// エネミーモデル処理 [Enemy.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "Timer.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ENEMY_HIT_WINDOW			(60)
#define ENEMY_RESPAWN_TIME			(2000.0f)

enum class CooldownState
{
	MOVE,
	WAIT,
};

enum class EnemyType
{
	Hilichurl,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct EnemyAttributes
{
	float				HP;
	float				maxHP;

	float				timer;

	float				moveDuration;	// 移動する時間
	float				moveTimer;		// 移動時間のカウンター
	float				waitTime;		// 待機時間
	float				distPlayerSq;

	bool				fixedDirMove;
	float				fixedDir;

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
	float				attackCooldownMin;
	float				attackCooldownMax;

	float				cooldownWaitTimer;
	float				cooldownMoveDirection;
	float				cooldownOrbitRadius;
	float				cooldownMoveTimer;

	float				cooldownProbability;
	CooldownState		cooldownState;


	EnemyState			initState;
	Transform			initTrans;

	EnemyType			enemyType;

	bool				die;
	bool				isDead;
	bool				startFadeOut;
	bool				randomMove;
	float				respawnTimer;

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

		fixedDirMove = false;
		fixedDir = 0.0f;

		timer = 0.0f;
		moveDuration = 0.0f;
		moveTimer = 0.0f;
		distPlayerSq = 0.0f;
		waitTime = 0.0f;
		cooldownMoveDirection = 0.0f;
		cooldownWaitTimer = 0.0f;
		cooldownMoveTimer = 0.0f;
		cooldownOrbitRadius = 0.0f;
		cooldownProbability = 0.5f;
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

// 前方宣言
class Player;
class BehaviorTree;
class BehaviorNode;

class Enemy : public GameObject<SkinnedMeshModelInstance>
{
public:

	friend class BehaviorNode;

	Enemy(EnemyType enemyType, Transform trans);
	~Enemy();

	inline const EnemyAttributes& GetEnemyAttribute(void) { return enemyAttr; }
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void);

	void SetPlayer(const Player* player) { this->player = player; }

	void ReduceHP(float amount) { enemyAttr.HP -= amount; }

	void SetRandomMove(bool random) { enemyAttr.randomMove = random; }

	void InitHPGauge(void);

	void ChasePlayer(void);
	void AttackPlayer(void);
	void Patrol(void);
	void CooldownMove(void);
	void CooldownWait(void);
	bool DetectPlayer(void);
	bool CheckAvailableToMove(void);

protected:
	EnemyAttributes enemyAttr;
	const Player* player;
	virtual void Initialize(void);
private:
	void SetNewPosTarget(void);
	void StartWaiting(void);

	bool UpdateAliveState(void);

	HRESULT MakeVertexHPGauge(float w, float h);
	void DrawHPGauge(void);

	BehaviorTree*				behaviorTree;  // 敵AIの行動ツリー

	// テクスチャ情報
	ID3D11ShaderResourceView*	HPGaugeTex = nullptr;
	ID3D11ShaderResourceView*	HPGaugeCoverTex = nullptr;

	// 頂点バッファ
	ID3D11Buffer*				HPGaugeVertexBuffer = nullptr;
	ID3D11Buffer*				HPGaugeCoverVertexBuffer = nullptr;
	UISprite					HPGauge;

	//void UpdateEditorSelect(int sx, int sy);
};