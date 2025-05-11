//=============================================================================
//
// レンダリング処理 [Renderer.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "Renderer.h"
#include "LightManager.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
//デバッグ用画面テキスト出力を有効にする
#define DEBUG_DISP_TEXTOUT
//シェーダーデバッグ設定を有効にする
//#define DEBUG_SHADER


Renderer::Renderer()
{
	g_RenderMode = RenderMode::OBJ;
}

ID3D11Device* Renderer::GetDevice( void )
{
	return g_D3DDevice;
}


ID3D11DeviceContext* Renderer::GetDeviceContext( void )
{
	return g_ImmediateContext;
}


void Renderer::SetDepthEnable( BOOL Enable )
{
	if (Enable)
	{
		g_ImmediateContext->OMSetDepthStencilState(g_DepthStateEnable, NULL);
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_SceneDepthStencilView);
	}
	else
	{
		g_ImmediateContext->OMSetDepthStencilState(g_DepthStateDisable, NULL);
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, nullptr);
	}
}

void Renderer::SetDepthForParticle(void)
{
	g_ImmediateContext->OMSetDepthStencilState(g_DepthStateParticle, 0);
	g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_SceneDepthStencilView);
}

void Renderer::SetDepthMode(DepthMode mode)
{
	ID3D11DepthStencilState* depthState = nullptr;
	ID3D11DepthStencilView* depthView = nullptr;

	switch (mode)
	{
	case DepthMode::Enable:
		// 深度テスト・書き込みどちらも有効（3Dモデル描画用）
		depthState = g_DepthStateEnable;
		depthView = g_SceneDepthStencilView;
		break;

	case DepthMode::Particle:
		// 深度テストは有効、書き込みは無効（パーティクル描画用）
		depthState = g_DepthStateParticle;
		depthView = g_SceneDepthStencilView;
		break;

	case DepthMode::Disable:
		// 深度テスト無効、DSVもバインドしない（UI描画などに使用）
		depthState = g_DepthStateDisable;
		depthView = nullptr;
		break;
	}

	// 深度ステンシルステートを設定
	g_ImmediateContext->OMSetDepthStencilState(depthState, 0);

	// レンダーターゲットと深度ビューのバインド
	g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, depthView);
}


