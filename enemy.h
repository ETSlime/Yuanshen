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
#define ENEMY_HIT_WINDOW			(60)

#define HILICHURL_SIZE	XMFLOAT3(0.9f, 0.9f, 0.9f)
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
	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��

	INTERPOLATION_DATA* moveTbl;

	XMFLOAT3	move;			// �ړ����x
	float		dir;
	float		targetDir;

	float		time;			// ���`��ԗp
	int			tblMax;			// ���̃e�[�u���̃f�[�^��

	EnemyType	enemyType;
};

//struct EnemyInstance : public ModelInstance
//{
//	EnemyAttribute		attribute;
//
//	INTERPOLATION_DATA*	moveTbl;
//	BOOL				jumpUp;
//	int					jumpCnt;
//	float				jumpYMax;
//	int					state;
//
//	XMFLOAT3	move;			// �ړ����x
//	float		dir;
//	float		targetDir;
//
//	float		time;			// ���`��ԗp
//	int			tblMax;			// ���̃e�[�u���̃f�[�^��
//};

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

class Enemy : public GameObject<SkinnedMeshModelInstance>
{
public:
	Enemy(EnemyType enemyType, Transform trans);
	inline EnemyAttribute* GetEnemyAttribute(void) { return &attribute; }
	void Update(void) override;
	void Draw(void) override;
	inline INTERPOLATION_DATA* GetMoveTbl() { return attribute.moveTbl; }
	void SetMoveTbl(const MoveTable* moveTbl);

private:

	EnemyAttribute attribute;

	//void UpdateEditorSelect(int sx, int sy);
};

class EnemyManager : public SingletonBase<EnemyManager>
{
public:
	EnemyManager();
	void Init();
	void SpawnEnemy(EnemyType enemType, Transform trans, EnemyState initState, MoveTable* moveTbl = nullptr);
	void Draw(void);
	void Update(void);
	const DoubleLinkedList<Enemy*>* GetEnemy() { return &enemyList; }
	Renderer& renderer = Renderer::get_instance();
private:
	void InitializeMoveTbl();

	DoubleLinkedList<Enemy*> enemyList;
	SimpleArray<MoveTable> moveTbls;
};