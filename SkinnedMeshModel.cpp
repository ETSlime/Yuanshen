#include "SkinnedMeshModel.h"

void SkinnedMeshModel::Draw()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(10.f, 10.f, 10.f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	SetCurrentWorldMatrix(&mtxWorld);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &this->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	GetDeviceContext()->IASetIndexBuffer(this->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GetDeviceContext()->PSSetShaderResources(0, 1, &this->Texture);

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	if (this->Texture)
		material.noTexSampling = FALSE;
	else
		material.noTexSampling = TRUE;
	SetMaterial(material);

	SetFillMode(D3D11_FILL_WIREFRAME);
	SetCullingMode(CULL_MODE_NONE);
	// ポリゴン描画
	GetDeviceContext()->DrawIndexed(this->modelData->IndexNum, 0, 0);
	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}
