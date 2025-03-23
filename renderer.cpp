//=============================================================================
//
// レンダリング処理 [renderer.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "sprite.h"
//デバッグ用画面テキスト出力を有効にする
#define DEBUG_DISP_TEXTOUT
//シェーダーデバッグ設定を有効にする
//#define DEBUG_SHADER


Renderer::Renderer()
{
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_ShadowDSV[i] = NULL;
		g_ShadowMapSRV[i] = NULL;
	}

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

void Renderer::SetBoneMatrix(XMMATRIX matrices[BONE_MAX])
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

void Renderer::SetLightBuffer(void)
{
	g_ImmediateContext->UpdateSubresource(g_LightBuffer, 0, NULL, &g_Light, 0, 0);
}

void Renderer::SetLightEnable(BOOL flag)
{
	// フラグを更新する
	g_Light.Enable = flag;

	SetLightBuffer();
}

void Renderer::SetLight(int index, LIGHT* pLight)
{
	g_Light.Position[index] = XMFLOAT4(pLight->Position.x, pLight->Position.y, pLight->Position.z, 0.0f);
	g_Light.Direction[index] = XMFLOAT4(pLight->Direction.x, pLight->Direction.y, pLight->Direction.z, 0.0f);
	g_Light.Diffuse[index] = pLight->Diffuse;
	g_Light.Ambient[index] = pLight->Ambient;
	g_Light.Flags[index].Type = pLight->Type;
	g_Light.Flags[index].OnOff = pLight->Enable;
	g_Light.Attenuation[index].x = pLight->Attenuation;

	//g_Light.LightViewProj = pLight->LightViewProj;

	SetLightBuffer();
}

void Renderer::SetLightProjView(LightViewProjBuffer *lightBuffer)
{
	lightBuffer->ProjView[4] = lightBuffer->ProjView[lightBuffer->LightIndex];
	g_ImmediateContext->UpdateSubresource(g_LightProjViewBuffer, 0, NULL, lightBuffer, 0, 0);
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

void Renderer::SetRenderShadowMap(int lightIdx)
{
	g_RenderMode = RenderMode::OBJ_SHADOW;

	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	g_ImmediateContext->PSSetShaderResources(1, 1, nullSRV);

	g_ImmediateContext->OMSetRenderTargets(0, nullptr, g_ShadowDSV[lightIdx]);


	g_ImmediateContext->VSSetShader(g_DepthVertexShader, nullptr, 0);
	g_ImmediateContext->PSSetShader(nullptr, nullptr, 0);
	
	SetLightViewProjBuffer(lightIdx);

}

void Renderer::SetRenderSkinnedMeshShadowMap(int lightIdx)
{
	g_RenderMode = RenderMode::SKINNED_MESH_SHADOW;


	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	g_ImmediateContext->PSSetShaderResources(1, 1, nullSRV);

	g_ImmediateContext->OMSetRenderTargets(0, nullptr, g_ShadowDSV[lightIdx]);


	g_ImmediateContext->VSSetShader(g_DepthSkinnedMeshVertexShader, nullptr, 0);
	g_ImmediateContext->PSSetShader(nullptr, nullptr, 0);

	SetLightViewProjBuffer(lightIdx);

}

void Renderer::SetRenderInstanceShadowMap(int lightIdx)
{
	g_RenderMode = RenderMode::INSTANCE_SHADOW;


	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	g_ImmediateContext->PSSetShaderResources(1, 1, nullSRV);

	g_ImmediateContext->OMSetRenderTargets(0, nullptr, g_ShadowDSV[lightIdx]);

	SetLightViewProjBuffer(lightIdx);

}

void Renderer::SetRenderSkinnedMeshModel(void)
{
	g_RenderMode = RenderMode::SKINNED_MESH;

	ResetRenderTarget();

	g_ImmediateContext->VSSetShader(g_SkinnedMeshVertexShader, nullptr, 0);
	g_ImmediateContext->PSSetShader(g_SkinnedMeshPixelShader, nullptr, 0);
	g_ImmediateContext->VSSetConstantBuffers(11, 1, &g_BoneMatrixBuffer);
	g_ImmediateContext->PSSetConstantBuffers(11, 1, &g_BoneMatrixBuffer);

	g_ImmediateContext->PSSetShaderResources(1, LIGHT_MAX, g_ShadowMapSRV);
}

void Renderer::SetRenderInstance(void)
{
	g_RenderMode = RenderMode::INSTANCE;

	g_ImmediateContext->PSSetShaderResources(1, LIGHT_MAX, g_ShadowMapSRV);
}

void Renderer::SetRenderVFX(void)
{
	g_RenderMode = RenderMode::VFX;

	XMMATRIX mtxWorld = XMMatrixIdentity();
	SetCurrentWorldMatrix(&mtxWorld);

	SetCullingMode(CULL_MODE_NONE);

	g_ImmediateContext->VSSetShader(g_VFXVertexShader, NULL, 0);
	g_ImmediateContext->PSSetShader(g_VFXPixelShader, NULL, 0);
}

void Renderer::SetRenderUI(void)
{
	g_RenderMode = RenderMode::UI;

	g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
	g_ImmediateContext->PSSetShader(g_PixelShader, NULL, 0);
}

void Renderer::SetModelInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(g_VertexLayout);
}