void Renderer::SetBlendState(BLEND_MODE bm)
{
	g_BlendStateParam = bm;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	switch (g_BlendStateParam)
	{
	case BLEND_MODE_NONE:
		g_ImmediateContext->OMSetBlendState(g_BlendStateNone, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_ALPHABLEND:
		g_ImmediateContext->OMSetBlendState(g_BlendStateAlphaBlend, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_ADD:
		g_ImmediateContext->OMSetBlendState(g_BlendStateAdd, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_SUBTRACT:
		g_ImmediateContext->OMSetBlendState(g_BlendStateSubtract, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_SWORDTRAIL:
		g_ImmediateContext->OMSetBlendState(g_BlendStateSwordTrail, blendFactor, 0xffffffff);
		break;
	}
}

void Renderer::SetCullingMode(CULL_MODE cm)
{
	switch (cm)
	{
	case CULL_MODE_NONE:
		g_ImmediateContext->RSSetState(g_RasterStateCullOff);
		break;
	case CULL_MODE_FRONT:
		g_ImmediateContext->RSSetState(g_RasterStateCullCW);
		break;
	case CULL_MODE_BACK:
		g_ImmediateContext->RSSetState(g_RasterStateCullCCW);
		break;
	}
}

void Renderer::SetRenderLayer(RenderLayer layer)
{
	switch (layer)
	{
	case RenderLayer::LAYER_0:
		g_ImmediateContext->RSSetState(g_RasterizerLayer0);
		break;
	case RenderLayer::LAYER_1:
		g_ImmediateContext->RSSetState(g_RasterizerLayer1);
		break;
	case RenderLayer::LAYER_2:
		g_ImmediateContext->RSSetState(g_RasterizerLayer2);
		break;
	case RenderLayer::LAYER_3:
		g_ImmediateContext->RSSetState(g_RasterizerLayer3);
		break;
	default:
		g_ImmediateContext->RSSetState(g_RasterStateCullOff);
		break;
	}
}

void Renderer::SetFillMode(D3D11_FILL_MODE mode)
{
	ID3D11RasterizerState* pRasterizerState;
	GetDeviceContext()->RSGetState(&pRasterizerState);
	D3D11_RASTERIZER_DESC rasterDesc;
	pRasterizerState->GetDesc(&rasterDesc);
	rasterDesc.FillMode = mode;
	ID3D11RasterizerState* newRasterizerState;
	g_D3DDevice->CreateRasterizerState(&rasterDesc, &newRasterizerState);
	g_ImmediateContext->RSSetState(newRasterizerState);

}

void Renderer::SetBoneMatrix(const XMMATRIX matrices[BONE_MAX]) const
{
	g_ImmediateContext->UpdateSubresource(g_BoneMatrixBuffer, 0, NULL, matrices, 0, 0);
}

void Renderer::SetAlphaTestEnable(BOOL flag)
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	if (flag)
		blendDesc.AlphaToCoverageEnable = TRUE;
	else
		blendDesc.AlphaToCoverageEnable = FALSE;

	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	switch (g_BlendStateParam)
	{
	case BLEND_MODE_NONE:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_ALPHABLEND:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_ADD:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_SUBTRACT:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	}

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState* blendState = NULL;
	g_D3DDevice->CreateBlendState(&blendDesc, &blendState);
	g_ImmediateContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	if (blendState != NULL)
		blendState->Release();
}


void Renderer::SetWorldViewProjection2D( void )
{
	XMMATRIX world;
	world = XMMatrixTranspose(XMMatrixIdentity());


	XMMATRIX invWorld = XMMatrixTranspose(XMMatrixIdentity());

	WorldMatrixBuffer worldBuffer;
	worldBuffer.world = world;
	worldBuffer.invWorld = invWorld;

	g_ImmediateContext->UpdateSubresource(g_WorldBuffer, 0, NULL, &worldBuffer, 0, 0);

	XMMATRIX view;
	view = XMMatrixTranspose(XMMatrixIdentity());
	g_ImmediateContext->UpdateSubresource(g_ViewBuffer, 0, NULL, &view, 0, 0);

	XMMATRIX worldViewProjection;
	worldViewProjection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	worldViewProjection = XMMatrixTranspose(worldViewProjection);
	g_ImmediateContext->UpdateSubresource(g_ProjectionBuffer, 0, NULL, &worldViewProjection, 0, 0);
}


void Renderer::SetCurrentWorldMatrix(const XMMATRIX *WorldMatrix ) const
{
	XMMATRIX world, invWorld;
	world = *WorldMatrix;

	// 最適化のポイント
// 1  もし World が単位行列 (Identity Matrix) ならば 逆転置行列も単位行列になる
//     -  inverse(transpose(Identity)) = Identity なので、XMMatrixIdentity() をそのまま使える
//     - **XMMatrixInverse() のコストをゼロにできる
//     - **逆転置行列をそのまま返すだけで OK** なので、計算時間を大幅に削減できる
//
// 2  XMMatrixIsIdentity() の実行コストは非常に低い！**
//     - XMMatrixIsIdentity() は **4 回の SIMD 比較 (XMVector4Equal) だけでチェックできる**
//     - そのため、**逆行列 (XMMatrixInverse) のコストと比較すると、事前チェックの方が圧倒的に軽い**
//
// 3  非均一スケール (Non-uniform Scale) がある場合、逆転置行列は transpose(inverse(World3x3)) を計算する必要がある**
//     - World にスケールが含まれていない場合、transpose(World3x3) だけでよい。
//     - **スケールがある場合、逆行列計算が必要になるが、これはコストが高い処理なので、単位行列の事前チェックが有効**
	if (XMMatrixIsIdentity(world))
	{
		invWorld = XMMatrixIdentity();
	}
	else
	{
		XMMATRIX world3x3 = XMMatrixSet(
			WorldMatrix->r[0].m128_f32[0], WorldMatrix->r[0].m128_f32[1], WorldMatrix->r[0].m128_f32[2], 0.0f,
			WorldMatrix->r[1].m128_f32[0], WorldMatrix->r[1].m128_f32[1], WorldMatrix->r[1].m128_f32[2], 0.0f,
			WorldMatrix->r[2].m128_f32[0], WorldMatrix->r[2].m128_f32[1], WorldMatrix->r[2].m128_f32[2], 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		invWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, world3x3));
	}

	world = XMMatrixTranspose(world);

	WorldMatrixBuffer buffer;
	buffer.world = world;
	buffer.invWorld = invWorld;

	g_ImmediateContext->UpdateSubresource(g_WorldBuffer, 0, NULL, &buffer, 0, 0);
}

void Renderer::SetViewMatrix(const XMMATRIX *ViewMatrix ) const
{
	XMMATRIX view;
	view = *ViewMatrix;
	view = XMMatrixTranspose(view);

	g_ImmediateContext->UpdateSubresource(g_ViewBuffer, 0, NULL, &view, 0, 0);
}

void Renderer::SetProjectionMatrix(const XMMATRIX *ProjectionMatrix ) const
{
	XMMATRIX projection;
	projection = *ProjectionMatrix;
	projection = XMMatrixTranspose(projection);

	g_ImmediateContext->UpdateSubresource(g_ProjectionBuffer, 0, NULL, &projection, 0, 0);
}

void Renderer::SetMaterial( MATERIAL material )
{
	g_Material.Diffuse = material.Diffuse;
	g_Material.Ambient = material.Ambient;
	g_Material.Specular = material.Specular;
	g_Material.Emission = material.Emission;
	g_Material.Shininess = material.Shininess;
	g_Material.noTexSampling = material.noTexSampling;
	g_Material.lightMapSampling = material.lightMapSampling;
	g_Material.normalMapSampling = material.normalMapSampling;
	g_Material.bumpMapSampling = material.bumpMapSampling;
	g_Material.opacityMapSampling = material.opacityMapSampling;
	g_Material.reflectMapSampling = material.reflectMapSampling;
	g_Material.translucencyMapSampling = material.translucencyMapSampling;

	g_ImmediateContext->UpdateSubresource( g_MaterialBuffer, 0, NULL, &g_Material, 0, 0 );
}

void Renderer::SetLightBuffer(const LIGHT_CBUFFER& lightBuffer)
{
	g_ImmediateContext->UpdateSubresource(g_LightBuffer, 0, NULL, &lightBuffer, 0, 0);
}



void Renderer::SetFogBuffer(void)
{
	g_ImmediateContext->UpdateSubresource(g_FogBuffer, 0, NULL, &g_Fog, 0, 0);
}

void Renderer::SetFogEnable(BOOL flag)
{
	// フラグを更新する
	g_Fog.Enable = flag;

	SetFogBuffer();
}

void Renderer::SetFog(FOG* pFog)
{
	g_Fog.Fog.x = pFog->FogStart;
	g_Fog.Fog.y = pFog->FogEnd;
	g_Fog.FogColor = pFog->FogColor;

	SetFogBuffer();
}

void Renderer::SetRenderProgress(RenderProgressBuffer renderProgress)
{
	g_ImmediateContext->UpdateSubresource(g_RenderProgressBuffer, 0, NULL, &renderProgress, 0, 0);
}

void Renderer::SetFuchi(int flag)
{
	g_Fuchi.fuchi = flag;
	g_ImmediateContext->UpdateSubresource(g_FuchiBuffer, 0, NULL, &g_Fuchi, 0, 0);
}


void Renderer::SetShaderCamera(XMFLOAT3 pos)
{
	XMFLOAT4 tmp = XMFLOAT4( pos.x, pos.y, pos.z, 0.0f );

	g_ImmediateContext->UpdateSubresource(g_CameraPosBuffer, 0, NULL, &tmp, 0, 0);
}

void Renderer::SetRenderSkinnedMeshModel(void)
{
	g_RenderMode = RenderMode::SKINNED_MESH;

	ResetRenderTarget();

	g_ImmediateContext->VSSetShader(m_SkinnedModelShaderSet.vs, nullptr, 0);
	g_ImmediateContext->PSSetShader(m_SkinnedModelShaderSet.ps, nullptr, 0);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_BONE_MATRIX_ARRAY, g_BoneMatrixBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_BONE_MATRIX_ARRAY, g_BoneMatrixBuffer);
}

void Renderer::SetRenderInstance(void)
{
	g_RenderMode = RenderMode::INSTANCE;
}

void Renderer::SetRenderVFX(void)
{
	g_RenderMode = RenderMode::VFX;

	XMMATRIX mtxWorld = XMMatrixIdentity();
	SetCurrentWorldMatrix(&mtxWorld);

	SetCullingMode(CULL_MODE_NONE);

	g_ImmediateContext->VSSetShader(m_VFXShaderSet.vs, NULL, 0);
	g_ImmediateContext->PSSetShader(m_VFXShaderSet.ps, NULL, 0);
}

void Renderer::SetRenderUI(void)
{
	g_RenderMode = RenderMode::UI;

	g_ImmediateContext->VSSetShader(m_UIShaderSet.vs, NULL, 0);
	g_ImmediateContext->PSSetShader(m_UIShaderSet.ps, NULL, 0);
}

void Renderer::SetStaticModelInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(m_StaticModelShaderSet.inputLayout);
}

