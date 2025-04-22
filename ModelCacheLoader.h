//=============================================================================
//
// ModelCacheLoader処理 [ModelCacheLoader.h]
// Author :
//
//=============================================================================
#pragma once
#include "main.h"
#include "Renderer.h"
#include "SimpleArray.h"

struct MODEL_DATA;

class ModelCacheLoader 
{
public:

    static inline bool SafeReadFile(HANDLE hFile, void* dest, DWORD size) 
    {
        DWORD bytesRead;
        return ReadFile(hFile, dest, size, &bytesRead, nullptr);
    }

    // 書き込み用の安全ラッパー関数
    static bool SafeWriteFile(HANDLE hFile, const void* src, DWORD size) 
    {
        DWORD bytesWritten = 0;
        // WriteFile が失敗した場合や、書き込んだバイト数が一致しない場合は false を返す
        return WriteFile(hFile, src, size, &bytesWritten, nullptr) && bytesWritten == size;
    }

    static bool LoadFromCache(const char* cachePath, MODEL_DATA* modelData);
    static bool SaveToCache(const char* cachePath, const MODEL_DATA* modelData);

    // .obj パスから .bin パスに変換（char* を使う）
    static void ConvertToBinPath(const char* objPath, char* outBinPath, size_t maxLength);
};