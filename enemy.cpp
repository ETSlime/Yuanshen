//=============================================================================
//
// �G�l�~�[���f������ [enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "enemy.h"
#include "shadow.h"
#include "camera.h"
#include "MapEditor.h"
#include "score.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/neko.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(5.0f)						// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// ��]��

#define ENEMY_SHADOW_SIZE	(1.1f)						// �e�̑傫��
#define ENEMY_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��
#define ENEMY_SCALE			(1.8f)

#define	TEXTURE_MAX			(2)

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static UISprite			g_HPGauge[MAX_ENEMY];

static ID3D11Buffer* g_VertexBuffer = NULL;	// ���_�o�b�t�@
static char* g_TextureName[] = {
	"data/TEXTURE/EnemyHPGauge.png",
	"data/TEXTURE/EnemyHPGauge_bg.png",
};
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����


//void UpdateEnemy(void)
//{
//	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
//	for (int i = 0; i < MAX_ENEMY; i++)
//	{
//		
//
//		if (g_Enemy[i].use == true)		// ���̃G�l�~�[���g���Ă���H
//		{								// Yes
//			UpdateEditorSelect(&g_Enemy[i], GetMousePosX(), GetMousePosY());
//				// �ړ�����
//			if (g_Enemy[i].tblMax > 0 && g_Enemy[i].pModel->GetIsModelSelected() == FALSE)	// ���`��Ԃ����s����H
//			{	// ���`��Ԃ̏���
//				int nowNo = (int)g_Enemy[i].time;			// �������ł���e�[�u���ԍ������o���Ă���
//				int maxNo = g_Enemy[i].tblMax;				// �o�^�e�[�u�����𐔂��Ă���
//				int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
//				INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Enemy[i].tblNo];	// �s���e�[�u���̃A�h���X���擾
//
//				XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
//				XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
//				XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�
//
//				XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
//				XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
//				XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���
//
//				float nowTime = g_Enemy[i].time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���
//
//				Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
//				Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
//				Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���
//
//				// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
//				float oldY = g_Enemy[i].pos.y;
//				XMStoreFloat3(&g_Enemy[i].pos, nowPos + Pos);
//				g_Enemy[i].pos.y += oldY;
//
//				// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
//				XMStoreFloat3(&g_Enemy[i].rot, nowRot + Rot);
//
//				// �v�Z���ċ��߂��g�嗦�����݂̈ړ��e�[�u���ɑ����Ă���
//				XMStoreFloat3(&g_Enemy[i].scl, nowScl + Scl);
//
//				XMVECTOR direction = XMVectorSubtract(nowPos, Pos);
//				direction = XMVector3Normalize(direction);
//				g_Enemy[i].targetDir = atan2(XMVectorGetZ(direction), XMVectorGetX(direction));
//				g_Enemy[i].targetDir += XM_PI / 2;
//				// frame���g�Ď��Ԍo�ߏ���������
//				g_Enemy[i].time += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
//				if ((int)g_Enemy[i].time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
//				{
//					g_Enemy[i].time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
//				}
//			}
//
//			float deltaDir = g_Enemy[i].targetDir - g_Enemy[i].dir;
//			if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
//			if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
//			g_Enemy[i].dir += deltaDir * ROTATION_SPEED;
//			if (i != 4)
//				g_Enemy[i].rot.y = g_Enemy[i].dir;
//
//			PlayEnemyWalkAnim(&g_Enemy[i]);
//			if (g_Enemy[i].state == ENEMY_WALK)
//			{
//				PlayEnemyWalkAnim(&g_Enemy[i]);
//			}
//
//			XMFLOAT3 pos = g_Enemy[i].pos;
//			pos.y = (-40.0f - ENEMY_OFFSET_Y - 0.1f);
//			SetPositionShadow(g_Enemy[i].shadowIdx, pos);
//
//			UpdateHPGauge(i);
//
//			if (g_Enemy[i].HP <= 0.0f)
//			{
//				g_Enemy[i].use = FALSE;
//				AddScore(100);
//			}
//				
//
//		}
//	}
//
//
//
//
//#ifdef _DEBUG
//
//	//if (GetKeyboardTrigger(DIK_P))
//	//{
//	//	// ���f���̐F��ύX�ł����I�������ɂ��ł����B
//	//	for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
//	//	{
//	//		SetModelDiffuse(&g_Enemy[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
//	//	}
//	//}
//
//	//if (GetKeyboardTrigger(DIK_L))
//	//{
//	//	// ���f���̐F�����ɖ߂��Ă���
//	//	for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
//	//	{
//	//		SetModelDiffuse(&g_Enemy[0].model, j, g_Enemy[0].diffuse[j]);
//	//	}
//	//}
//
//	if (GetKeyboardTrigger(DIK_P))
//	{
//		g_Enemy[1].state = ENEMY_WALK;
//	}
//#endif
//
//	
//
//}

