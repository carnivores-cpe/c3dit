#pragma once
#ifndef EDITORMAIN_H
#define EDITORMAIN_H

#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <glm/fwd.hpp>
#include "version.h"
#include "resource.h"


#ifndef DEBUGONLYPRINT
#ifndef NDEBUG
#define DEBUGONLYPRINT(x) std::cout << (x) << std::endl
#else
#define DEBUGONLYPRINT(x)
#endif // !NDEBUG
#endif // DEBUGONLYPRINT


#ifdef GLOBALVAR_DEFINE
#define EXTORNOT
#else
#define EXTORNOT extern
#endif // GLOBALVAR_DEFINE



// Maximum vertices allowed by the game
#define MAX_VERTICES		1024
// Maximum triangles allowed by the game
#define MAX_TRIANGLES		1024
constexpr float pi = 3.141592653f;

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
	VIEW_CUSTOM = 3,
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
	int32_t Vertex[3];
	glm::vec3 Normal;
	glm::vec2 VertexTexture[3];
	uint32_t Flags;
	//uint16_t DMask;
	//int32_t Prev;
	//int32_t Next;
	int32_t Group;
};

class CBone {
public:
	std::string Name;
	glm::vec3 Position;
	int16_t Parent;
	int16_t Hide;

	CBone(const char* name, float xx, float yy, float zz, int16_t parent, int16_t hide) :
		Name(name), Position(xx, yy, zz), Parent(parent), Hide(hide)
	{}

	CBone(const CBone& src) :
		Name(src.Name), Position(src.Position), Parent(src.Parent), Hide(src.Hide)
	{}

	CBone& operator=(const CBone& src) {
		Name = src.Name;
		Position = src.Position;
		Parent = src.Parent;
		Hide = src.Hide;
		return *this;
	}
};


class CAnimation
{
protected:
	std::string mName;

public:

	CAnimation() : mName("") {}
	virtual ~CAnimation() = 0;

	void SetName(std::string const& name) { mName = name; }
	std::string GetName() const { return mName; }

	virtual bool Load(std::string const& filepath) = 0;
	virtual bool Save(std::string const& filepath) = 0;
};


class CVertexAnimation : public CAnimation
{
public:

	class CVertexFrame
	{
	protected:
		std::vector<glm::vec3> mPositions;

	public:
		CVertexFrame() {}
		CVertexFrame(const CVertexFrame& f)
		{
			//Positions = f.Positions;
			for (auto pos : f.mPositions)
			{
				mPositions.insert(mPositions.end(), pos);
			}
		}
	};

	//--Animation
	int32_t mSoundFX; // Index of Sound Effect associated 
	int32_t mKPS;
	uint32_t mFrameCount; // Deprecated
	std::vector<CVertexFrame> mFrames;

	CVertexAnimation() :
		CAnimation(),
		mSoundFX(-1),
		mKPS(0),
		mFrameCount(0),
		mFrames()
	{}

	~CVertexAnimation() {}

	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
};


class CBoneAnimation : public CAnimation
{
public:

	class CBoneFrame
	{
	protected:
		struct SBoneData { glm::vec3 mPosition; glm::vec3 mRotation; glm::vec3 mScale; };
		std::vector<SBoneData> mData;

	public:
		CBoneFrame() {}
		CBoneFrame(const CBoneFrame& f)
		{
			for (auto data : f.mData) {
				mData.insert(mData.end(), data);
			}
		}
	};

	//--Animation
	int32_t mSoundFX; // Index of Sound Effect associated 
	int32_t mKPS;
	uint32_t mFrameCount; // Deprecated
	std::vector<CBoneFrame> mFrames;

	CBoneAnimation() :
		CAnimation(),
		mSoundFX(-1),
		mKPS(0),
		mFrameCount(0),
		mFrames()
	{}

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
	std::unique_ptr<int16_t[]> Data;
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
		Data.reset(new int16_t[s.Length]);
		std::memcpy(Data.get(), s.Data.get(), s.Length);
	}

	/*CSound(CSound&& s) noexcept :
		Name(s.Name),
		Length(s.Length),
		SampleCount(s.SampleCount),
		Data(nullptr)
	{
		s.Name = "";
		s.Length = 0u;
		s.SampleCount = 0u;
		Data.swap(s.Data);
	}*/

	CSound& operator= (const CSound& rhs)
	{
		Name = rhs.Name;
		Length = rhs.Length;
		SampleCount = rhs.SampleCount;
		Data.reset(new int16_t[rhs.Length]);
		std::memcpy(Data.get(), rhs.Data.get(), rhs.Length);
		return *this;
	}

	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
};



