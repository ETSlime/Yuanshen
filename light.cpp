//=============================================================================
//
// ライト処理 [light.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "player.h"
#include "input.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(1.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(3000.0f)										// ビュー平面のFarZ値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];
static LightViewProjBuffer g_LightViewProj;
static FOG		g_Fog;

static BOOL		g_FogEnable = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
void InitLight(void)
{

	//ライト初期化
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
		g_Light[i].Attenuation = 100.0f;	// 減衰距離
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ライトのタイプ
		g_Light[i].Enable = FALSE;			// ON / OFF
		SetLight(i, &g_Light[i]);

		g_LightViewProj.ProjView[i] = XMMatrixIdentity();
	}


	g_Light[1].Enable = FALSE;									// このライトをON
	g_Light[0].Enable = TRUE;

	// フォグの初期化（霧の効果）
	g_Fog.FogStart = 100.0f;									// 視点からこの距離離れるとフォグがかかり始める
	g_Fog.FogEnd   = 250.0f;									// ここまで離れるとフォグの色で見えなくなる
	g_Fog.FogColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );		// フォグの色
	SetFog(&g_Fog);
	SetFogEnable(g_FogEnable);				// 他の場所もチェックする shadow

}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateLight(void)
{
	PLAYER* player = GetPlayer();

	// 並行光源の設定（世界を照らす光）
	g_Light[1].Direction = XMFLOAT3(-1.0f, -15.0f, -13.0f);		// 光の向き
	g_Light[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 光の色
	g_Light[1].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源

	g_Light[1].Ambient = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	g_Light[1].Position = XMFLOAT3(600.0f, 500.0f, 250.0f);

	//g_Light[0].Direction = XMFLOAT3(-1.0f, -15.0f, -13.0f);		// 光の向き
	g_Light[0].Direction = XMFLOAT3(1.1f, -1.0f, 0.0f);		// 光の向き
	g_Light[0].Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);	// 光の色
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
	//g_Light[0].Position = XMFLOAT3(-400.0f, 200.0f, 50.0f);

	g_Light[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f); // player->trans[ALL].pos;
	g_Light[0].Position.y += 11;
	g_Light[0].Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	SetLight(0, &g_Light[0]);
	XMFLOAT3 targetPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 lightUp = { 0.0f, 1.0f, 0.0f };
	XMVECTOR pos = XMLoadFloat3(&g_Light[0].Position);
	XMVECTOR lightDir = XMLoadFloat3(&g_Light[0].Direction);
	XMMATRIX lightView = XMMatrixLookAtLH(
		XMLoadFloat3(&g_Light[0].Position),
		pos + lightDir,
		XMLoadFloat3(&lightUp)
	);
	XMMATRIX lightProj = XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_WIDTH, VIEW_NEAR_Z, VIEW_FAR_Z);
	g_LightViewProj.ProjView[0] = XMMatrixTranspose(lightView * lightProj);


	//targetPosition = XMFLOAT3(player->trans[ALL].pos.x, player->trans[ALL].pos.y, player->trans[ALL].pos.z);
	XMVECTOR dir = XMVector3Normalize(
		XMVectorSubtract(XMLoadFloat3(&targetPosition), XMLoadFloat3(&g_Light[1].Position))
	);
	XMFLOAT3 lightDirection;
	XMStoreFloat3(&lightDirection, dir);
	g_Light[1].Direction = lightDirection;
	lightUp = { 0.0f, 1.0f, 0.0f };
	//lightView = XMMatrixLookAtLH(
	//	XMLoadFloat3(&g_Light[1].Position),
	//	XMLoadFloat3(&targetPosition),
	//	XMLoadFloat3(&lightUp)
	//);
	lightView = XMMatrixLookAtLH(
		XMLoadFloat3(&g_Light[1].Position),
		XMLoadFloat3(&targetPosition),
		XMLoadFloat3(&lightUp)
	);
	lightProj = XMMatrixOrthographicLH(SCREEN_WIDTH * 1.2f, SCREEN_HEIGHT * 1.2f, VIEW_NEAR_Z, VIEW_FAR_Z);
	g_LightViewProj.ProjView[1] = XMMatrixTranspose(lightView * lightProj);

	SetLight(1, &g_Light[1]);									// これで設定している

	if (GetKeyboardTrigger(DIK_L))
	{
		if (g_Light[0].Enable == TRUE)
			g_Light[0].Enable = FALSE;
		else
			g_Light[0].Enable = TRUE;

		if (g_Light[1].Enable == TRUE)
			g_Light[1].Enable = FALSE;
		else
			g_Light[1].Enable = TRUE;

	}
}

void SetLightViewProjBuffer(int lightIdx)
{
	g_LightViewProj.LightIndex = lightIdx;
	//SetLightProjView2(&g_LightViewProj.ProjView[lightIdx]);
	SetLightProjView(&g_LightViewProj);
}


//=============================================================================
// ライトの設定
// Typeによってセットするメンバー変数が変わってくる
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// フォグの設定
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


BOOL	GetFogEnable(void)
{
	return(g_FogEnable);
}