void Renderer::SetUIInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(m_UIShaderSet.inputLayout);
}

void Renderer::SetSkinnedMeshInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(m_SkinnedModelShaderSet.inputLayout);
}

void Renderer::SetVFXInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(m_VFXShaderSet.inputLayout);
}

void Renderer::SetRenderObject(void)
{
	g_RenderMode = RenderMode::OBJ;

	ResetRenderTarget();

	g_ImmediateContext->VSSetShader(m_StaticModelShaderSet.vs, NULL, 0);
	g_ImmediateContext->PSSetShader(m_StaticModelShaderSet.ps, NULL, 0);
}

void Renderer::ResetRenderTarget(void)
{
	g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_SceneDepthStencilView);
}

void Renderer::SetMainPassViewport(void)
{
	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)SCREEN_WIDTH;
	vp.Height = (FLOAT)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_ImmediateContext->RSSetViewports(1, &vp);
}

void Renderer::SetShadersets(void)
{
	m_StaticModelShaderSet = m_ShaderManager.GetShaderSet(ShaderSetID::StaticModel);
	m_SkinnedModelShaderSet = m_ShaderManager.GetShaderSet(ShaderSetID::SkinnedModel);
	m_InstanceModelShaderSet = m_ShaderManager.GetShaderSet(ShaderSetID::Instanced_Tree);
	m_VFXShaderSet = m_ShaderManager.GetShaderSet(ShaderSetID::VFX);
	m_UIShaderSet = m_ShaderManager.GetShaderSet(ShaderSetID::UI);
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Renderer::Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	HRESULT hr = S_OK;

	// デバイス、スワップチェーン、コンテキスト生成
	DWORD deviceFlags = 0;
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = SCREEN_WIDTH;
	sd.BufferDesc.Height = SCREEN_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = bWindow;

	//デバッグ文字出力用設定
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	deviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	hr = D3D11CreateDeviceAndSwapChain( NULL,
										D3D_DRIVER_TYPE_HARDWARE,
										NULL,
										deviceFlags,
										NULL,
										0,
										D3D11_SDK_VERSION,
										&sd,
										&g_SwapChain,
										&g_D3DDevice,
										&g_FeatureLevel,
										&g_ImmediateContext );
	if( FAILED( hr ) )
		return hr;

	m_ShaderResourceBinder.Initialize(g_ImmediateContext);

	//デバッグ文字出力用設定
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	hr = g_SwapChain->ResizeBuffers(0, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE); // N.B. the GDI compatible flag
	if (FAILED(hr))
		return hr;
#endif

	// レンダーターゲットビュー生成、設定
	ID3D11Texture2D* pBackBuffer = NULL;
	g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	g_D3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_RenderTargetView );
	pBackBuffer->Release();


	D3D11_TEXTURE2D_DESC sceneDepthDesc = {};
	sceneDepthDesc.Width = sd.BufferDesc.Width;
	sceneDepthDesc.Height = sd.BufferDesc.Height;
	sceneDepthDesc.MipLevels = 1;
	sceneDepthDesc.ArraySize = 1;
	sceneDepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	sceneDepthDesc.SampleDesc.Count = 1;
	sceneDepthDesc.Usage = D3D11_USAGE_DEFAULT;
	sceneDepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* sceneDepthTexture = nullptr;
	g_D3DDevice->CreateTexture2D(&sceneDepthDesc, nullptr, &sceneDepthTexture);


	D3D11_DEPTH_STENCIL_VIEW_DESC sceneDSVDesc = {};
	sceneDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	sceneDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	g_D3DDevice->CreateDepthStencilView(sceneDepthTexture, &sceneDSVDesc, &g_SceneDepthStencilView);



	// ラスタライザステート作成
	D3D11_RASTERIZER_DESC rd; 
	ZeroMemory( &rd, sizeof( rd ) );
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE; 
	rd.DepthClipEnable = TRUE; 
	rd.MultisampleEnable = FALSE; 
	g_D3DDevice->CreateRasterizerState( &rd, &g_RasterStateCullOff);

	rd.CullMode = D3D11_CULL_FRONT;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterStateCullCW);

	rd.CullMode = D3D11_CULL_BACK;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterStateCullCCW);

	// カリングモード設定（CCW）
	SetCullingMode(CULL_MODE_BACK);

	ZeroMemory(&rd, sizeof(rd));

	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	rd.DepthClipEnable = TRUE;
	rd.MultisampleEnable = FALSE;

	// Layer 0
	rd.DepthBias = DEPTHBIAS_LAYER_0;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterizerLayer0);

	// Layer 1
	rd.DepthBias = DEPTHBIAS_LAYER_1;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterizerLayer1);

	// Layer 2
	rd.DepthBias = DEPTHBIAS_LAYER_2;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterizerLayer2);

	// Layer 3
	rd.DepthBias = DEPTHBIAS_LAYER_3;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterizerLayer3);


	// ブレンドステートの作成
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory( &blendDesc, sizeof( blendDesc ) );
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState( &blendDesc, &g_BlendStateAlphaBlend );

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateNone);

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateAdd);

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateSubtract);

	ZeroMemory(&blendDesc, sizeof(blendDesc));

	blendDesc.AlphaToCoverageEnable = TRUE;  // アルファ テスト カバレッジを有効にする
	blendDesc.IndependentBlendEnable = FALSE;

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateSwordTrail);

	// アルファブレンド設定
	SetBlendState(BLEND_MODE_ALPHABLEND);


	// 深度ステンシルステート作成
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	//深度Test有効ステート
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateEnable);

	//深度Write無効（パーティクル用）
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateParticle);

	//深度Test無効ステート
	depthStencilDesc.DepthEnable = FALSE;
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateDisable);

	// 深度ステンシルステート設定
	SetDepthEnable(TRUE);


	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory( &samplerDesc, sizeof( samplerDesc ) );

	// general sampler
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	g_D3DDevice->CreateSamplerState( &samplerDesc, &g_SamplerState);
	m_ShaderResourceBinder.BindSampler(ShaderStage::PS, SLOT_SAMPLER_DEFAULT, g_SamplerState);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));

	// shadow map sampler
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.BorderColor[0] = 1.0f;                       
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;      
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	g_D3DDevice->CreateSamplerState(&samplerDesc, &g_SamplerStateShadow);
	m_ShaderResourceBinder.BindSampler(ShaderStage::PS, SLOT_SAMPLER_SHADOW, g_SamplerStateShadow);


	ZeroMemory(&samplerDesc, sizeof(samplerDesc));

	// opacity map sampler
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	g_D3DDevice->CreateSamplerState(&samplerDesc, &g_SamplerStateOpacity);
	m_ShaderResourceBinder.BindSampler(ShaderStage::PS, SLOT_SAMPLER_OPACITY, g_SamplerStateOpacity);

	// 定数バッファ生成
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(WorldMatrixBuffer);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	//ワールドマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WorldBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_WORLD_MATRIX, g_WorldBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_WORLD_MATRIX, g_WorldBuffer);

	//ビューマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ViewBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_VIEW_MATRIX, g_ViewBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_VIEW_MATRIX, g_ViewBuffer);

	//プロジェクションマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ProjectionBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_PROJECTION_MATRIX, g_ProjectionBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_PROJECTION_MATRIX, g_ProjectionBuffer);

	//マテリアル情報
	hBufferDesc.ByteWidth = sizeof(MATERIAL_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_MaterialBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_MATERIAL, g_MaterialBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_MATERIAL, g_MaterialBuffer);

	//ライト情報
	hBufferDesc.ByteWidth = sizeof(LIGHT_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_LIGHT, g_LightBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_LIGHT, g_LightBuffer);

	//フォグ情報
	hBufferDesc.ByteWidth = sizeof(FOG_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FogBuffer);

	//縁取り
	hBufferDesc.ByteWidth = sizeof(FUCHI);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FuchiBuffer);

	//カメラ
	hBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_CameraPosBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_CAMERA_POS, g_CameraPosBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_CAMERA_POS, g_CameraPosBuffer);

	hBufferDesc.ByteWidth = sizeof(BoneMatrices);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_BoneMatrixBuffer);


	hBufferDesc.ByteWidth = sizeof(LIGHTMODE_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightModeBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_LIGHT_MODE, g_LightModeBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_LIGHT_MODE, g_LightModeBuffer);
	
	hBufferDesc.ByteWidth = sizeof(RenderProgressBuffer);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_RenderProgressBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_RENDER_PROGRESS, g_RenderProgressBuffer);
	m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::PS, SLOT_CB_RENDER_PROGRESS, g_RenderProgressBuffer);

	// 入力レイアウト設定
	g_ImmediateContext->IASetInputLayout(m_ShaderManager.GetInputLayout(VertexLayoutID::Static));

	//マテリアル初期化
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	return S_OK;
}