void Enemy::UpdateEditorSelect(int sx, int sy)
{
	if (MapEditor::get_instance().GetOnEditorCursor() == TRUE)
	{
		return;
	}
		
	this->SetIsCursorIn(FALSE);

	CAMERA* camera = GetCamera();
	XMMATRIX P = XMLoadFloat4x4(&camera->mtxProjection);

	// Compute picking ray in view space.
	float vx = (+2.0f * sx / SCREEN_WIDTH - 1.0f) / P.r[0].m128_f32[0];
	float vy = (-2.0f * sy / SCREEN_HEIGHT + 1.0f) / P.r[1].m128_f32[1];

	// Ray definition in view space.
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	// Tranform ray to local space of Mesh.
	XMMATRIX V = XMLoadFloat4x4(&camera->mtxView);
	XMMATRIX invView = XMLoadFloat4x4(&camera->mtxInvView);

	XMMATRIX W = instance.transform.mtxWorld;
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// Make the ray direction unit length for the intersection tests.
	rayDir = XMVector3Normalize(rayDir);
	
	XMVECTOR minPoint = XMLoadFloat3(&instance.pModel->GetBondingBox().minPoint);
	XMVECTOR maxPoint = XMLoadFloat3(&instance.pModel->GetBondingBox().maxPoint);


	float tMin = 0.0f;
	float tMax = FLT_MAX;
	BOOL intersect = FALSE;
	for (int i = 0; i < 3; ++i) 
	{
		float rayOriginComponent = XMVectorGetByIndex(rayOrigin, i);
		float rayDirComponent = XMVectorGetByIndex(rayDir, i);
		float minComponent = XMVectorGetByIndex(minPoint, i);
		float maxComponent = XMVectorGetByIndex(maxPoint, i);

		if (fabs(rayDirComponent) < 1e-8f) {
			if (rayOriginComponent < minComponent || rayOriginComponent > maxComponent) 
			{
				intersect =  FALSE;
				return;
			}
		}
		else 
		{
			float t1 = (minComponent - rayOriginComponent) / rayDirComponent;
			float t2 = (maxComponent - rayOriginComponent) / rayDirComponent;

			if (t1 > t2) 
			{
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			tMin = max(tMin, t1);
			tMax = min(tMax, t2);

			if (tMin > tMax) 
			{
				intersect = FALSE;
				return;
			}
		}
	}

	this->SetIsCursorIn(TRUE);


	this->UpdateModelEditor();

	PrintDebugProc("Enemy:X:%f Y:%f Z:%f\n", instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);

}

//void UpdateHPGauge(int idx)
//{
//	g_HPGauge[idx].pos = g_Enemy[idx].pos;
//	g_HPGauge[idx].pos.y += ENEMY_SIZE;
//	g_HPGauge[idx].pos.x -= 15.0f;
//}

void Enemy::PlayEnemyWalkAnim(void)
{
	if (this->GetIsModelSelected() == TRUE) return;

	float angle = (XM_PI / JUMP_CNT_MAX) * instance.jumpCnt;
	instance.jumpCnt++;

	if (instance.jumpUp == FALSE)
	{
		instance.transform.scl.y += 0.008f;
		instance.transform.pos.y += instance.jumpYMax * cosf(angle);
		//SetShadowSize(enemy->shadowIdx, enemy->shadowSize, enemy->shadowSize);
	}
	else
	{
		instance.transform.scl.y -= 0.008f;
		instance.transform.pos.y -= instance.jumpYMax * cosf(angle);
		//SetShadowSize(enemy->shadowIdx, enemy->shadowSize, enemy->shadowSize);
	}

	if (instance.jumpCnt > JUMP_CNT_MAX * 0.5f)
	{
		instance.jumpUp = !instance.jumpUp;
		instance.jumpCnt = 0;
	}
}

//void DrawHPGauge(int idx)
//{
//	// ���C�e�B���O�𖳌�
//	SetLightEnable(FALSE);
//
//	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
//	CAMERA* cam = GetCamera();
//
//	// ���_�o�b�t�@�ݒ�
//	UINT stride = sizeof(VERTEX_3D);
//	UINT offset = 0;
//	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
//
//	// �v���~�e�B�u�g�|���W�ݒ�
//	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	if (g_HPGauge[idx].bUse)
//	{
//		// ���[���h�}�g���b�N�X�̏�����
//		mtxWorld = XMMatrixIdentity();
//
//		// �r���[�}�g���b�N�X���擾
//		mtxView = XMLoadFloat4x4(&cam->mtxView);
//
//
//		// �Ȃɂ�����Ƃ���
//		// �����s��i�����s��j��]�u�s�񂳂��ċt�s�������Ă��(����)
//		mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
//		mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
//		mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];
//
//		mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
//		mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
//		mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];
//
//		mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
//		mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
//		mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];
//
//		int ratio = g_Enemy[idx].HP / g_Enemy[idx].maxHP;
//		//MakeVertexHPGauge(15.0f * ratio, 5.0f);
//
//		// �X�P�[���𔽉f
//		mtxScl = XMMatrixScaling(g_HPGauge[idx].scl.x, g_HPGauge[idx].scl.y, g_HPGauge[idx].scl.z);
//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);
//
//		// �ړ��𔽉f
//		mtxTranslate = XMMatrixTranslation(g_HPGauge[idx].pos.x, g_HPGauge[idx].pos.y, g_HPGauge[idx].pos.z);
//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);
//
//		// ���[���h�}�g���b�N�X�̐ݒ�
//		SetCurrentWorldMatrix(&mtxWorld);
//
//
//		// �}�e���A���ݒ�
//		SetMaterial(g_HPGauge[idx].material);
//
//		// �e�N�X�`���ݒ�
//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
//
//		// �|���S���̕`��
//		GetDeviceContext()->Draw(4, 0);
//	}
//
//	// ���C�e�B���O��L����
//	SetLightEnable(TRUE);
//}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexHPGauge(int width, int height)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = width;
	float fHeight = height;

	// ���_���W�̐ݒ�
	//vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	//vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	//vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	//vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);
	vertex[0].Position = XMFLOAT3(0.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth, 0.0f, 0.0f);

	// �@���̐ݒ�
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

//=============================================================================
// ����������
//=============================================================================
Enemy::Enemy(char* enemyPath, Transform trans)
{
	Instantiate(enemyPath);
	MapEditor::get_instance().AddToList(this);

	instance.load = true;

	instance.attribute.maxHP = 20.0f;
	instance.attribute.HP = 20.0f;

	instance.attribute.spd = 0.0f;			// �ړ��X�s�[�h�N���A
	instance.attribute.size = ENEMY_SIZE;	// �����蔻��̑傫��

	instance.transform.pos = trans.pos;
	instance.transform.rot = trans.rot;
	instance.transform.scl = trans.scl;

	instance.moveTbl = nullptr;


	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	instance.pModel->GetModelDiffuse(&instance.diffuse[0]);


	instance.jumpUp = FALSE;
	instance.jumpCnt = 0;
	instance.jumpYMax = 0.9f;

	instance.move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

	instance.time = 0.0f;			// ���`��ԗp�̃^�C�}�[���N���A
	instance.tblMax = 0;			// �Đ�����s���f�[�^�e�[�u���̃��R�[�h�����Z�b�g


	instance.use = true;		// true:�����Ă�
}

//=============================================================================
// �X�V����
//=============================================================================
void Enemy::Update(void)
{
	if (instance.use == true)		// ���̃G�l�~�[���g���Ă���H
	{								// Yes
		GameObject::Update();

		UpdateEditorSelect(GetMousePosX(), GetMousePosY());
		// �ړ�����
		BOOL isSelected = this->GetIsModelSelected();
		if (instance.tblMax > 0 && isSelected == FALSE)	// ���`��Ԃ����s����H
		{	// ���`��Ԃ̏���
			int nowNo = (int)instance.time;			// �������ł���e�[�u���ԍ������o���Ă���
			int maxNo = instance.tblMax;				// �o�^�e�[�u�����𐔂��Ă���
			int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
			INTERPOLATION_DATA* tbl = instance.moveTbl;//g_MoveTblAdr[instance.tblNo];	// �s���e�[�u���̃A�h���X���擾

			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�

			XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
			XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
			XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���

			float nowTime = instance.time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���

			Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
			Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
			Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���

			// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
			float oldY = instance.transform.pos.y;
			XMStoreFloat3(&instance.transform.pos, nowPos + Pos);
			instance.transform.pos.y += oldY;

			// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
			XMStoreFloat3(&instance.transform.rot, nowRot + Rot);

			// �v�Z���ċ��߂��g�嗦�����݂̈ړ��e�[�u���ɑ����Ă���
			XMStoreFloat3(&instance.transform.scl, nowScl + Scl);

			XMVECTOR direction = XMVectorSubtract(nowPos, Pos);
			direction = XMVector3Normalize(direction);
			instance.targetDir = atan2(XMVectorGetZ(direction), XMVectorGetX(direction));
			instance.targetDir += XM_PI / 2;
			// frame���g�Ď��Ԍo�ߏ���������
			instance.time += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
			if ((int)instance.time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
			{
				instance.time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
			}
		}

		float deltaDir = instance.targetDir - instance.dir;
		if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
		if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
		instance.dir += deltaDir * ROTATION_SPEED;
		//if (i != 4)
		//	instance.rot.y = g_Enemy[i].dir;

		PlayEnemyWalkAnim();
		if (instance.state == ENEMY_WALK)
		{
			PlayEnemyWalkAnim();
		}

		XMFLOAT3 pos = instance.transform.pos;
		pos.y = (-40.0f - ENEMY_OFFSET_Y - 0.1f);
		//SetPositionShadow(g_Enemy[i].shadowIdx, pos);

		//UpdateHPGauge(i);

		if (instance.attribute.HP <= 0.0f)
		{
			instance.use = FALSE;
			AddScore(100);
		}

	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void Enemy::Draw(void)
{
	// ���[���h�}�g���b�N�X�̐ݒ�
	SetCurrentWorldMatrix(&instance.transform.mtxWorld);

	// ���f���`��
	this->DrawModelEditor();
	instance.pModel->DrawBoundingBox();
}

void Enemy::SetMoveTbl(MoveTable moveTbl)
{
	instance.moveTbl = moveTbl.moveTblAddr;
	instance.tblMax = moveTbl.size;
}

EnemyManager::EnemyManager()
{

}

void EnemyManager::Init()
{
	InitializeMoveTbl();
	for (int i = 0; i < 4; i++)
	{
		Transform trans;
		trans.pos = XMFLOAT3(-50.0f + i * 30.0f, -70.0f, 20.0f);
		trans.scl = XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE);
		SpawnEnemy(MODEL_ENEMY, trans, moveTbls[i]);
	}
}



void EnemyManager::SpawnEnemy(char* enemyPath, Transform trans, MoveTable moveTbl)
{
	Enemy* enemy = new Enemy(enemyPath, trans);
	enemy->SetMoveTbl(moveTbl);
	enemyList.push_back(enemy);
}

void EnemyManager::Draw(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		// ���f���`��
		cur->data->Draw();
		cur = cur->next;
	}

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

void EnemyManager::Update(void)
{
	Node<Enemy*>* cur = enemyList.getHead();
	while (cur != nullptr)
	{
		cur->data->Update();
		if (cur->data->GetUse() == FALSE)
		{
			Node<Enemy*>* toDelete = cur;
			cur = cur->next;
			delete toDelete->data;
			enemyList.remove(toDelete);
		}
		else
			cur = cur->next;
	}
}

void EnemyManager::InitializeMoveTbl()
{
	static INTERPOLATION_DATA moveTbl[] = {
		//���W									��]��							�g�嗦					����
		{ XMFLOAT3(50.0f,  0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(250.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 }
	};
	moveTbls.push_back({ moveTbl , sizeof(moveTbl) / sizeof(INTERPOLATION_DATA) });

	static INTERPOLATION_DATA moveTbl2[] = {
		//���W									��]��							�g�嗦							����
		{ XMFLOAT3(450.0f,  0.0f, 55.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(250.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
	};

	moveTbls.push_back({ moveTbl2 , sizeof(moveTbl2) / sizeof(INTERPOLATION_DATA) });

	static INTERPOLATION_DATA moveTbl3[] = {
		//���W									��]��							�g�嗦							����
		{ XMFLOAT3(50.0f,  0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(250.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
	};

	moveTbls.push_back({ moveTbl3 , sizeof(moveTbl3) / sizeof(INTERPOLATION_DATA) });

	static INTERPOLATION_DATA moveTbl4[] = {
		//���W									��]��							�g�嗦							����
		{ XMFLOAT3(471.0f,  0.0f, -437.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
		{ XMFLOAT3(752.0f, 0.0f, 742.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE),	600 },
	};
	moveTbls.push_back({ moveTbl4 , sizeof(moveTbl4) / sizeof(INTERPOLATION_DATA) });
}

