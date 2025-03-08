//=============================================================================
//
// �G�l�~�[���f������ [Enemy.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
#include "DoubleLinkedList.h"
#include "SingletonBase.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_ENEMY		(5)					// �G�l�~�[�̐�
#define JUMP_CNT_MAX	(120)
#define	ENEMY_SIZE		(65.0f)				// �����蔻��̑傫��
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
// �\���̒�`
//*****************************************************************************
//struct ENEMY
//{
//
//	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
//	XMFLOAT3			pos;				// ���f���̈ʒu
//	XMFLOAT3			rot;				// ���f���̌���(��])
//	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)
//
//
//	bool				use;
//	bool				load;
//	Model*				pModel;				// ���f�����
//	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F
//
//
//	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
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

	float				viewAngle;     // ����p (��: 60�� = 60 * (PI / 180))
	float				viewDistance;  // ���F�͈�
	float				chaseRange;    // �ǐՔ͈�
	float				attackRange;   // �U���͈�
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
	XMFLOAT3	move;			// �ړ����x
	float		time;			// ���`��ԗp
	int			tblMax;			// ���̃e�[�u���̃f�[�^��

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