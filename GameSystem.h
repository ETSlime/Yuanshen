//=============================================================================
//
// GameSystem���� [GameSystem.h]
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
// �}�N����`
//*****************************************************************************
// �Q�[�����[�h�̗�
enum class GameMode : int		// �Q�[�����[�h
{
    TITLE = 0,			        // �^�C�g�����
    TUTORIAL,			        // �Q�[���������
    GAME,				        // �Q�[�����
    RESULT,				        // ���U���g���
    MAX
};

// �Q�[���S�̂̏�Ԃ��Ǘ�����N���X
class GameSystem : public SingletonBase<GameSystem>
{
public:

    // �������ƍX�V
    void Init(void);
    void Uninit(void);
    void Update(void);
    void Draw(void);
    void RenderShadowPass(void);

    // �Q�[���̈ꎞ��~/�ĊJ
    void PauseGame(void);
    void ResumeGame(void);
    bool IsPaused(void) const { return m_isPaused; }
    bool IsAltDown(void) const { return m_isAltDown; } // Alt�L�[��������Ă��邩�ǂ���

    // ��b���[�h�ؑ�
    void EnterDialogue(void);
    void ExitDialogue(void);
    bool IsInDialogue(void) const { return m_inDialogue; }

    // �~�j�}�b�v�\���ؑ�
    void ToggleMiniMap(void);
    bool IsMiniMapVisible(void) const { return m_showMiniMap; }

    // ���[�h�؂�ւ�
    void SetMode(GameMode mode);
    GameMode GetMode() const { return m_mode; }

protected:
    // �v���C�x�[�g�R���X�g���N�^
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

    // ���̍s����΂ɕK�v�I�I
    // SingletonBase<GameSystem> �� GameSystem �� protected �R���X�g���N�^�ɃA�N�Z�X�ł���悤�ɂ��邽�߂̓��ʂȋ���
    // �e���v���[�g�N���X�́u�����I�Ƀt�����h�v�ɂ͂Ȃ�Ȃ����߁A�����I�� friend �錾���K�v
    friend class SingletonBase<GameSystem>;


private:
    // �������
    bool m_isPaused = false;
    bool m_inDialogue = false;
    bool m_showMiniMap = true;
    bool m_isAltDown = false; // Alt�L�[��������Ă��邩�ǂ���

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

    // ���[�h���Ƃ̍X�V�ƕ`��֐�
    void UpdateTitle(void);
    void UpdateGame(void);
    void UpdateResult(void);

    void DrawTitle(void);
    void DrawGame(void);
    void DrawResult(void);
};