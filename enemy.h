//=============================================================================
//
// �G�l�~�[���f������ [Enemy.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "Timer.h"
//*****************************************************************************
// �}�N����`
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
// �\���̒�`
//*****************************************************************************

struct EnemyAttributes
{
	float				HP;
	float				maxHP;

	float				timer;

	float				moveDuration;	// �ړ����鎞��
	float				moveTimer;		// �ړ����Ԃ̃J�E���^�[
	float				waitTime;		// �ҋ@����
	float				distPlayerSq;

	bool				fixedDirMove;
	float				fixedDir;

	bool				isWaiting;
	bool				isSurprised;
	bool				isChasingPlayer;
	bool				isSitting;
	bool				isInCooldown;

	float				viewAngle;     // ����p (��: 60�� = 60 * (PI / 180))
	float				viewDistance;  // ���F�͈�
	float				chaseRange;    // �ǐՔ͈�
	float				attackRange;   // �U���͈�
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
	XMFLOAT3	pos;			// �ʒu
	XMFLOAT3	scl;			// �X�P�[��
	MATERIAL	material;		// �}�e���A��
	float		fWidth;			// ��
	float		fHeight;		// ����
	BOOL		bUse;			// �g�p���Ă��邩�ǂ���

};

struct MoveTable
{
	INTERPOLATION_DATA* moveTblAddr;
	int size;
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

// �O���錾
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

	BehaviorTree*				behaviorTree;  // �GAI�̍s���c���[

	// �e�N�X�`�����
	ID3D11ShaderResourceView*	HPGaugeTex = nullptr;
	ID3D11ShaderResourceView*	HPGaugeCoverTex = nullptr;

	// ���_�o�b�t�@
	ID3D11Buffer*				HPGaugeVertexBuffer = nullptr;
	ID3D11Buffer*				HPGaugeCoverVertexBuffer = nullptr;
	UISprite					HPGauge;

	//void UpdateEditorSelect(int sx, int sy);
};