//=============================================================================
//
// EnemyManager処理 [EnemyManager.cpp]
// Author : 
//
//=============================================================================
#include "EnemyManager.h"
#include "Hilichurl.h"

EnemyManager::EnemyManager()
{
	player = nullptr;
}

void EnemyManager::Init(const Player* player)
{
	this->player = player;
	Transform trans;
	trans.pos = XMFLOAT3(12937.0f, -2384.0f, -19485.0f);
	trans.scl = HILICHURL_SIZE;
	//for (int i = 0; i < 25; i++)
	//{
	//	float radius = 20000.f;
	//	float r = GetRandFloat(2000.0f, radius);
	//	float angle = GetRandFloat(0.0f, 2.0f * 3.14159265359f);
	//	float x = trans.pos.x + r * cos(angle);
	//	float z = trans.pos.z + r * sin(angle);
	//	Transform t = trans;
	//	t.pos.x = x;
	//	t.pos.z = z;

	//	SpawnEnemy(EnemyType::Hilichurl, t, EnemyState::IDLE);
	//}


	SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	//trans.pos = XMFLOAT3(12759.5f, -2384.0f, -19177.56f);
	//SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	//trans.pos = XMFLOAT3(12404.5f, -2384.0f, -19177.56f);
	//SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	//trans.pos = XMFLOAT3(12227.0f, -2384.0f, -19485.0f);
	//SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	//trans.pos = XMFLOAT3(12404.5f, -2384.0f, -19792.43f);
	//SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);

	//trans.pos = XMFLOAT3(12759.5f, -2384.0f, -19792.43f);
	//SpawnEnemy(EnemyType::Hilichurl, trans, EnemyState::HILI_DANCE);
}



void EnemyManager::SpawnEnemy(EnemyType enemyType, Transform trans, EnemyState initState)
{
	Enemy* enemy;
	switch (enemyType)
	{
	case EnemyType::Hilichurl:
		enemy = new Hilichurl(trans, initState);
		break;
	default:
		return;
	}

	if (initState == EnemyState::IDLE)
		enemy->SetRandomMove(true);
	else
		enemy->SetRandomMove(false);

	enemy->SetPlayer(player);
	enemyList.push_back(enemy);
}

void EnemyManager::Draw(void)
{
	// カリング無効
	renderer.SetCullingMode(CULL_MODE_NONE);

	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		// モデル描画
		if (cur->data->GetInstance()->renderProgress.progress < 1.0f)
			renderer.SetRenderProgress(cur->data->GetInstance()->renderProgress);

		if (!cur->data->GetEnemyAttribute().isDead)
			cur->data->Draw();

		if (cur->data->GetInstance()->renderProgress.progress < 1.0f)
		{
			RenderProgressBuffer defaultRenderProgress;
			defaultRenderProgress.isRandomFade = false;
			defaultRenderProgress.progress = 1.0f;
			renderer.SetRenderProgress(defaultRenderProgress);
		}

		cur = cur->next;
	}

	// カリング設定を戻す
	renderer.SetCullingMode(CULL_MODE_BACK);
}

void EnemyManager::DrawUI(void)
{
	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		// モデル描画
		if (cur->data->GetInstance()->renderProgress.progress < 1.0f)
			renderer.SetRenderProgress(cur->data->GetInstance()->renderProgress);

		if (!cur->data->GetEnemyAttribute().isDead)
			cur->data->DrawUI();

		if (cur->data->GetInstance()->renderProgress.progress < 1.0f)
		{
			RenderProgressBuffer defaultRenderProgress;
			defaultRenderProgress.isRandomFade = false;
			defaultRenderProgress.progress = 1.0f;
			renderer.SetRenderProgress(defaultRenderProgress);
		}

		cur = cur->next;
	}
}

void EnemyManager::Update(void)
{
	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		cur->data->Update();
		if (cur->data->GetUse() == FALSE)
		{
			Node<Enemy*>* toDelete = cur;
			cur = cur->next;
			delete toDelete->data;
			enemyList.remove(toDelete);
		}
		else
			cur = cur->next;
	}
}