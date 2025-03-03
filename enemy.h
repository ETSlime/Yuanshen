//=============================================================================
//
// エネミーモデル処理 [enemy.h]
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
	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ

	INTERPOLATION_DATA* moveTbl;

	XMFLOAT3	move;			// 移動速度
	float		dir;
	float		targetDir;

	float		time;			// 線形補間用
	int			tblMax;			// そのテーブルのデータ数

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
//	XMFLOAT3	move;			// 移動速度
//	float		dir;
//	float		targetDir;
//
//	float		time;			// 線形補間用
//	int			tblMax;			// そのテーブルのデータ数
//};

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