//=============================================================================
//
// GameSystem処理 [GameSystem.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"

// ゲームモードの列挙
enum class GameMode
{
    TITLE,
    GAME,
    RESULT
};

// ゲーム全体の状態を管理するクラス
class GameSystem : public SingletonBase<GameSystem>
{
public:

    // 初期化と更新
    void Init(void);
    void Update(void);
    void Draw(void);

    // ゲームの一時停止/再開
    void PauseGame(void);
    void ResumeGame(void);
    bool IsPaused(void) const { return m_isPaused; }

    // 会話モード切替
    void EnterDialogue(void);
    void ExitDialogue(void);
    bool IsInDialogue(void) const { return m_inDialogue; }

    // ミニマップ表示切替
    void ToggleMiniMap(void);
    bool IsMiniMapVisible(void) const { return m_showMiniMap; }

    // モード切り替え
    void SetMode(GameMode mode);
    GameMode GetMode() const { return m_mode; }

private:
    // プライベートコンストラクタ
    GameSystem() = default;
    GameSystem(const GameSystem&) = delete;
    GameSystem& operator=(const GameSystem&) = delete;

    // 内部状態
    bool m_isPaused = false;
    bool m_inDialogue = false;
    bool m_showMiniMap = true;
    GameMode m_mode = GameMode::TITLE;

    // モードごとの更新と描画関数
    void UpdateTitle(void);
    void UpdateGame(void);
    void UpdateResult(void);

    void DrawTitle(void);
    void DrawGame(void);
    void DrawResult(void);
};