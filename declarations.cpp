
#include "header.h"

CAMERA       cam,ortho;
TRIANGLE     g_Triangles[MAX_TRIANGLES];
VERTEX       g_Verticies[MAX_VERTICES];
NORM         g_Normals[MAX_VERTICES];
NORM         g_VNormals[MAX_VERTICES];
BONE         g_Bones[32];
GLuint       g_TextureID[3];
WORD         g_Texture[256*1024];
WORD         g_Sprite[128*128];
TVtl         g_Animations[64];
SOUND        g_Sounds[64];
HWND         g_hMain,gHTool,gHStatus;
HWND         g_hAniDlg,g_hSndDlg,g_hCarDlg,g_hAbout, g_DrawArea,g_FileView,g_AniTrack, g_AniDlgTrack, embedTest;
MODEL_HEADER Model;
bool			key[256],
				mouse[3],
				ISCAR,
				ISC2O,
				WIREFRAME=false,
				LIGHT=false,
				COLOR=false,
				DRAW_GRID=true,
				DRAW_AXES=true,
				DRAW_BONES=true,
				DRAW_JOINTS=true,
				DRAW_SPECULAR=true,
				DRAW_ENVMAP=true,
				ANIMPLAY=false;
char			fileName[260];
int				CUR_FRAME=0,
				CUR_ANIM=0,
				CUR_SOUND=0;
long         LastTick,PrevTime=0,RealTime=0,TimeDt=0,FTime=0;
int          FPS,Frames;
int          WinX,WinY,WinW,WinH,CurX,CurY;
EView			CameraView = VIEW_PERSPECTIVE;

GLbyte       WAVhead[] = {0x52,0x49,0x46,0x46,0x8C,
                          0xD7,0x00,0x00,0x57,0x41,
                          0x56,0x45,0x66,0x6D,0x74,
                          0x20,0x12,0x00,0x00,0x00,
                          0x01,0x00,0x01,0x00,0x22,
                          0x56,0x00,0x00,0x44,0xAC,
                          0x00,0x00,0x02,0x00,0x10,
                          0x00,0x00,0x00,0x66,0x61,
                          0x63,0x74,0x04,0x00,0x00,
                          0x00,0xAD,0x6B,0x00,0x00,
                          0x64,0x61,0x74,0x61,0x5A,
                          0xD7,0x00,0x00};
