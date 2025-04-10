//=============================================================================
//
// PauseModal処理 [PauseModal.h]
// Author : 
//
//=============================================================================
#pragma once
#include "UIElement.h"
#include "UIManager.h"

class PauseModal : public UIElement
{
public:
    PauseModal() 
    {
        m_position = { 640, 360 };
        m_size = { 400, 200 };
    }

    void Update(void) override {} // 無処理
    void Draw(void) override {}
    bool NeedsOverlay(void) const override { return true; } // オーバーレイが必要
    void DrawCustomOverlay(void) const override
    {
        UIManager::get_instance().DrawOverlay(0, 0, 1280, 720, { 1.0f, 0.7f, 0.9f, 0.4f });
    }
};

