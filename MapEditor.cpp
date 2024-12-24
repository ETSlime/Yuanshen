//=============================================================================
//
// MapEditor処理 [MapEditor.cpp]
// Author : 
//
//=============================================================================
#include "MapEditor.h"
#include "enemy.h"
#include "debugproc.h"
#include "camera.h"
#include "input.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_POS_CUROSR			"data/MODEL/posCursor.obj"		// 読み込むモデル名

MapEditor::MapEditor()
{
	curModelCnt = 0;
	curSelectedModelIdx = -1;
	onEditorCursor = FALSE;
	posCursor.cursorX = new CursorInstance();
	posCursor.cursorY = new CursorInstance();
	posCursor.cursorZ = new CursorInstance();
}

void MapEditor::Init()
{
	posCursor.cursorX->Instantiate(MODEL_POS_CUROSR);
	posCursor.cursorY->Instantiate(MODEL_POS_CUROSR);
	posCursor.cursorZ->Instantiate(MODEL_POS_CUROSR);

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	material.noTexSampling = TRUE;
	posCursor.cursorX->SetMaterial(material);

	material.Diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	posCursor.cursorY->SetMaterial(material);
	material.Diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	posCursor.cursorZ->SetMaterial(material);

	//XMStoreFloat4x4(&posCursor.cursorX.mtxWorld, XMMatrixIdentity());
	//XMStoreFloat4x4(&posCursor.cursorY.mtxWorld, XMMatrixIdentity());
	//XMStoreFloat4x4(&posCursor.cursorZ.mtxWorld, XMMatrixIdentity());
}

void MapEditor::Update()
{
	UpdateMouseDrag();

	BOOL isOnEditorCursor = FALSE;
	isOnEditorCursor |= UpdateEditorSelect(posCursor.cursorX, GetMousePosX(), GetMousePosY());
	isOnEditorCursor |= UpdateEditorSelect(posCursor.cursorY, GetMousePosX(), GetMousePosY());
	isOnEditorCursor |= UpdateEditorSelect(posCursor.cursorZ, GetMousePosX(), GetMousePosY());
	onEditorCursor = isOnEditorCursor;

	if (deltaX != 0 || deltaY != 0)
	{
		PrintDebugProc("deltaX:%f deltaY:%f\n", deltaX, deltaY);
	}

	const DoubleLinkedList<Enemy*>* enemyList = EnemyManager::get_instance().GetEnemy();
	

	if (posCursor.cursorX->GetIsDragging() == TRUE)
	{
		Node<Enemy*>* cur = enemyList->getHead();
		while (cur != nullptr)
		{
			if (cur->data->GetEditorIndex() == curSelectedModelIdx)
			{
				Transform trans = cur->data->GetTransform();
				trans.pos.x += deltaX;
				cur->data->SetPosition(trans.pos);
				PrintDebugProc("enemy PosX:%f", cur->data->GetTransform().pos.x);
			}
			cur = cur->next;
		}
	}
	else if (posCursor.cursorY->GetIsDragging() == TRUE)
	{
		Node<Enemy*>* cur = enemyList->getHead();
		while (cur != nullptr)
		{
			if (cur->data->GetEditorIndex() == curSelectedModelIdx)
			{
				Transform trans = cur->data->GetTransform();
				trans.pos.y -= deltaY;
				cur->data->SetPosition(trans.pos);
				PrintDebugProc("enemy PosX:%f", cur->data->GetTransform().pos.x);
			}
			cur = cur->next;
		}
	}
	else if (posCursor.cursorZ->GetIsDragging() == TRUE)
	{

		Node<Enemy*>* cur = enemyList->getHead();
		while (cur != nullptr)
		{
			if (cur->data->GetEditorIndex() == curSelectedModelIdx)
			{
				Transform trans = cur->data->GetTransform();
				trans.pos.z -= deltaX;
				cur->data->SetPosition(trans.pos);
				PrintDebugProc("enemy PosX:%f", cur->data->GetTransform().pos.x);
			}
			cur = cur->next;
		}
	}
}

