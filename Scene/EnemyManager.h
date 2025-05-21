//=============================================================================
//
// �G�l�~�[�����̂܂Ƃߖ������ [EnemyManager.h]
// Author : 
// �G�L�����̐����E�X�V�E�`��EUI�\���܂ł��[��Ԃ����b���Ă����Ǘ��N���X�ł����I
// �_�u�������N���X�g�œG����񂽂����������萮�񂳂��Ă邨�o����^�C�v�����c�H
//
//=============================================================================
#pragma once

#include "Scene/Enemy.h"
#include "Utility/DoubleLinkedList.h"
#include "Utility/SingletonBase.h"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
class EnemyManager : public SingletonBase<EnemyManager>, public IDebugUI
{
public:
	EnemyManager();

	void Init(const Player* player = nullptr);
	void Draw(void);
	void Update(void);
	void DrawUI(EnemyUIType type);

	// �G�L�����̐���
	void SpawnEnemy(EnemyType enemType, Transform trans, EnemyState initState); 

	const DoubleLinkedList<Enemy*>* GetEnemy() { return &m_enemyList; }

private:

	virtual void RenderImGui(void) override;
	virtual const char* GetPanelName(void) const override { return "Enemy Manager"; };

	DoubleLinkedList<Enemy*> m_enemyList;
	Renderer& m_renderer = Renderer::get_instance();
	const Player* m_player;
	bool m_drawBoundingBox = true;
};