//=============================================================================
// 終了処理
//=============================================================================
void Renderer::Uninit(void)
{
	// オブジェクト解放
	SafeRelease(&g_DepthStateEnable);
	SafeRelease(&g_DepthStateDisable);
	SafeRelease(&g_DepthStateParticle);

	SafeRelease(&g_BlendStateNone);
	SafeRelease(&g_BlendStateAlphaBlend);
	SafeRelease(&g_BlendStateAdd);
	SafeRelease(&g_BlendStateSubtract);

	SafeRelease(&g_RasterStateCullOff);
	SafeRelease(&g_RasterStateCullCW);
	SafeRelease(&g_RasterStateCullCCW);
	SafeRelease(&g_RasterizerLayer0);
	SafeRelease(&g_RasterizerLayer1);
	SafeRelease(&g_RasterizerLayer2);
	SafeRelease(&g_RasterizerLayer3);


	SafeRelease(&g_WorldBuffer);
	SafeRelease(&g_ViewBuffer);
	SafeRelease(&g_ProjectionBuffer);
	SafeRelease(&g_MaterialBuffer);
	SafeRelease(&g_LightBuffer);
	SafeRelease(&g_FogBuffer);
	SafeRelease(&g_FuchiBuffer);
	SafeRelease(&g_CameraPosBuffer);
	SafeRelease(&g_LightProjViewBuffer);
	SafeRelease(&g_BoneMatrixBuffer);
	SafeRelease(&g_LightModeBuffer);
	SafeRelease(&g_RenderProgressBuffer);

	SafeRelease(&g_SamplerState);
	SafeRelease(&g_SamplerStateShadow);
	SafeRelease(&g_SamplerStateOpacity);

	if (g_ImmediateContext)		g_ImmediateContext->ClearState();
	SafeRelease(&g_RenderTargetView);
	SafeRelease(&g_SwapChain);
	SafeRelease(&g_ImmediateContext);
	SafeRelease(&g_D3DDevice);
}



