//=============================================================================
//
// EnemyManager処理 [EnemyManager.h]
// Author : 
//
//=============================================================================
#pragma once

#include "Enemy.h"
#include "DoubleLinkedList.h"
#include "SingletonBase.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
class EnemyManager : public SingletonBase<EnemyManager>
{
public:
	EnemyManager();
	void Init(const Player* player = nullptr);
	void SpawnEnemy(EnemyType enemType, Transform trans, EnemyState initState);
	void Draw(void);
	void DrawUI(void);
	void Update(void);
	const DoubleLinkedList<Enemy*>* GetEnemy() { return &enemyList; }
	Renderer& renderer = Renderer::get_instance();
private:

	DoubleLinkedList<Enemy*> enemyList;
	SimpleArray<MoveTable> moveTbls;
	const Player* player;
};