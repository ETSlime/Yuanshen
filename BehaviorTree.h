#pragma once
//=============================================================================
//
// BehaviorTree���� [BehaviorTree.h]
// Author : 
//
//=============================================================================
#include "SimpleArray.h"
#include "Enemy.h"

class BehaviorNode 
{
public:
    virtual bool Execute(void) = 0;

protected:
    EnemyAttributes& GetEnemyAttributes(Enemy* enemy)
    {
        return enemy->enemyAttr;
    }
};

// ======= �I���m�[�h�iSelector�j =======
class SelectorNode : public BehaviorNode 
{
public:
    SimpleArray<BehaviorNode*> children;

    void AddChild(BehaviorNode* child) 
    {
        children.push_back(child);
    }

    bool Execute(void) override
    {
        for (UINT i = 0; i < children.getSize(); i++) 
        {
            if (children[i]->Execute())
            {
                return true;  // �ǂꂩ����������A�����I������
            }
        }
        return false;  // �S�Ă̎q�����s�����ꍇ�A���s��Ԃ�
    }
};

// ======= �����m�[�h�iSequence�j =======
class SequenceNode : public BehaviorNode
{
public:
    SimpleArray<BehaviorNode*> children;

    void AddChild(BehaviorNode* child) 
    {
        children.push_back(child);
    }

    bool Execute(void) override 
    {
        for (UINT i = 0; i < children.getSize(); i++)
        {
            if (!children[i]->Execute()) 
            {
                return false;  // �ǂꂩ���s������A�S�̂����s
            }
        }
        return true;  // �S�Đ����Ȃ�A������Ԃ�
    }
};

// ======= ��p��Ԃ��`�F�b�N����m�[�h =======
class CheckCooldown : public BehaviorNode
{
    Enemy* enemy;
public:
    CheckCooldown(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        return GetEnemyAttributes(enemy).isInCooldown;
    }
};

// ======= �N�[���_�E�����̈ړ��m�[�h =======
class PerformCooldownMovement : public BehaviorNode
{
    Enemy* enemy;
public:
    PerformCooldownMovement(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        bool nextMove = false;

        PrintDebugProc("%d\n", static_cast<int>(GetEnemyAttributes(enemy).cooldownState));

        switch (GetEnemyAttributes(enemy).cooldownState)
        {
        case CooldownState::MOVE:
            GetEnemyAttributes(enemy).cooldownMoveTimer -= enemy->timer.GetScaledDeltaTime();
            if (GetEnemyAttributes(enemy).cooldownMoveTimer <= 0.0f)
            {
                GetEnemyAttributes(enemy).cooldownMoveTimer = 0.0f;
                nextMove = true;
            }
            else
                enemy->CooldownMove();
            break;
        case CooldownState::WAIT:
            GetEnemyAttributes(enemy).cooldownWaitTimer -= enemy->timer.GetScaledDeltaTime();
            if (GetEnemyAttributes(enemy).cooldownWaitTimer <= 0.0f)
            {
                GetEnemyAttributes(enemy).cooldownWaitTimer = 0.0f;
                nextMove = true;
            }
            else
                enemy->CooldownWait();
            break;
        default:
            break;
        }
        
        if (nextMove)
        {
            float randValue = GetRandFloat(0.0f, 1.0f);
            if (randValue < GetEnemyAttributes(enemy).cooldownProbability)
            {
                GetEnemyAttributes(enemy).cooldownState = CooldownState::MOVE;
                GetEnemyAttributes(enemy).cooldownMoveDirection = (GetRand(0, 1) == 0) ? -1.0f : 1.0f;
                GetEnemyAttributes(enemy).cooldownMoveTimer = GetEnemyAttributes(enemy).attackCooldownTimer *
                    GetRandFloat(HILI_MIN_COOLDOWN_MOVE_TIME, HILI_MAX_COOLDOWN_MOVE_TIME);

                GetEnemyAttributes(enemy).cooldownProbability += 0.2f;
            }
            else
            {
                GetEnemyAttributes(enemy).cooldownState = CooldownState::WAIT;
                GetEnemyAttributes(enemy).cooldownWaitTimer = GetEnemyAttributes(enemy).attackCooldownTimer *
                    GetRandFloat(HILI_MIN_COOLDOWN_WAIT_TIME, HILI_MAX_COOLDOWN_WAIT_TIME);

                GetEnemyAttributes(enemy).cooldownProbability -= 0.2f;
            }
        }


        GetEnemyAttributes(enemy).attackCooldownTimer -= enemy->timer.GetScaledDeltaTime();
        if (GetEnemyAttributes(enemy).attackCooldownTimer <= 0.0f)
        {
            GetEnemyAttributes(enemy).isInCooldown = false;
            GetEnemyAttributes(enemy).fixedDirMove = false;
        }
        return true;
    }
};

