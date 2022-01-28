#ifndef __HEADER__
#define __HEADER__

#include <windows.h>
#include <commctrl.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/GLEXT.h>
#include <cmath>
#include <cstdio>

#include "version.h"
#include "Ini.h"


typedef unsigned char   byte;

#define MAX_VERTICES    2048
#define MAX_TRIANGLES     2048
#define pi          3.141592653f
//--Windows
#define IDM_TOOL    2
#define IDM_STATUS  3
#define ID_DLG_CAR  4
#define ID_dlg_ANI  5
#define ID_dlg_SND  6
#define IDR_MENU    7
#define IDF_NEW     8
#define IDF_OPEN    9
#define IDF_SAVEAS  10
#define IDF_EXIT    11
#define IDF_IMTEX   12
#define IDF_EXTEX   13
#define IDF_IMMOD   14
#define IDF_EXMOD   15
#define IDF_ANI     16
#define IDF_SND     17
#define IDF_ABOUT   18
#define IDF_HELP    19
#define IDF_LIGHT   20
#define IDF_FLAGS   21
#define IDF_WIRE    22
#define IDB_OK      23
#define IDB_CLOSE   24
#define IDB_OPEN    25
#define IDB_SAVE    26
#define IDB_NEXT    27
#define IDB_PREV        28
#define IDB_SPLAY       29
#define IDB_APLAY       30
#define IDB_ASTOP       31
#define ID_NAME         32
#define ID_AKPS         33
#define ID_ATIME        34
#define ID_AFRM         35
#define ID_SIZE         36
#define ID_TB_IMAGE     37
#define ID_ANIM_PLAY    38
#define ID_ANIM_PAUSE   39
#define ID_ANIM_STOP    40
#define ID_UPLOAD       41
#define IDF_PROPERTIES  42
#define ID_O_RADIUS     43
#define ID_O_YLO        44
#define ID_O_YHI        45
#define ID_O_BOUNDR     46
#define IDV_BONES       47
#define IDV_JOINTS      48
#define IDF_UPDATE      49
#define IDV_PERSPECTIVE 50
#define IDV_GRID		58
#define IDV_AXES		59
#define ID_ABOUT_DIALOG 60
#define ID_ABOUT_OK		61
#define ID_ABOUT_TEXT	62
#define IDT_FLIPTRIS    69
#define IDT_FLIPUV		70
#define IDV_SPECULAR	71
#define IDV_ENVMAP		72


// Context Menu
enum {
	// Animation root
	MPM_ADD_ANIM =			200,
	MPM_CLEAR_ANIM =		201,
	// Animations
	MPM_RENAME_ANIM =		202,
	MPM_INSERT_ANIM =		203,
	MPM_REMOVE_ANIM =		204,
	MPM_PROP_ANIM =			205,

	// Sounds root
	MPM_ADD_SOUND =			206,
	MPM_CLEAR_SOUND = 		207,
	// Sounds
	MPM_RENAME_SOUND =		208,
	MPM_PROP_SOUND =		209,
};

#define sfDoubleSide    0x0001
#define sfDarkBack      0x0002
#define sfOpacity       0x0004
#define sfTransparent   0x0008
#define sfMortal        0x0010
//#define sfPhong         0x0030
//#define sfEnvMap        0x0050
#define sfPhong         0x0020
#define sfEnvMap        0x0040
#define sfNeedVC        0x0080

#define sfDark          0x8000

#define ofPLACEWATER       1
#define ofPLACEGROUND      2
#define ofPLACEUSER        4
#define ofCIRCLE           8
#define ofBOUND            16
#define ofNOBMP            32
#define ofNOLIGHT          64
#define ofDEFLIGHT         128
#define ofGRNDLIGHT        256
#define ofNOSOFT           512
#define ofNOSOFT2          1024
#define ofANIMATED         0x80000000

enum EView
{
    VIEW_PERSPECTIVE = 0,
    VIEW_LEFT,
    VIEW_RIGHT,
    VIEW_FRONT,
    VIEW_BACK,
    VIEW_TOP,
    VIEW_BOTTOM,
	VIEW_FIRSTPERSON
};

//==Type definitions==//

