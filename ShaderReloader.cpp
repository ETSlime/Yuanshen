//=============================================================================
//
// ShaderReloader処理 [ShaderReloader.cpp]
// Author : 
//
//=============================================================================
#include "ShaderReloader.h"

void ShaderReloader::WatchFile(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA attr;

    // ファイル属性を取得
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &attr))
    {
        // 最終更新時刻を記録
        m_fileTimeMap[path] = attr.ftLastWriteTime;
    }
}

bool ShaderReloader::HasFileChanged(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA attr;

    // ファイル属性を取得できなければ変更なし
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr))
    {
        return false;
    }

    FILETIME currentTime = attr.ftLastWriteTime;

    auto it = m_fileTimeMap.find(path);

    // 初回アクセスの場合は登録だけ行い、変更なしと判断
    if (it == m_fileTimeMap.end())
    {
        m_fileTimeMap[path] = currentTime;
        return false;
    }

    // 前回の記録と比較して違っていれば変更とみなす
    if (CompareFileTime(&currentTime, &(*it).value) != 0)
    {
        (*it).value = currentTime;
        return true;
    }

    return false;
}
