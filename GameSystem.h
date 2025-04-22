//=============================================================================
//
// GameSystem処理 [GameSystem.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"
#include "UIManager.h"
#include "Renderer.h"
#include "Player.h"
#include "EnemyManager.h"
#include "Ground.h"
#include "Skybox.h"
#include "Camera.h"
#include "ShadowMapRenderer.h"
#include "CursorManager.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
// ゲームモードの列挙
enum class GameMode : int		// ゲームモード
{
    TITLE = 0,			        // タイトル画面
    TUTORIAL,			        // ゲーム説明画面
    GAME,				        // ゲーム画面
    RESULT,				        // リザルト画面
    MAX
};

// ゲーム全体の状態を管理するクラス
class GameSystem : public SingletonBase<GameSystem>
{
public:

    // 初期化と更新
    void Init(void);
    void Uninit(void);
    void Update(void);
    void Draw(void);
    void RenderShadowPass(void);

    // ゲームの一時停止/再開
    void PauseGame(void);
    void ResumeGame(void);
    bool IsPaused(void) const { return m_isPaused; }
    bool IsAltDown(void) const { return m_isAltDown; } // Altキーが押されているかどうか

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

protected:
    // プライベートコンストラクタ
    GameSystem()
        : SingletonBase()
        , m_renderer(Renderer::get_instance())
        , m_lightManager(LightManager::get_instance())
        , m_uiManager(UIManager::get_instance())
        , m_enemyManager(EnemyManager::get_instance())
        , m_camera(Camera::get_instance())
        , m_scene(Scene::get_instance())
        , m_shadowMapRenderer(ShadowMapRenderer::get_instance())
        , m_cursorManager(CursorManager::get_instance()) {}

    // この行が絶対に必要！！
    // SingletonBase<GameSystem> が GameSystem の protected コンストラクタにアクセスできるようにするための特別な許可
    // テンプレートクラスは「自動的にフレンド」にはならないため、明示的に friend 宣言が必要
    friend class SingletonBase<GameSystem>;


private:
    // 内部状態
    bool m_isPaused = false;
    bool m_inDialogue = false;
    bool m_showMiniMap = true;
    bool m_isAltDown = false; // Altキーが押されているかどうか

    void HandleInput(void);
    void HandlePauseInput(void);

    GameMode m_mode = GameMode::GAME;

    Player* m_player = nullptr;
    Ground* m_ground = nullptr;
    Skybox* m_skybox = nullptr;

    UIManager& m_uiManager;
    EnemyManager& m_enemyManager;
    Renderer& m_renderer;
    LightManager& m_lightManager;
    Camera& m_camera;
    Scene& m_scene;
    ShadowMapRenderer& m_shadowMapRenderer;
    CursorManager& m_cursorManager;

    // モードごとの更新と描画関数
    void UpdateTitle(void);
    void UpdateGame(void);
    void UpdateResult(void);

    void DrawTitle(void);
    void DrawGame(void);
    void DrawResult(void);
};