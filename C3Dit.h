#pragma once
#ifndef C3DIT_H
#define C3DIT_H

#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>

#include <cstdint>
#include <vector>
#include <string>

#include "vec3.h"
#include "version.h"
#include "resource.h"


#ifdef GLOBALVAR_DEFINE
#define EXTORNOT
#else
#define EXTORNOT extern
#endif // GLOBALVAR_DEFINE



// Maximum vertices allowed by the gmae
#define MAX_VERTICES		1024
// Maximum triangles allowed by the game
#define MAX_TRIANGLES		1024
// TODO: get rid of this?
constexpr float pi = 3.141592653f;
//#define pi					3.141592653f

// Face Flags
enum FaceFlagsEnum {
	sfDoubleSide = 1, //(1 << 0)
	sfDarkBack = 2, //(1 << 1)
	sfOpacity = 4, //(1 << 2)
	sfTransparent = 8, //(1 << 3)
	sfMortal = 16, //(1 << 4)
	sfPhong = 32, //(1 << 5)
	sfEnvMap = 64, //(1 << 6)
	sfNeedVC = 128, //(1 << 7)
	sfUnused1 = 256, //(1 << 8)
	sfUnused2 = 512, //(1 << 9)
	sfUnused3 = 1024, //(1 << 10)
	sfUnused4 = 2048, //(1 << 11)
	sfUnused5 = 4096, //(1 << 12)
	sfUnused6 = 8192, //(1 << 13)
	sfUNused7 = 16384, //(1 << 14)
	sfDark = 32768,  //(1 << 15)
	sfAll = 0xFFFF, // 65535
	FACEFLAGS_MAX
};

// Object Flags
#define ofPLACEWATER       1 //(1 << 0)
#define ofPLACEGROUND      2 //(1 << 1)
#define ofPLACEUSER        4 //(1 << 2)
#define ofCIRCLE           8 //(1 << 3)
#define ofBOUND            16 //(1 << 4)
#define ofNOBMP            32 //(1 << 5)
#define ofNOLIGHT          64 //(1 << 6)
#define ofDEFLIGHT         128 //(1 << 7)
#define ofGRNDLIGHT        256 //(1 << 8)
#define ofNOSOFT           512 //(1 << 9)
#define ofNOSOFT2          1024 //(1 << 10)
#define ofANIMATED         0x80000000 //(1 << 31)

enum ViewEnum
{
	VIEW_PERSPECTIVE = 0,
	VIEW_FIRSTPERSON = 1,
	VIEW_ORTHOGRAPHIC = 2,
	VIEW_MAX
};


//////////////////////////////////////////////////
// Type definitions

class CVertex
{
public:

	float x, y, z;
	short Bone;
	short Hidden;
};


class CFace
{
public:
	int32_t v1, v2, v3;
	struct _tcoord {
		int16_t x, y;
	} tcoord[3];
	uint16_t Flags;
	uint16_t DMask;
	int32_t Prev;
	int32_t Next;
	int32_t Group;
	uint32_t Reserved[3];
};

class CBone
{
public:
	std::string Name;
	float x, y, z;
	int16_t Parent;
	int16_t Hide;
};


class CAnimation
{
public:

	virtual bool Load(const std::string& filepath) = 0;
	virtual bool Save(const std::string& filepath) = 0;
};


template <typename T> class CFrame
{
public:

	std::vector<T> Positions;

	CFrame() {}
	CFrame(const CFrame& f) { std::copy(f.Positions.begin(), f.Positions.end(), Positions); }
	//CFrame(CFrame&& f) { std::move(f.Positions.begin(), f.Positions.end(), Positions); }
};


class CVertexAnimation : public CAnimation
{
public:

	//--Animation
	std::string Name;
	int32_t SoundFX; // Index of Sound Effect associated 
	int32_t KPS;
	int32_t FrameCount; // Deprecated
	int16_t* Data; // Deprecated
	std::vector<CFrame<vec3>> Frames;

	CVertexAnimation() :
		Name(""),
		SoundFX(-1),
		KPS(0),
		FrameCount(0),
		Data(nullptr),
		Frames()
	{}

	~CVertexAnimation() { if (Data) delete[] Data; }

	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
};


class CSound
{
public:

	enum {
		PCM_8BIT,
		PCM_16BIT,
		PCM_24BIT
	};

	std::string Name;
	size_t Length;
	uint32_t SampleCount;
	int16_t* Data;
	//uint32_t BitDepth;
	//uint32_t Frequency;

	CSound() :
		Name(""),
		Length(0u),
		SampleCount(0u),
		Data(nullptr)
	{}