// ======= �v���C���[�������`�F�b�N����m�[�h =======
class CheckDetectPlayer : public BehaviorNode
{
    Enemy* enemy;
public:
    CheckDetectPlayer(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        return enemy->DetectPlayer();
    }
};

// ======= �G�̏�Ԃ��X�V����m�[�h�i�����A�ǐՁj =======
class UpdateEnemyState : public BehaviorNode
{
    Enemy* enemy;
public:
    UpdateEnemyState(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        if (!GetEnemyAttributes(enemy).isSitting) 
        {

            if (!GetEnemyAttributes(enemy).isChasingPlayer)
            {
                GetEnemyAttributes(enemy).isSurprised = true;
            }
            if (!GetEnemyAttributes(enemy).isSurprised) 
            {
                GetEnemyAttributes(enemy).isChasingPlayer = true;
            }
        }
        return true;
    }
};

// ======= �ǐՏ�Ԃ��`�F�b�N����m�[�h =======
class CheckChasingPlayer : public BehaviorNode
{
    Enemy* enemy;
public:
    CheckChasingPlayer(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        return GetEnemyAttributes(enemy).isChasingPlayer || GetEnemyAttributes(enemy).isSurprised;
    }
};

// ======= �v���C���[��ǐՂ���m�[�h =======
class MoveToPlayer : public BehaviorNode
{
    Enemy* enemy;
public:
    MoveToPlayer(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        enemy->ChasePlayer();
        return true;
    }
};

// ======= �����_���ړ����`�F�b�N����m�[�h =======
class CheckRandomMove : public BehaviorNode
{
    Enemy* enemy;
public:
    CheckRandomMove(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        return GetEnemyAttributes(enemy).randomMove;
    }
};

// ======= ����E�ҋ@�m�[�h =======
class PatrolMovement : public BehaviorNode
{
    Enemy* enemy;
public:
    PatrolMovement(Enemy* e) : enemy(e) {}

    bool Execute(void) override 
    {
        enemy->Patrol();
        return true;
    }
};


class BehaviorTree 
{
protected:
    SelectorNode root;  // ���[�g�m�[�h�i�I���m�[�h�j

public:
    BehaviorTree(Enemy* enemy) 
    {
        InitializeTree(enemy);
    }

    virtual void InitializeTree(Enemy* enemy) 
    {
        // ===== ��p�V�[�P���X =====
        SequenceNode* cooldownSequence = new SequenceNode();
        cooldownSequence->AddChild(new CheckCooldown(enemy));
        cooldownSequence->AddChild(new PerformCooldownMovement(enemy));

        // ===== �v���C���[�����V�[�P���X =====
        SequenceNode* detectAndChasePlayerSequence = new SequenceNode();
        detectAndChasePlayerSequence->AddChild(new CheckDetectPlayer(enemy));
        detectAndChasePlayerSequence->AddChild(new UpdateEnemyState(enemy));
        detectAndChasePlayerSequence->AddChild(new CheckChasingPlayer(enemy));
        detectAndChasePlayerSequence->AddChild(new MoveToPlayer(enemy));

        // ===== ����V�[�P���X�i�ʏ�̈ړ��j =====
        SequenceNode* randomMoveSequence = new SequenceNode();
        randomMoveSequence->AddChild(new CheckRandomMove(enemy));
        randomMoveSequence->AddChild(new PatrolMovement(enemy));

        root.AddChild(cooldownSequence);
        root.AddChild(detectAndChasePlayerSequence);
        root.AddChild(randomMoveSequence);
    }

    void RunBehaviorTree() 
    {
        root.Execute();
    }
};
