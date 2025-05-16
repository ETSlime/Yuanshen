//=============================================================================
//
// エネミーたちのまとめ役ちゃん [EnemyManager.h]
// Author : 
// 敵キャラの生成・更新・描画・UI表示までぜーんぶお世話してくれる管理クラスですっ！
// ダブルリンクリストで敵ちゃんたちをしっかり整列させてるお姉さんタイプかも…？
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
	void DrawUI(EnemyUIType type);
	void Update(void);
	const DoubleLinkedList<Enemy*>* GetEnemy() { return &m_enemyList; }
	Renderer& m_renderer = Renderer::get_instance();
private:

	DoubleLinkedList<Enemy*> m_enemyList;
	const Player* m_player;
};