class CColourRGBA
{
private:
	typedef uint8_t component_type;

public:
	union {
		struct { component_type r, g, b, a; };
		component_type rgba[4];
	};

	CColourRGBA() : r(0), g(0), b(0), a(0) {}
	CColourRGBA(component_type red, component_type green, component_type blue, component_type alpha) : r(red), g(green), b(blue), a(alpha) {}
	CColourRGBA(const CColourRGBA& src) = default;
	CColourRGBA(CColourRGBA&&) = default;

	component_type& operator[](unsigned i) {
		if (i >= 4) throw std::out_of_range("CColourRGBA::array accessor, index is out of range!");
		return rgba[i];
	}
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
protected:
	uint32_t mWidth, mHeight;
	std::shared_ptr<uint8_t[]> mData;

public:

	CTexture() :
		mWidth(0u),
		mHeight(0u),
		mData()
	{}

	CTexture(CTexture&& t) noexcept
	{
		mWidth = t.mWidth;
		mHeight = t.mHeight;
		mData.reset();
		mData.swap(t.mData);
		t.mWidth = 0u;
		t.mHeight = 0u;
	}

	~CTexture()
	{}

	void Clear();
	bool Load(const std::string& filepath);
	bool Save(const std::string& filepath);
	uint32_t GetWidth() { return mWidth; }
	uint32_t GetHeight() { return mHeight; }
	std::weak_ptr<uint8_t[]> GetData() { return mData; }
};


class CCamera
{
public:
	glm::vec3 mPosition;
	glm::vec4 mTargetPosition; // W-COMPONENT == Depth/Distance
	glm::vec3 mRotation;
	ViewEnum mType;
};


class CProject
{
public:

	CProject() { Clear(); }

	CObjectInfo ObjInfo;
	CModel Model;
	CTexture Texture;
	CTexture Sprite;
	std::vector<CVertexAnimation> Animations;
	std::vector<CSound> SoundEffects;

	uint32_t animation_current_frame;
	int32_t animation_current;
	int32_t sound_current;

	// TODO: move these to window.cpp
	int64_t LastTick;
	int64_t RealTime;
	int64_t PrevTime;
	int64_t TimeDt;
	int64_t FTime;

	void Clear();
};


/////////////////////////////////////////////////////////////////////////////////////////
// Variables
EXTORNOT CCamera g_Camera;
EXTORNOT CProject g_Project;
EXTORNOT HWND g_hMain, gHTool, gHStatus, g_hAbout, g_hTools;
EXTORNOT HWND g_hAniDlg, g_hSndDlg, g_hCarDlg, g_DrawArea, g_FileView, g_AniTrack, g_AniDlgTrack, embedTest;
EXTORNOT int CurX, CurY;

extern char g_ProjectFileFormats[];
extern char g_ModelFileFormats[];
extern char g_AnimationFileFormats[];
extern char g_ImageFileFormats[];
extern char g_SoundFileFormats[];


/////////////////////////////////////////////////////////////////////////////////////////
// Functions
namespace std {
	/* Split a string into a vector of tokenised strings, using the delimeter characters supplied */
	bool tokenize(vector<string>& _tokens, string _string, const string& _delims);
};

std::string OpenFileDlg(HWND parent, const char* filter, const std::string& ext, const std::string& title = "");
std::string SaveFileDlg(HWND parent, const char* filter, const std::string& ext, const std::string& title = "");

bool LoadProject(const std::string& filepath = "");
bool SaveProject(const std::string& filepath = "");

// NOTE: This isn't necessary as it is only used in Window.cpp but also defined there
void PlayWave(CSound&);

bool LoadProjectFile(const std::string&);
bool LoadCARFile(const std::string&);
bool LoadC1OFile(const std::string&);
bool LoadC2OFile(const std::string&);
bool Load3DFFile(const std::string&);
bool Load3DNFile(const std::string&);
bool LoadOBJFile(const std::string&);

bool SaveProjectFile(const std::string&);
bool SaveCARFile(const std::string&);
bool SaveC1OFile(const std::string&);
bool SaveC2OFile(const std::string&);
bool Save3DFFile(const std::string&);
bool Save3DNFile(const std::string&);
bool SaveOBJFile(const std::string&);

#endif // EDITORMAIN_H