void MapEditor::UpdateMouseDrag()
{

	if (IsMouseLeftTriggered()) 
	{
		if (posCursor.cursorX->GetIsCursorIn() == TRUE)
		{
			posCursor.cursorX->SetIsDragging(TRUE);
			startX = GetMousePosX();
			startY = GetMousePosY();
			currentX = startX;
			currentY = startY;
			deltaX = 0;
			deltaY = 0;
		}
		else if (posCursor.cursorY->GetIsCursorIn() == TRUE)
		{
			posCursor.cursorY->SetIsDragging(TRUE);
			startX = GetMousePosX();
			startY = GetMousePosY();
			currentX = startX;
			currentY = startY;
			deltaX = 0;
			deltaY = 0;
		}
		else if (posCursor.cursorZ->GetIsCursorIn() == TRUE)
		{
			posCursor.cursorZ->SetIsDragging(TRUE);
			startX = GetMousePosX();
			startY = GetMousePosY();
			currentX = startX;
			currentY = startY;
			deltaX = 0;
			deltaY = 0;
		}

	}
	else if (IsMouseLeftPressed()) 
	{
		if (posCursor.cursorX->GetIsDragging() == TRUE ||
			posCursor.cursorY->GetIsDragging() == TRUE ||
			posCursor.cursorZ->GetIsDragging() == TRUE)
		{
			long newX = GetMousePosX();
			long newY = GetMousePosY();
			deltaX = newX - currentX;
			deltaY = newY - currentY;
			currentX = newX;
			currentY = newY;
		}

	}
	else if (!IsMouseLeftPressed()) 
	{
		posCursor.cursorX->SetIsDragging(FALSE);
		posCursor.cursorY->SetIsDragging(FALSE);
		posCursor.cursorZ->SetIsDragging(FALSE);
	}
}

