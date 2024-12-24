//=============================================================================
//
// 影処理 [shadow.h]
// Author : 
//
//=============================================================================
#pragma once


#define	MAX_SHADOW			(1024)			// 影最大数

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitShadow(void);
void UninitShadow(void);
void UpdateShadow(void);
void DrawShadow(void);

int CreateShadow(XMFLOAT3 pos, float fSizeX, float fSizeZ);
void ReleaseShadow(int nIdxShadow);
void SetPositionShadow(int nIdxShadow, XMFLOAT3 pos);
void SetColorShadow(int nIdxShadow, XMFLOAT4 col);
void SetShadowSize(int nIdxShadow, float fSizeX, float fSizeZ);