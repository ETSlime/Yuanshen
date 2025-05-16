//=============================================================================
//
// �G�l�~�[�����̂܂Ƃߖ������ [EnemyManager.h]
// Author : 
// �G�L�����̐����E�X�V�E�`��EUI�\���܂ł��[��Ԃ����b���Ă����Ǘ��N���X�ł����I
// �_�u�������N���X�g�œG����񂽂����������萮�񂳂��Ă邨�o����^�C�v�����c�H
//
//=============================================================================
#pragma once

#include "Enemy.h"
#include "DoubleLinkedList.h"
#include "SingletonBase.h"

//*****************************************************************************
// �v���g�^�C�v�錾
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