	CSound(const CSound& s) :
		Name(s.Name),
		Length(s.Length),
		SampleCount(s.SampleCount),
		Data(nullptr)
	{
		Data = new int16_t[Length];
		std::memcpy(Data, s.Data, Length);
	}

	/*CSound(CSound&& s) noexcept :
		Name(s.Name),
		Length(s.Length),
		SampleCount(s.SampleCount),
		Data(s.Data)
	{
		s.Name = "";
		s.Length = 0u;
		s.SampleCount = 0u;
		s.Data = nullptr;
	}*/
	
	~CSound() { if (Data) delete[] Data; }

	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
};


typedef struct _rgba16 {
	uint16_t r : 5;
	uint16_t g : 5;
	uint16_t b : 5;
	uint16_t a : 1;
} RGBA16;


class RGB
{
public:
	union {
		struct { uint8_t r, g, b; };
		uint8_t rgb[3];
	};
};


class RGBA
{
public:
	union {
		struct { uint8_t r, g, b, a; };
		uint8_t rgba[4];
	};

	RGBA& operator= (const RGBA16& rhs);
	RGBA& operator= (const RGB& rhs);
};


class CObjectInfo
{
public:
	int32_t Radius;
	int32_t YLo, YHi;

	struct SLine {
		int32_t Length;
		int32_t Intensity;
	} Line;

	struct SCircle {
		int32_t Radius;
		int32_t Intensity;
	} Circle;

	uint32_t Flags;
	int32_t GrRad;
	uint32_t DefLight;
	int32_t LastAniTime;
	float BoundR;
	uint32_t Reserved[4];

	CObjectInfo() :
		Radius(0),
		YLo(0),
		YHi(0),
		Line({ 0, 0 }),
		Circle({ 0, 0 }),
		Flags(0x00),
		GrRad(0),
		DefLight(0xFFFFFF),
		LastAniTime(0),
		BoundR(0.0f),
		Reserved()
	{}

	void Clear();
};


class CModel
{
public:
	std::string Name;

	// -- Data
	std::vector<CVertex> Vertices;
	std::vector<CFace> Faces;
	std::vector<CBone> Bones;

	CModel() : 
		Name(""),
		Vertices(),
		Faces(),
		Bones()
	{}

	void Clear();

	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
};


class CTexture
{
public:
	uint32_t Width, Height;
	RGBA* Data;

	CTexture() :
		Width(0u),
		Height(0u),
		Data(nullptr)
	{}
	
	CTexture(CTexture&& t) noexcept
	{
		Width = t.Width;
		Height = t.Height;
		Data = t.Data;
		t.Width = 0u;
		t.Height = 0u;
		t.Data = nullptr;
	}

	~CTexture()
	{
		if (Data)
			delete[] Data;
	}

	void Clear();
	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
};


class CCamera
{
public:
	float x, y, z;
	float TargetX, TargetY, TargetZ;
	float Yaw, Pitch;
	float Dist;
	ViewEnum Type;
};


class CScene
{
public:
	CObjectInfo ObjInfo;
	CModel Model;
	CTexture Texture;
	CTexture Sprite;
	std::vector<CVertexAnimation> Animations;
	std::vector<CSound> SoundEffects;

	int64_t LastTick, RealTime, PrevTime, TimeDt, FTime;

	void Clear();
};


/////////////////////////////////////////////////////////////////////////////////////////
// Variables
EXTORNOT CCamera g_Camera;
EXTORNOT CScene g_Scene;
EXTORNOT HWND g_hMain, gHTool, gHStatus, g_hAbout, g_hTools;
EXTORNOT HWND g_hAniDlg, g_hSndDlg, g_hCarDlg, g_DrawArea, g_FileView, g_AniTrack, g_AniDlgTrack, embedTest;
EXTORNOT int WinX, WinY, WinW, WinH, CurX, CurY;


/////////////////////////////////////////////////////////////////////////////////////////
// Functions
std::string OpenFileDlg(HWND parent, const std::string& filter, const std::string& ext, const std::string& title = "");
std::string SaveFileDlg(HWND parent, const std::string& filter, const std::string& ext, const std::string& title = "");

void loadCAR();
void loadCAR(const std::string&);
void newScene();
void mouseE();
void keyboardE();
void exportTexture();
void exportModel();
void importTexture();
void PlayWave(CSound&);
void LoadCARData(const std::string&);
void LoadC2OData(const std::string&);
void Load3DFData(const std::string&);
void Load3DNData(const std::string&);
void LoadOBJData(const std::string&);

#endif // C3DIT_H