void MapEditor::Draw()
{
	if (GetRenderMode() == RENDER_MODE_SHADOW) return;

	Model* model = GetCurSelectedModel();

	if (model)
	{
		SetDepthEnable(FALSE);

		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// カリング無効
		SetCullingMode(CULL_MODE_NONE);

		const DoubleLinkedList<Enemy*>* enemyList = EnemyManager::get_instance().GetEnemy();
		Node<Enemy*>* cur = enemyList->getHead();
		while (cur != nullptr)
		{
			if (cur->data->GetEditorIndex() == curSelectedModelIdx)
			{

				XMVECTOR minPoint = XMLoadFloat3(&cur->data->GetModel()->GetBondingBox().minPoint);
				XMVECTOR maxPoint = XMLoadFloat3(&cur->data->GetModel()->GetBondingBox().maxPoint);
				XMMATRIX worldMatrix = cur->data->GetTransform().mtxWorld;
				XMVECTOR corners[8];
				corners[0] = XMVectorSet(minPoint.m128_f32[0], minPoint.m128_f32[1], minPoint.m128_f32[2], 1.0f);
				corners[1] = XMVectorSet(maxPoint.m128_f32[0], minPoint.m128_f32[1], minPoint.m128_f32[2], 1.0f);
				corners[2] = XMVectorSet(minPoint.m128_f32[0], maxPoint.m128_f32[1], minPoint.m128_f32[2], 1.0f);
				corners[3] = XMVectorSet(maxPoint.m128_f32[0], maxPoint.m128_f32[1], minPoint.m128_f32[2], 1.0f);
				corners[4] = XMVectorSet(minPoint.m128_f32[0], minPoint.m128_f32[1], maxPoint.m128_f32[2], 1.0f);
				corners[5] = XMVectorSet(maxPoint.m128_f32[0], minPoint.m128_f32[1], maxPoint.m128_f32[2], 1.0f);
				corners[6] = XMVectorSet(minPoint.m128_f32[0], maxPoint.m128_f32[1], maxPoint.m128_f32[2], 1.0f);
				corners[7] = XMVectorSet(maxPoint.m128_f32[0], maxPoint.m128_f32[1], maxPoint.m128_f32[2], 1.0f);

				XMVECTOR minWorld = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
				XMVECTOR maxWorld = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

				for (int i = 0; i < 8; ++i) {
					XMVECTOR worldCorner = XMVector3TransformCoord(corners[i], worldMatrix);
					minWorld = XMVectorMin(minWorld, worldCorner);
					maxWorld = XMVectorMax(maxWorld, worldCorner);
				}
				XMVECTOR centerWorld = XMVectorScale(XMVectorAdd(minWorld, maxWorld), 0.5f);

				CAMERA* camera = GetCamera();
				XMFLOAT3 worldPosition = XMFLOAT3(cur->data->GetTransform().pos.x, cur->data->GetTransform().pos.y, cur->data->GetTransform().pos.z);
				XMVECTOR modelPosition = XMLoadFloat3(&worldPosition);
				XMVECTOR cameraPosition = XMLoadFloat3(&camera->pos);
				XMVECTOR distanceVector = XMVectorSubtract(modelPosition, cameraPosition);
				float cameraDistance = XMVectorGetX(XMVector3Length(distanceVector));

				float projectionFactor = SCREEN_HEIGHT / tanf(camera->fov * 0.5f) * 30;

				float desiredScreenSize = 100.0f;
				float modelScale = desiredScreenSize * (cameraDistance / projectionFactor);
				mtxScl = XMMatrixScaling(modelScale, modelScale, modelScale);

				// 平移向量を追加（例：Y方向に5単位平移）
				XMVECTOR offset = XMVectorSet(0.0f, 15.0f, 0.0f, 0.0f);
				XMVECTOR newCenterWorld = XMVectorAdd(centerWorld, offset);

				mtxTranslate = XMMatrixTranslationFromVector(newCenterWorld);

				// スケールを反映
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

				// 回転を反映
				mtxRot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

				// 移動を反映
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);


				// ワールドマトリックスの設定
				SetCurrentWorldMatrix(&mtxWorld);
				posCursor.cursorY->SetWorldMatrix(mtxWorld);
				if (posCursor.cursorY->GetIsCursorIn() == TRUE)
				{
					MATERIAL material;
					ZeroMemory(&material, sizeof(material));
					material.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
					material.noTexSampling = TRUE;
					SetMaterial(material);
				}
				else
					SetMaterial(posCursor.cursorY->GetMaterial());
				posCursor.cursorY->GetModel()->DrawModel();

				offset = XMVectorSet(15.0f, 0.0f, 0.0f, 0.0f);
				newCenterWorld = XMVectorAdd(centerWorld, offset);

				mtxTranslate = XMMatrixTranslationFromVector(newCenterWorld);

				mtxWorld = XMMatrixIdentity();
				// スケールを反映
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

				// 回転を反映
				mtxRot = XMMatrixRotationRollPitchYaw(-XM_PI * 0.5f, 0.0f, 0.0f);
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

				// 移動を反映
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

				SetCurrentWorldMatrix(&mtxWorld);
				posCursor.cursorZ->SetWorldMatrix(mtxWorld);
				if (posCursor.cursorZ->GetIsCursorIn() == TRUE)
				{
					MATERIAL material;
					ZeroMemory(&material, sizeof(material));
					material.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
					material.noTexSampling = TRUE;
					SetMaterial(material);
				}
				else
					SetMaterial(posCursor.cursorZ->GetMaterial());
				posCursor.cursorZ->GetModel()->DrawModel();
				

				offset = XMVectorSet(0.0f, 0.0f, 15.0f, 0.0f);
				newCenterWorld = XMVectorAdd(centerWorld, offset);

				mtxTranslate = XMMatrixTranslationFromVector(newCenterWorld);

				mtxWorld = XMMatrixIdentity();
				// スケールを反映
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

				// 回転を反映
				mtxRot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, XM_PI * 0.5f);
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

				// 移動を反映
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

				SetCurrentWorldMatrix(&mtxWorld);
				posCursor.cursorX->SetWorldMatrix(mtxWorld);
				if (posCursor.cursorX->GetIsCursorIn() == TRUE)
				{
					MATERIAL material;
					ZeroMemory(&material, sizeof(material));
					material.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
					material.noTexSampling = TRUE;
					SetMaterial(material);
				}
				else
					SetMaterial(posCursor.cursorX->GetMaterial());
				posCursor.cursorX->GetModel()->DrawModel();
				break;
			}
			cur = cur->next;
		}

		SetDepthEnable(TRUE);
	}
}

void MapEditor::Uninit()
{
	delete posCursor.cursorX;
	delete posCursor.cursorY;
	delete posCursor.cursorZ;
}

BOOL MapEditor::UpdateEditorSelect(CursorInstance* cursor, int sx, int sy)
{
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

	XMMATRIX W = cursor->GetTransform().mtxWorld;
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// Make the ray direction unit length for the intersection tests.
	rayDir = XMVector3Normalize(rayDir);

	XMVECTOR minPoint = XMLoadFloat3(&cursor->GetModel()->GetBondingBox().minPoint);
	XMVECTOR maxPoint = XMLoadFloat3(&cursor->GetModel()->GetBondingBox().maxPoint);


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
				intersect = FALSE;
				cursor->SetIsCursorIn(FALSE);
				return FALSE;
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
				cursor->SetIsCursorIn(FALSE);
				return FALSE;
			}
		}
	}

	cursor->SetIsCursorIn(TRUE);
	return TRUE;
}

Model* MapEditor::GetCurSelectedModel()
{
	if (curSelectedModelIdx == -1 || curSelectedModelIdx >= MAX_MODEL)
		return nullptr;

	return modelList[curSelectedModelIdx];
}

