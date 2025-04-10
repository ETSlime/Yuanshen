//=============================================================================
//
// GameSystemèàóù [GameSystem.cpp]
// Author : 
//
//=============================================================================
#include "GameSystem.h"
#include "UIManager.h"

void GameSystem::Init(void)
{
	UIManager::get_instance().Init();
}
