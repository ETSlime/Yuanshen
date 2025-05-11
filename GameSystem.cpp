//=============================================================================
//
// GameSystem���� [GameSystem.cpp]
// Author : 
//
//=============================================================================
#include "GameSystem.h"
#include "input.h"
#include "PauseModal.h"
#include "Debugproc.h"
#include "EffectSystem.h"

void GameSystem::Init(void)
{
    m_effectSystem.Initialize(m_renderer.GetDevice(), m_renderer.GetDeviceContext()); // �G�t�F�N�g�V�X�e���̏�����

    m_player = new Player(); // �v���C���[�̏�����

    m_ground = new Ground(); // �n�ʂ̏�����

    m_skybox = new Skybox(); // Skybox�̏�����

    m_uiManager.Init(); // UIManager�̏�����

    m_enemyManager.Init(m_player); // �G�l�~�[�̏�����

    m_cursorManager.Init(); // �J�[�\���}�l�[�W���̏�����
}

void GameSystem::Uninit(void)
{
    SAFE_DELETE(m_player); // �v���C���[�̉��

    SAFE_DELETE(m_ground); // �n�ʂ̉��

    SAFE_DELETE(m_skybox); // Skybox�̉��

    m_uiManager.Uninit(); // UIManager�̉��

    m_effectSystem.Shutdown();
}

void GameSystem::Update(void)
{
    HandleInput();// ���͏���

    bool isPaused = IsPaused(); // �Q�[�����ꎞ��~�����ǂ������m�F

    // �Q�[�����ꎞ��~���̏ꍇ�A���[�_���̍X�V�������s��
    if (!isPaused)
    {
        // ���[�h�ɂ���ď����𕪂���
        switch (m_mode)
        {
        case GameMode::TITLE:
            UpdateTitle(); // �^�C�g����ʂ̍X�V����
            break;
        case GameMode::GAME:
            UpdateGame(); // �Q�[���̍X�V����
            break;
        case GameMode::RESULT:
            UpdateResult(); // ���U���g��ʂ̍X�V����
            break;
        }
    }

    m_uiManager.Update();
}

void GameSystem::Draw(void)
{
    // ���C���p�X�̃r���[�|�[�g��ݒ�
    m_renderer.SetMainPassViewport();

    // ���[�h�ɂ���ď����𕪂���
    switch (m_mode)
    {
    case GameMode::TITLE:
        DrawTitle(); // �^�C�g����ʂ̕`�揈��
        break;
    case GameMode::GAME:
        DrawGame(); // �Q�[���̕`�揈��
        break;
    case GameMode::RESULT:
        DrawResult(); // ���U���g��ʂ̕`�揈��
        break;
    default:
        break;
    }

    // UI�̕`��
    //m_lightManager.SetLightEnable(FALSE); // ���C�e�B���O�𖳌�
    m_renderer.SetCullingMode(CULL_MODE_NONE); // �J�����O���[�h�𖳌���
    m_renderer.SetUIInputLayout(); // ���f���̓��̓��C�A�E�g��ݒ�
    m_renderer.SetRenderUI(); // UI�̕`���ݒ�
    // �[�x�e�X�g�𖳌���
    m_renderer.SetDepthMode(DepthMode::Disable);
    m_uiManager.Draw();
    // ���C�e�B���O��L����
    //m_lightManager.SetLightEnable(TRUE);
}

void GameSystem::RenderShadowPass(void)
{
    if (m_mode != GameMode::GAME)
        return;

    // �V���h�E�}�b�v�̃����_�����O
    const auto& lightList = m_lightManager.GetLightList();
    const auto& sceneObjects = m_scene.GetAllRenderableObjects();

    int lightIdx = 0;
    for (const auto& light : lightList)
    {
        // ���C�g�̗L�������m�F
        if (!light->GetLightData().Enable || lightIdx >= LIGHT_MAX) continue;

        // �f�B���N�V���i�����C�g�̏ꍇ
        if (light->GetType() == LIGHT_TYPE::DIRECTIONAL)
        {
            m_shadowMapRenderer.RenderCSMForLight(static_cast<DirectionalLight*>(light), lightIdx, sceneObjects);
        }

        lightIdx++;
    }
}

void GameSystem::PauseGame(void)
{
    m_isPaused = true;    
    // �J�[�\���ʒu�𕜌����A���̌�UI��p��Ԃɐ؂�ւ���
    m_cursorManager.RestoreCursorAndEnterUIExclusive(); 
    // �|�[�Y���[�_����\��
    m_uiManager.SetModalIfNotExist<PauseModal>(); 

}

void GameSystem::ResumeGame(void)
{
    m_isPaused = false;
    m_cursorManager.EnterUIHidden(); // UI��\�����[�h�ɓ���
    m_cursorManager.RememberCursorPosition(); // �J�[�\���ʒu��ۑ�
    SetMouseRecentered(true); // �J�����p�F���̃t���[���Ń}�E�X�ʒu���Z�b�g��v��
}

