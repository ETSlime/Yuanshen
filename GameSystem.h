//=============================================================================
//
// GameSystem���� [GameSystem.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"

// �Q�[�����[�h�̗�
enum class GameMode
{
    TITLE,
    GAME,
    RESULT
};

// �Q�[���S�̂̏�Ԃ��Ǘ�����N���X
class GameSystem : public SingletonBase<GameSystem>
{
public:

    // �������ƍX�V
    void Init(void);
    void Update(void);
    void Draw(void);

    // �Q�[���̈ꎞ��~/�ĊJ
    void PauseGame(void);
    void ResumeGame(void);
    bool IsPaused(void) const { return m_isPaused; }

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

private:
    // �v���C�x�[�g�R���X�g���N�^
    GameSystem() = default;
    GameSystem(const GameSystem&) = delete;
    GameSystem& operator=(const GameSystem&) = delete;

    // �������
    bool m_isPaused = false;
    bool m_inDialogue = false;
    bool m_showMiniMap = true;
    GameMode m_mode = GameMode::TITLE;

    // ���[�h���Ƃ̍X�V�ƕ`��֐�
    void UpdateTitle(void);
    void UpdateGame(void);
    void UpdateResult(void);

    void DrawTitle(void);
    void DrawGame(void);
    void DrawResult(void);
};