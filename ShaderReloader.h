//=============================================================================
//
// ShaderReloader処理 [ShaderReloader.h]
// Author : 
//
//=============================================================================
#pragma once
#include "HashMap.h"

class ShaderReloader
{
public:
    // ファイルを監視対象に追加する
    void WatchFile(const char* path);

    // ファイルが変更されたかをチェックする
    bool HasFileChanged(const char* path);

private:
    // ファイルパスと最終更新時刻のマップ
    HashMap<const char*, FILETIME, CharPtrHash, CharPtrEquals> m_fileTimeMap;
};