void GameSystem::EnterDialogue(void)
{
    m_inDialogue = true;
}

void GameSystem::ExitDialogue(void)
{
    m_inDialogue = false;
}

void GameSystem::ToggleMiniMap()
{
    m_showMiniMap = !m_showMiniMap;
}

void GameSystem::SetMode(GameMode mode)
{
    m_mode = mode;
}

void GameSystem::HandleInput(void)
{
    HandlePauseInput();
}

void GameSystem::HandlePauseInput(void)
{
    if (GetKeyboardTrigger(KEY_PAUSE) && m_mode == GameMode::GAME && !m_inDialogue)
    {
        // �Q�[�����ꎞ��~�����ǂ������m�F
        if (IsPaused())
        {
            // �Q�[�����ĊJ
            ResumeGame();
            m_uiManager.ClearModal();
        }
        else
        {
            // �Q�[�����ꎞ��~
            PauseGame();
        }
    }
}

// �e���[�h�̍X�V
void GameSystem::UpdateTitle(void)
{
    // �^�C�g����ʂ̃��W�b�N
}

void GameSystem::UpdateGame(void)
{
    if (m_isPaused)
        return;

    m_isAltDown = GetKeyboardPress(DIK_LALT) || GetKeyboardPress(DIK_RALT);

    if (m_isAltDown)
        m_cursorManager.OnEnterVisibleTemp();
    else
        m_cursorManager.OnExitVisibleTemp();

    m_player->Update(); // �v���C���[�̍X�V����
    m_ground->Update(); // �n�ʂ̍X�V����
    m_skybox->Update(); // Skybox�̍X�V����
    m_enemyManager.Update(); // �G�l�~�[�̍X�V����
    m_effectSystem.Update(); // �G�t�F�N�g�̍X�V����
}

void GameSystem::UpdateResult(void)
{
    // ���U���g��ʂ̃��W�b�N
}

// �e���[�h�̕`��
void GameSystem::DrawTitle(void)
{
    // �^�C�g����ʂ̕`��
}

void GameSystem::DrawGame(void)
{
    // �X�J�C�{�b�N�X�̕`��
    //m_skybox->Draw(XMLoadFloat4x4(&m_camera.GetViewMatrix()), XMLoadFloat4x4(&m_camera.GetProjMatrix()));

    m_renderer.SetDepthMode(DepthMode::Enable); // �[�x�e�X�g��L����
    m_renderer.SetCullingMode(CULL_MODE_BACK); // �J�����O���[�h��L����

    m_renderer.SetStaticModelInputLayout(); // ���f���̓��̓��C�A�E�g��ݒ�
    m_renderer.SetRenderObject(); // ���f���̕`���ݒ�

    // ���C�e�B���O�𖳌�
    m_lightManager.SetLightEnable(FALSE);
    m_renderer.SetRenderLayer(RenderLayer::LAYER_1); // UI�̕`�惌�C���[��ݒ�
    m_enemyManager.DrawUI(EnemyUIType::HPGauge); // HP�Q�[�W�̕`��
    m_renderer.SetRenderLayer(RenderLayer::DEFAULT);
    m_enemyManager.DrawUI(EnemyUIType::HPGaugeCover); // HP�Q�[�W�J�o�[�̕`��
    // ���C�e�B���O��L����
    m_lightManager.SetLightEnable(TRUE);

    m_ground->Draw(); // �n�ʂ̕`��

    
    m_renderer.SetSkinnedMeshInputLayout(); // �X�L�j���O���b�V���̓��̓��C�A�E�g��ݒ�
    m_renderer.SetRenderSkinnedMeshModel(); // �X�L�j���O���b�V�����f���̕`���ݒ�
    m_player->Draw(); // �v���C���[�̕`��
    m_enemyManager.Draw(); // �G�l�~�[�̕`��
    m_ground->Draw(); // �n�ʂ̕`��

    m_renderer.SetRenderInstance(); // �C���X�^���X�̕`���ݒ�
    m_ground->Draw(); // �n�ʂ̕`��

    m_renderer.SetVFXInputLayout(); // VFX�̓��̓��C�A�E�g��ݒ�
    m_renderer.SetRenderVFX(); // VFX�̕`���ݒ�
    m_player->DrawEffect(); // �v���C���[�̃G�t�F�N�g�`��

    m_renderer.SetDepthMode(DepthMode::Particle); // �p�[�e�B�N���̐[�x�ݒ�
    m_effectSystem.Draw(m_camera.GetViewProjMtx()); // �G�t�F�N�g�̕`��
    m_renderer.SetBlendState(BLEND_MODE_ALPHABLEND); // �u�����h�X�e�[�g��ݒ�

    //m_renderer.SetCullingMode(CULL_MODE_BACK);

    //SetOffScreenRender();
//DrawScene();

//SetLightEnable(FALSE);
//DrawOffScreenRender();
//SetLightEnable(FALSE);
}

void GameSystem::DrawResult(void)
{
    // ���U���g��ʂ̕`��
}