//=============================================================================
// バックバッファクリア
//=============================================================================
void Renderer::Clear(void)
{
	// バックバッファクリア
	g_ImmediateContext->ClearRenderTargetView( g_RenderTargetView, g_ClearColor );
	g_ImmediateContext->ClearDepthStencilView( g_SceneDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void Renderer::SetClearColor(float* color4)
{
	g_ClearColor[0] = color4[0];
	g_ClearColor[1] = color4[1];
	g_ClearColor[2] = color4[2];
	g_ClearColor[3] = color4[3];
}


//=============================================================================
// プレゼント
//=============================================================================
void Renderer::Present(void)
{
	g_SwapChain->Present( 0, 0 );
}


//=============================================================================
// デバッグ用テキスト出力
//=============================================================================
void Renderer::DebugTextOut(char* text, int x, int y)
{
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	HRESULT hr;

	//バックバッファからサーフェスを取得する
	IDXGISurface1* pBackSurface = NULL;
	hr = g_SwapChain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&pBackSurface);

	if (SUCCEEDED(hr))
	{
		//取得したサーフェスからデバイスコンテキストを取得する
		HDC hdc;
		hr = pBackSurface->GetDC(FALSE, &hdc);

		if (SUCCEEDED(hr))
		{
			//文字色を白に変更
			SetTextColor(hdc, RGB(255, 255, 255));
			//背景を透明に変更
			SetBkMode(hdc, TRANSPARENT);

			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = SCREEN_WIDTH;
			rect.bottom = SCREEN_HEIGHT;

			//テキスト出力
			DrawText(hdc, text, (int)strlen(text), &rect, DT_LEFT);

			//デバイスコンテキストを解放する
			pBackSurface->ReleaseDC(NULL);
		}
		//サーフェスを解放する
		pBackSurface->Release();

		//レンダリングターゲットがリセットされるのでセットしなおす
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_SceneDepthStencilView);
	}
#endif
}

void Renderer::SetLightModeBuffer(int mode)
{
	LIGHTMODE_CBUFFER md;
	md.mode = mode;
	g_ImmediateContext->UpdateSubresource(g_LightModeBuffer, 0, NULL, &md, 0, 0);
}

RenderMode Renderer::GetRenderMode(void)
{
	return g_RenderMode;
}

void Renderer::SetRenderMode(RenderMode mode)
{
	g_RenderMode = mode;
}

void Renderer::BindViewBuffer(ShaderStage stage)
{
	m_ShaderResourceBinder.BindConstantBuffer(stage, SLOT_CB_VIEW_MATRIX, g_ViewBuffer);
}
