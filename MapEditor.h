#pragma once

#include "model.h"
#include "SingletonBase.h"
#include "GameObject.h"
#include "enemy.h"
#define MAX_MODEL	500

//*****************************************************************************
// ç\ë¢ëÃíËã`
//*****************************************************************************
class CursorInstance : public GameObject<ModelInstance>
{
public:
	inline void SetMaterial(MATERIAL material) { this->material = material; }
	inline MATERIAL GetMaterial(void) { return material; }
	inline BOOL GetIsDragging() { return isDragging; }
	inline void SetIsDragging(BOOL isDragging) { this->isDragging = isDragging; }
private:
	MATERIAL			material;
	BOOL				isDragging;
};

struct Cursor
{
	CursorInstance*		cursorX;
	CursorInstance*		cursorY;
	CursorInstance*		cursorZ;
};

class MapEditor : public SingletonBase<MapEditor>
{
public:
	MapEditor();
	void Init();
	void Update();
	void Draw();
	void Uninit();

	template <typename T>
	int	 AddToList(GameObject<T>* GO);

	void SetCurSelectedModelIdx(int idx) { curSelectedModelIdx = idx; }
	void ResetCurSelectedModelIdx() { curSelectedModelIdx = -1; }
	BOOL GetOnEditorCursor()  { return onEditorCursor; }
private:
	BOOL UpdateEditorSelect(CursorInstance* cursor, int sx, int sy);
	Model* GetCurSelectedModel();
	void UpdateMouseDrag();
	Cursor posCursor, scaleCursor, rotateCursor;
	Model* modelList[MAX_MODEL];
	int	curModelCnt;
	int	curSelectedModelIdx;
	BOOL onEditorCursor;

	BOOL isDragging = FALSE;
	long startX = 0, startY = 0;
	long currentX = 0, currentY = 0;
	long deltaX = 0, deltaY = 0;
	
	XMFLOAT4 originDraggingRay = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 startDraggingRay = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 endDraggingRay = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
};

template <typename T>
int MapEditor::AddToList(GameObject<T>* GO)
{
	if (curModelCnt >= MAX_MODEL)
		return -1;

	modelList[curModelCnt] = GO->GetModel();
	GO->SetEditorIndex(curModelCnt);
	curModelCnt++;

	return curModelCnt;
}
