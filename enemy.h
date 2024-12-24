//=============================================================================
//
// �G�l�~�[���f������ [enemy.h]
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
enum
{
	ENEMY_IDLE,
	ENEMY_WALK,
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct ENEMY
{

	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)


	bool				use;
	bool				load;
	Model*				pModel;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F


	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
	float				shadowSize;



};

struct EnemyAttribute
{
	float				HP;
	float				maxHP;
	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
};

struct EnemyInstance : public ModelInstance
{
	EnemyAttribute		attribute;

	INTERPOLATION_DATA*	moveTbl;
	BOOL				jumpUp;
	int					jumpCnt;
	float				jumpYMax;
	int					state;

	XMFLOAT3	move;			// �ړ����x
	float		dir;
	float		targetDir;

	float		time;			// ���`��ԗp
	int			tblMax;			// ���̃e�[�u���̃f�[�^��
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

class Enemy : public GameObject<EnemyInstance>
{
public:
	Enemy(char* enemyPath, Transform trans);
	inline EnemyAttribute* GetEnemyAttribute(void) { return &instance.attribute; }
	void Update(void) override;
	void Draw(void) override;
	inline INTERPOLATION_DATA* GetMoveTbl() { return instance.moveTbl; }
	void SetMoveTbl(MoveTable moveTbl);
private:
	void UpdateEditorSelect(int sx, int sy);
	void PlayEnemyWalkAnim(void);
};

class EnemyManager : public SingletonBase<EnemyManager>
{
public:
	EnemyManager();
	void Init();
	void SpawnEnemy(char* enemyPath, Transform trans, MoveTable moveTbl);
	void Draw(void);
	void Update(void);
	const DoubleLinkedList<Enemy*>* GetEnemy() { return &enemyList; }
private:
	void InitializeMoveTbl();

	DoubleLinkedList<Enemy*> enemyList;
	SimpleArray<MoveTable> moveTbls;
};