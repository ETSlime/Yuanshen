#pragma once
//=============================================================================
//
// �t���[���o�ߎ��ԁE�ݐώ��Ԃ��v������^�C�}�[�N���X [Timer.h]
// Author : 
// �t���[���Ԃ̌o�ߎ��ԁE�ݐώ��Ԃ𐸖��ɑ��肵�A
// �Œ�^�C���X�e�b�v����уX�P�[�����O�Ή��̎��Ԑ����񋟂���
// 
//=============================================================================

#include "Utility/SingletonBase.h"
#include "main.h"
#include "Utility/Debugproc.h"

class Timer : public SingletonBase<Timer>
{
private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    float elapsedTime;
    float deltaTime;
    float scaledDeltaTime;
    const float targetFrameRate = 60.0f;
    const float timeScale = targetFrameRate;
    const float maxDeltaTime = 1.0f / 10.0f;
    DebugProc& debugProc = DebugProc::get_instance();

public:
    Timer() 
    {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);

        Init();
    }

    void Init()
    {
        Update();
    }

    void Update() 
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        deltaTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        if (deltaTime > maxDeltaTime)
        {
            deltaTime = maxDeltaTime; // ������������
        }
        deltaTime = 1.0f / 60.0f;
        lastTime = currentTime;
        scaledDeltaTime = deltaTime * timeScale; // �X�P�[�����O
        elapsedTime += deltaTime;

#ifdef _DEBUG
        debugProc.PrintDebugProc("scaledDeltaTime: %f\n", scaledDeltaTime);
        debugProc.PrintDebugProc("deltaTime: %f\n", deltaTime);
#endif // _DEBUG

    }

    float GetDeltaTime(void) { return deltaTime; }
    //float GetScaledDeltaTime(void) { return scaledDeltaTime; }
    float GetScaledDeltaTime(void) { return 1; };// scaledDeltaTime; }
    float GetElapsedTime(void) { return elapsedTime; }
};