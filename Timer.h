#pragma once
//=============================================================================
//
// Timerèàóù [Timer.h]
// Author : 
//
//=============================================================================

#include "SingletonBase.h"
#include "main.h"
#include "debugproc.h"

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
        lastTime = currentTime;
        scaledDeltaTime = deltaTime * timeScale;
        elapsedTime += deltaTime;

#ifdef _DEBUG
        PrintDebugProc("scaledDeltaTime: %f\n", scaledDeltaTime);
        PrintDebugProc("deltaTime: %f\n", deltaTime);
#endif // _DEBUG

    }

    float GetDeltaTime(void) { return deltaTime; }
    //float GetScaledDeltaTime(void) { return scaledDeltaTime; }
    float GetScaledDeltaTime(void) { return 1; };// scaledDeltaTime; }
    float GetElapsedTime(void) { return elapsedTime; }
};