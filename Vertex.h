#pragma once

#include "renderer.h"


#define MAX_POLYGON					(6)						// キューブ１個あたりの面数
#define	SIZE_WH						(2.0f)				// 地面のサイズ
#define SIZE_FACE					(SIZE_WH * 7)

#define MAX_COLOR					(50)

#define COLOR_PINK					XMFLOAT4(1.0f, 0.6941f, 0.8745f, 1.0f)
#define COLOR_PINK_2				XMFLOAT4(0.97f, 0.6641f, 0.8445f, 1.0f)
#define COLOR_PINK_3				XMFLOAT4(0.94f, 0.6341f, 0.8145f, 1.0f)
#define COLOR_PINK_0				XMFLOAT4(0.9843f, 0.6784f, 0.8745f, 1.0f)
#define COLOR_PINK_FACE				XMFLOAT4(1.0f, 0.8471f, 0.9098f, 1.0f)
#define COLOR_SKIN					XMFLOAT4(0.9843f, 0.9176f, 0.8745f, 1.0f)
#define COLOR_CLOTH					XMFLOAT4(0.85f, 0.7f, 0.55f, 1.0f)
#define COLOR_BODY					XMFLOAT4(0.05f, 0.05f, 0.15f, 1.0f)
#define COLOR_BLUE_0				XMFLOAT4(0.6f, 0.7373f, 1.0f, 1.0f)
#define COLOR_BLUE					XMFLOAT4(0.6f, 0.776f, 1.0f, 1.0f)
#define COLOR_BLUE_2				XMFLOAT4(0.57f, 0.746f, 0.97f, 1.0f)
#define COLOR_BLUE_3				XMFLOAT4(0.54f, 0.716f, 0.94f, 1.0f)
#define COLOR_BLUE_4				XMFLOAT4(0.851f, 0.9529f, 1.0f, 1.0f)
#define COLOR_PURPLE				XMFLOAT4(0.627f, 0.125f, 0.941f, 1.0f)
#define COLOR_PURPLE_0				XMFLOAT4(0.8824f, 0.6f, 1.0f, 1.0f)
#define COLOR_BLACK					XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f)
#define COLOR_WHITE                 XMFLOAT4(0.9725f, 0.9333f, 0.8118f, 1.0f)
#define COLOR_WHITE_2               XMFLOAT4(0.9525f, 0.9133f, 0.7918f, 1.0f)
#define COLOR_WHITE_3               XMFLOAT4(0.9325f, 0.8933f, 0.7718f, 1.0f)
#define COLOR_WHITE_4               XMFLOAT4(0.9125f, 0.8733f, 0.7518f, 1.0f)
#define COLOR_WHITE_5               XMFLOAT4(0.8925f, 0.8533f, 0.7318f, 1.0f)
#define COLOR_WHITE_6               XMFLOAT4(0.8725f, 0.8333f, 0.7118f, 1.0f)
#define COLOR_WHITE_7               XMFLOAT4(0.8625f, 0.8233f, 0.7018f, 1.0f)
#define COLOR_WHITE_8               XMFLOAT4(0.8525f, 0.8133f, 0.6918f, 1.0f)
#define COLOR_GREY					XMFLOAT4(0.7f, 0.7f, 0.76f, 1.0f)
#define COLOR_GREY_2				XMFLOAT4(0.68f, 0.68f, 0.74f, 1.0f)
#define COLOR_GREY_3                XMFLOAT4(0.66f, 0.66f, 0.72f, 1.0f)
#define COLOR_RED					XMFLOAT4(0.8902f, 0.2745f, 0.349f, 1.0f)
#define COLOR_RIBBON				XMFLOAT4(1.0f, 0.9882f, 0.8627f, 1.0f)
#define COLOR_WAND_RED				XMFLOAT4(0.9804f, 0.3608f, 0.4588f, 1.0f)
#define COLOR_WAND_PINK				XMFLOAT4(1.0f, 0.8157f, 0.9674f, 1.0f)
#define COLOR_WAND_BLUE				XMFLOAT4(0.8157f, 0.8941f, 1.0f, 1.0f)
#define COLOR_WAND_PURPLE			XMFLOAT4(0.9333f, 0.8157f, 1.0f, 1.0f)

enum
{
	FACE,
	PINK,
	PINK0,
	PINK2,
	PINK3,
	PINK_FACE,
	BODY_COLOR,
	CLOTH_COLOR,
	BLUE,
	BLUE0,
	BLUE2,
	BLUE3,
	BLUE4,
	PURPLE,
	PURPLE0,
	BLACK,
	WHITE,
	WHITE2,
	WHITE3,
	WHITE4,
	WHITE5,
	WHITE6,
	WHITE7,
	WHITE8,
	GREY,
	GREY2,
	GREY3,
	SKIN,
	RED,
	RIBBON,
	WAND_RED,
	WAND_BLUE,
	WAND_PURPLE,
	WAND_PINK,
};

HRESULT InitVertex(void);
void UninitVertex(void);
ID3D11Buffer* GetVertexArrayBuffer(int color);