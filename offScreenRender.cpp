//=============================================================================
//
// [offScreenRender.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "sprite.h"
#include "offScreenRender.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11ShaderResourceView* g_OffScreenSRV = NULL;
static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11Buffer* g_OffScreenCBuffer = NULL;
static ID3D11RenderTargetView* g_OffScreenRTV = NULL;

static float g_ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };	// 背景色
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitOffScreenRender()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = SCREEN_WIDTH;
	textureDesc.Height = SCREEN_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* renderTargetTexture = nullptr;
	GetDevice()->CreateTexture2D(&textureDesc, nullptr, &renderTargetTexture);

	GetDevice()->CreateRenderTargetView(renderTargetTexture, nullptr, &g_OffScreenRTV);

	GetDevice()->CreateShaderResourceView(renderTargetTexture, nullptr, &g_OffScreenSRV);

	float centerX = SCREEN_WIDTH / 2.0f;
	float centerY = SCREEN_HEIGHT / 2.0f;
	float smallScreenWidth = SCREEN_WIDTH / 4.0f;
	float smallScreenHeight = SCREEN_HEIGHT / 4.0f;



	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	SetSprite(g_VertexBuffer, centerX * 1.75f, centerY * .25, smallScreenWidth, smallScreenHeight, 0.0f, 0.0f, 1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	// 定数バッファ生成
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(OffSCREEN_CBUFFER);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	//ワールドマトリクス
	GetDevice()->CreateBuffer(&hBufferDesc, NULL, &g_OffScreenCBuffer);
	GetDeviceContext()->VSSetConstantBuffers(10, 1, &g_OffScreenCBuffer);
	GetDeviceContext()->PSSetConstantBuffers(10, 1, &g_OffScreenCBuffer);

	return S_OK;

}

void UninitOffScreenRender(void)
{
	if (g_OffScreenSRV) g_OffScreenSRV->Release();
	if (g_VertexBuffer) g_VertexBuffer->Release();
	if (g_OffScreenCBuffer) g_OffScreenCBuffer->Release();
	if (g_OffScreenRTV) g_OffScreenRTV->Release();
}

void DrawOffScreenRender(void)
{
	SetOffScreenModeBuffer(1);

	ResetRenderTarget();

	//// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(7, 1, &g_OffScreenSRV);

	SetWorldViewProjection2D();

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.noTexSampling = TRUE;
	SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	// ポリゴンの描画
	GetDeviceContext()->Draw(4, 0);

	SetOffScreenModeBuffer(0);
}

void SetOffScreenRender(void)
{
	GetDeviceContext()->OMSetRenderTargets(1, &g_OffScreenRTV, nullptr);
	GetDeviceContext()->ClearRenderTargetView(g_OffScreenRTV, g_ClearColor);
}

void SetOffScreenModeBuffer(int mode)
{
	OffSCREEN_CBUFFER md;
	md.mode = mode;
	GetDeviceContext()->UpdateSubresource(g_OffScreenCBuffer, 0, NULL, &md, 0, 0);
}