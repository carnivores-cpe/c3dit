#include "stdafx.h"
#include "C3Dit.h"

void CScene::Clear()
{
	ObjInfo.Clear();
	Model.Clear();
	Texture.Clear();
	Sprite.Clear();
	Animations.clear();
	SoundEffects.clear();
}

void CTexture::Clear()
{
	if (Data)
		delete[] Data;
	Data = nullptr;
	Width = 0u;
	Height = 0u;
}

void CModel::Clear()
{
	Name = "";
	Vertices.clear();
	Faces.clear();
	Bones.clear();
}

void CObjectInfo::Clear()
{
	Radius = (0);
	YLo = (0);
	YHi = (0);
	Line.Length = 0;
	Line.Intensity = 0;
	Circle.Radius = 0;
	Circle.Intensity = 0;
	Flags = (0x00);
	GrRad = (0);
	DefLight = (0xFFFFFF);
	LastAniTime = (0);
	BoundR = (0.0f);
	for (int i = 0; i < 4; i++)
		Reserved[i] = 0;
}

RGBA& RGBA::operator=(const RGB& rhs)
{
	this->r = rhs.r;
	this->g = rhs.g;
	this->b = rhs.b;
	this->a = 255;
	return *this;
}

RGBA& RGBA::operator=(const RGBA16& rhs)
{
	this->r = rhs.r * 4;
	this->g = rhs.g * 4;
	this->b = rhs.b * 4;
	this->a = rhs.a * 255;
	return *this;
}
