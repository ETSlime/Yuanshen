#include "SkinnedMeshModel.h"

void SkinnedMeshModel::Draw()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(10.f, 10.f, 10.f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	SetCurrentWorldMatrix(&mtxWorld);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &this->VertexBuffer, &stride, &offset);

	// �C���f�b�N�X�o�b�t�@�ݒ�
	GetDeviceContext()->IASetIndexBuffer(this->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// �v���~�e�B�u�g�|���W�ݒ�
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
	// �|���S���`��
	GetDeviceContext()->DrawIndexed(this->modelData->IndexNum, 0, 0);
	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}