#pragma pack(push, 1)
typedef struct _TARGAINFOHEADER
{
	enum {
		IMAGETYPE_NONE		= 0,
		IMAGETYPE_INDEXED	= 1,
		IMAGETYPE_RGB		= 2,
		IMAGETYPE_GREY		= 3,
		IMAGETYPE_RLE		= 8// 8bits?
	};

    BYTE  tgaIdentsize;         // size of ID field that follows 18 byte header (0 usually)
    BYTE  tgaColourmaptype;     // type of colour map 0=none, 1=has palette
    BYTE  tgaImagetype;         // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    WORD colourmapstart;		// first colour map entry in palette
    WORD tgaColourmaplength;    // number of colours in palette
    BYTE tgaColourmapbits;      // number of bits per palette entry 15,16,24,32

    WORD tgaXstart;             // image x origin
    WORD tgaYstart;             // image y origin
    WORD tgaWidth;              // image width in pixels
    WORD tgaHeight;             // image height in pixels
    BYTE  tgaBits;              // image bits per pixel 8,16,24,32
    BYTE  tgaDescriptor;        // image descriptor bits (vh flip bits)
} TARGAINFOHEADER;

typedef struct _WAVEHEADER
{
	DWORD	wavChunkID;
	DWORD	wavChunkSize;
	DWORD	wavFormat;
} WAVEHEADER;
#pragma pack(pop)

#include "Model.h"

typedef struct _rgb {
    byte r,g,b;
} RGB;


typedef struct _tagCamera {
    float x ,y ,z;
    float xt,yt,zt;
    float yaw,pitch;
    float dist;
} CAMERA;


//==Variables==//
extern CAMERA       cam,ortho;
extern TRIANGLE     g_Triangles[MAX_TRIANGLES];
extern VERTEX       g_Verticies[MAX_VERTICES];
extern NORM         g_Normals[MAX_VERTICES],g_VNormals[MAX_VERTICES];
extern BONE         g_Bones[32];
extern TVtl         g_Animations[32];
extern SOUND        g_Sounds[32];
extern GLuint       g_TextureID[3];
extern MODEL_HEADER Model;
extern WORD         g_Texture[256*1024];
extern WORD         g_Sprite[128*128];
extern HWND         g_hMain,gHTool,gHStatus,g_hAbout;
extern bool         key[256],
					mouse[3],
                    ISCAR,
					ISC2O,
                    WIREFRAME,
					LIGHT,
					COLOR,
					DRAW_GRID,
					DRAW_BONES,
					DRAW_JOINTS,
					DRAW_AXES,
					DRAW_SPECULAR,
					DRAW_ENVMAP,
					ANIMPLAY;
extern char         fileName[260];
extern int          CUR_FRAME,CUR_ANIM,CUR_SOUND;
extern long         LastTick,RealTime,PrevTime,TimeDt,FTime;
extern int          FPS,Frames;
extern int          WinX,WinY,WinW,WinH,CurX,CurY;
extern GLbyte       WAVhead[];
extern EView		CameraView;

extern HWND    g_hAniDlg,g_hSndDlg,g_hCarDlg, g_DrawArea,g_FileView,g_AniTrack, g_AniDlgTrack, embedTest;
//==Functions==//
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void    EnableOpenGL(HWND hwnd, HDC*, HGLRC*, int, int);
void    DisableOpenGL(HWND, HDC, HGLRC);
void    RenderMesh();
void    DrawTextGL(float, float, float, char*, unsigned int);
void    tga16_to_bgr24(WORD *A, byte *bgr, int L);
float   lengthdir_x(float len,float yaw,float pitch);
float   lengthdir_y(float len,float yaw,float pitch);
float   lengthdir_z(float len, float pitch);
float   degtorad(float deg);
//bool    rayIntersectsTriangle( EZvector &Origin, EZvector &Direction, EZvector &v1, EZvector &v2, EZvector &v3);
//bool rayIntersectsPoint(EZvector &Origin, EZvector &Direction, const EZvector &Point);
void    PlayWave(SOUND *sptr);
void    importModel();
void    exportSound();
void    importSound();
void    SaveProject();
NORM    ComputeNormals(VERTEX vt1, VERTEX vt2, VERTEX vt3);
BOOL    OpenFileDlg(const char *filter, const char *ext);
BOOL    SaveFileDlg(const char *filter, const char *ext);
BOOL    SaveFileDlg(char*,char*,char*);

#endif // __HEADER__