void Renderer::SetSkinnedMeshInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(g_SkinnedMeshVertexLayout);
}

void Renderer::SetVFXInputLayout(void)
{
	g_ImmediateContext->IASetInputLayout(g_VFXVertexLayout);
}

void Renderer::SetRenderObject(void)
{
	g_RenderMode = RenderMode::OBJ;

	ResetRenderTarget();

	g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
	g_ImmediateContext->PSSetShader(g_PixelShader, NULL, 0);
	g_ImmediateContext->PSSetShaderResources(1, LIGHT_MAX, g_ShadowMapSRV);
}



void Renderer::ResetRenderTarget(void)
{
	g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_SceneDepthStencilView);
}

void Renderer::ClearShadowDSV(int lightIdx)
{
	g_ImmediateContext->ClearDepthStencilView(g_ShadowDSV[lightIdx], D3D11_CLEAR_DEPTH, 1.0f, 0);
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

void Renderer::SetShadowPassViewport(void)
{
	D3D11_VIEWPORT shadowVP;
	shadowVP.Width = (FLOAT)SHADOWMAP_SIZE;
	shadowVP.Height = (FLOAT)SHADOWMAP_SIZE;
	shadowVP.MinDepth = 0.0f;
	shadowVP.MaxDepth = 1.0f;
	shadowVP.TopLeftX = 0;
	shadowVP.TopLeftY = 0;
	g_ImmediateContext->RSSetViewports(1, &shadowVP);
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



	for (UINT i = 0; i < LIGHT_MAX; i++)
	{
		//ステンシル用テクスチャー作成
		ID3D11Texture2D* depthTexture = NULL;
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(td));
		td.Width = SHADOWMAP_SIZE;
		td.Height = SHADOWMAP_SIZE;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R32_TYPELESS;
		td.SampleDesc = sd.SampleDesc;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		g_D3DDevice->CreateTexture2D(&td, NULL, &depthTexture);

		//ステンシルターゲット作成
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(dsvd));
		dsvd.Format = DXGI_FORMAT_D32_FLOAT;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Flags = 0;
		g_D3DDevice->CreateDepthStencilView(depthTexture, &dsvd, &g_ShadowDSV[i]);


		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		g_D3DDevice->CreateShaderResourceView(depthTexture, &srvDesc, &g_ShadowMapSRV[i]);
	}


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

	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateEnable );//深度有効ステート

	//depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateDisable );//深度無効ステート

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

	ID3D11SamplerState* samplerState = NULL;
	g_D3DDevice->CreateSamplerState( &samplerDesc, &samplerState );

	g_ImmediateContext->PSSetSamplers( 0, 1, &samplerState );

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

	ID3D11SamplerState* samplerStateShadow = NULL;
	g_D3DDevice->CreateSamplerState(&samplerDesc, &samplerStateShadow);

	g_ImmediateContext->PSSetSamplers(1, 1, &samplerStateShadow);


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

	ID3D11SamplerState* samplerStateOpacity = NULL;
	g_D3DDevice->CreateSamplerState(&samplerDesc, &samplerStateOpacity);

	g_ImmediateContext->PSSetSamplers(2, 1, &samplerStateOpacity);

	// 頂点シェーダコンパイル・生成
	ID3DBlob* pErrorBlob2;
	ID3DBlob* pVSBlob2 = NULL;
	hr = D3DX11CompileFromFile("DepthMap.hlsl", NULL, NULL, "VS", "vs_4_0", 0, 0, NULL, &pVSBlob2, &pErrorBlob2, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob2->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), NULL, &g_DepthVertexShader);

	pVSBlob2->Release();

	hr = D3DX11CompileFromFile("DepthMap.hlsl", NULL, NULL, "SkinnedMeshVertexShaderPolygon", "vs_4_0", 0, 0, NULL, &pVSBlob2, &pErrorBlob2, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob2->GetBufferPointer(), "SkinnedMeshVertexShaderPolygon", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), NULL, &g_DepthSkinnedMeshVertexShader);

	pVSBlob2->Release();

	ID3DBlob* pErrorBlob3;
	ID3DBlob* pVSBlob3 = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "SkinnedMeshVertexShaderPolygon", "vs_4_0", 0, 0, NULL, &pVSBlob3, &pErrorBlob3, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob3->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob3->GetBufferPointer(), pVSBlob3->GetBufferSize(), NULL, &g_SkinnedMeshVertexShader);


	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG) && defined(DEBUG_SHADER)
	shFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	hr = D3DX11CompileFromFile( "shader.hlsl", NULL, NULL, "VertexShaderPolygon", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR );
	}

	g_D3DDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShader );


	// 入力レイアウト生成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE( layout );

	g_D3DDevice->CreateInputLayout( layout,
		numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_VertexLayout );

	pVSBlob->Release();

	D3D11_INPUT_ELEMENT_DESC skinnedMeshLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, Normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, Tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, Bitangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, Diffuse), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, Weights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(SKINNED_VERTEX_3D, BoneIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	numElements = ARRAYSIZE(skinnedMeshLayout);

	g_D3DDevice->CreateInputLayout(skinnedMeshLayout,
		numElements,
		pVSBlob3->GetBufferPointer(),
		pVSBlob3->GetBufferSize(),
		&g_SkinnedMeshVertexLayout);

	pVSBlob3->Release();

	// ピクセルシェーダコンパイル・生成
	ID3DBlob* pPSBlob = NULL;
	hr = D3DX11CompileFromFile( "shader.hlsl", NULL, NULL, "PixelShaderPolygon", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR );
	}

	g_D3DDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShader );
	
	pPSBlob->Release();

	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "SkinnedMeshPixelShader", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_SkinnedMeshPixelShader);

	pPSBlob->Release();

	ID3DBlob* pErrorBlobVFX = NULL;
	ID3DBlob* pVSBlobVFX = NULL, *pPSBlobVFX = NULL;

	hr = D3DX11CompileFromFile("VFX.hlsl", NULL, NULL, "VS", "vs_4_0", 0, 0, NULL, &pVSBlobVFX, &pErrorBlobVFX, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlobVFX->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlobVFX->GetBufferPointer(), pVSBlobVFX->GetBufferSize(), NULL, &g_VFXVertexShader);

	hr = D3DX11CompileFromFile("VFX.hlsl", NULL, NULL, "PS", "ps_4_0", shFlag, 0, NULL, &pPSBlobVFX, &pErrorBlobVFX, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlobVFX->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlobVFX->GetBufferPointer(), pPSBlobVFX->GetBufferSize(), NULL, &g_VFXPixelShader);

	D3D11_INPUT_ELEMENT_DESC VFXLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	numElements = ARRAYSIZE(VFXLayout);

	g_D3DDevice->CreateInputLayout(VFXLayout,
		numElements,
		pVSBlobVFX->GetBufferPointer(),
		pVSBlobVFX->GetBufferSize(),
		&g_VFXVertexLayout);

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
	g_ImmediateContext->VSSetConstantBuffers(0, 1, &g_WorldBuffer);
	g_ImmediateContext->PSSetConstantBuffers(0, 1, &g_WorldBuffer);

	//ビューマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(1, 1, &g_ViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(1, 1, &g_ViewBuffer);

	//プロジェクションマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ProjectionBuffer);
	g_ImmediateContext->VSSetConstantBuffers(2, 1, &g_ProjectionBuffer);
	g_ImmediateContext->PSSetConstantBuffers(2, 1, &g_ProjectionBuffer);

	//マテリアル情報
	hBufferDesc.ByteWidth = sizeof(MATERIAL_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_MaterialBuffer);
	g_ImmediateContext->VSSetConstantBuffers(3, 1, &g_MaterialBuffer);
	g_ImmediateContext->PSSetConstantBuffers(3, 1, &g_MaterialBuffer);

	//ライト情報
	hBufferDesc.ByteWidth = sizeof(LIGHT_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightBuffer);
	g_ImmediateContext->VSSetConstantBuffers(4, 1, &g_LightBuffer);
	g_ImmediateContext->PSSetConstantBuffers(4, 1, &g_LightBuffer);
	hBufferDesc.ByteWidth = sizeof(LightViewProjBuffer);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightProjViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(8, 1, &g_LightProjViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(8, 1, &g_LightProjViewBuffer);

	//フォグ情報
	hBufferDesc.ByteWidth = sizeof(FOG_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FogBuffer);
	g_ImmediateContext->VSSetConstantBuffers(5, 1, &g_FogBuffer);
	g_ImmediateContext->PSSetConstantBuffers(5, 1, &g_FogBuffer);

	//縁取り
	hBufferDesc.ByteWidth = sizeof(FUCHI);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FuchiBuffer);
	g_ImmediateContext->VSSetConstantBuffers(6, 1, &g_FuchiBuffer);
	g_ImmediateContext->PSSetConstantBuffers(6, 1, &g_FuchiBuffer);

	//カメラ
	hBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_CameraPosBuffer);
	g_ImmediateContext->VSSetConstantBuffers(7, 1, &g_CameraPosBuffer);
	g_ImmediateContext->PSSetConstantBuffers(7, 1, &g_CameraPosBuffer);

	hBufferDesc.ByteWidth = sizeof(BoneMatrices);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_BoneMatrixBuffer);

	hBufferDesc.ByteWidth = sizeof(LIGHTMODE_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightModeBuffer);
	g_ImmediateContext->VSSetConstantBuffers(10, 1, &g_LightModeBuffer);
	g_ImmediateContext->PSSetConstantBuffers(10, 1, &g_LightModeBuffer);

	hBufferDesc.ByteWidth = sizeof(RenderProgressBuffer);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_RenderProgressBuffer);
	g_ImmediateContext->VSSetConstantBuffers(13, 1, &g_RenderProgressBuffer);
	g_ImmediateContext->PSSetConstantBuffers(13, 1, &g_RenderProgressBuffer);

	// 入力レイアウト設定
	g_ImmediateContext->IASetInputLayout( g_VertexLayout );

	// シェーダ設定
	g_ImmediateContext->VSSetShader( g_VertexShader, NULL, 0 );
	g_ImmediateContext->PSSetShader( g_PixelShader, NULL, 0 );

	//ライト初期化
	ZeroMemory(&g_Light, sizeof(LIGHT_CBUFFER));
	g_Light.Direction[0] = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
	g_Light.Diffuse[0] = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	g_Light.Ambient[0] = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	g_Light.Flags[0].Type = LIGHT_TYPE_DIRECTIONAL;
	SetLightBuffer();


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
	if (g_DepthStateEnable)		g_DepthStateEnable->Release();
	if (g_DepthStateDisable)	g_DepthStateDisable->Release();
	if (g_BlendStateNone)		g_BlendStateNone->Release();
	if (g_BlendStateAlphaBlend)	g_BlendStateAlphaBlend->Release();
	if (g_BlendStateAdd)		g_BlendStateAdd->Release();
	if (g_BlendStateSubtract)	g_BlendStateSubtract->Release();
	if (g_RasterStateCullOff)	g_RasterStateCullOff->Release();
	if (g_RasterStateCullCW)	g_RasterStateCullCW->Release();
	if (g_RasterStateCullCCW)	g_RasterStateCullCCW->Release();
	if (g_RasterizerLayer0)		g_RasterizerLayer0->Release();
	if (g_RasterizerLayer1)		g_RasterizerLayer1->Release();
	if (g_RasterizerLayer2)		g_RasterizerLayer2->Release();
	if (g_RasterizerLayer3)		g_RasterizerLayer3->Release();

	if (g_WorldBuffer)				g_WorldBuffer->Release();
	if (g_ViewBuffer)				g_ViewBuffer->Release();
	if (g_ProjectionBuffer)			g_ProjectionBuffer->Release();
	if (g_MaterialBuffer)			g_MaterialBuffer->Release();
	if (g_LightBuffer)				g_LightBuffer->Release();
	if (g_FogBuffer)				g_FogBuffer->Release();
	if (g_FuchiBuffer)				g_FuchiBuffer->Release();
	if (g_CameraPosBuffer)			g_CameraPosBuffer->Release();
	if (g_LightProjViewBuffer)		g_LightProjViewBuffer->Release();
	if (g_BoneMatrixBuffer)			g_BoneMatrixBuffer->Release();
	if (g_LightModeBuffer)			g_LightModeBuffer->Release();
	if (g_RenderProgressBuffer)		g_RenderProgressBuffer->Release();


	if (g_VertexLayout)			g_VertexLayout->Release();
	if (g_VertexShader)			g_VertexShader->Release();
	if (g_DepthVertexShader)	g_DepthVertexShader->Release();
	if (g_PixelShader)			g_PixelShader->Release();

	if (g_SkinnedMeshVertexLayout)			g_SkinnedMeshVertexLayout->Release();
	if (g_SkinnedMeshVertexShader)			g_SkinnedMeshVertexShader->Release();
	if (g_SkinnedMeshPixelShader)			g_SkinnedMeshPixelShader->Release();
	if (g_DepthSkinnedMeshVertexShader)		g_DepthSkinnedMeshVertexShader->Release();

	if (g_VFXVertexShader)			g_VFXVertexShader->Release();
	if (g_VFXPixelShader)			g_VFXPixelShader->Release();
	if (g_VFXVertexLayout)			g_VFXVertexLayout->Release();

	if (g_ImmediateContext)		g_ImmediateContext->ClearState();
	if (g_RenderTargetView)		g_RenderTargetView->Release();
	if (g_SwapChain)			g_SwapChain->Release();
	if (g_ImmediateContext)		g_ImmediateContext->Release();
	if (g_D3DDevice)			g_D3DDevice->Release();

	for (int i = 0; i < LIGHT_MAX; i++)
		if (g_ShadowDSV[i])	g_ShadowDSV[i]->Release();
	if (g_SceneDepthStencilView) g_SceneDepthStencilView->Release();
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
