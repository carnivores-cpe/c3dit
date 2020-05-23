#include "stdafx.h"
#include "C3Dit.h"
#include "Global.h"

#include <GL/glext.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


struct GL {
	HGLRC rc;
	HDC dc;
	HWND wnd;
};

typedef struct _GLvertex {
	float x,y,z;
} GLvertex;


Globals		*ga = 0;
int			A_Timer=0;
float		g_Anisotropy = 1.0f;
float		lAmbient[] = {0.05f, 0.05f, 0.05f, 1.0f};
float		lDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
float		lShine[]   = {1.0f, 1.0f, 1.0f, 1.0f};
float		lSun[]     = {0.0f, 0.0f, 1.0f, 0.0f};

GLvertex	GridArrayVerts[64+64];
DWORD		GridArrayColors[64+64];

int			SplineD = 0;

bool		g_ShadersSupported = false;
GLint		g_Program,g_VShader,g_FShader;
GLint		g_Shaders_bLighting;	//uniform bool
GLint		g_Shaders_baseTexture;	//uniform sampler2D

GLuint		g_DefaultFont = 0;


VideoGL::VideoGL(HWND hWnd)
{
	try {
		gl = new GL;
	}
	catch (std::bad_alloc& e) {
		std::cerr << "" << std::endl;
		gl = nullptr;
		return;
	}

	Globals* g = GlobalContainer::GlobalVar();
	PIXELFORMATDESCRIPTOR pfd;
	std::memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	gl->wnd = hWnd;
	
	if (!(gl->dc = GetDC(hWnd)))
	{
		MessageBox(gl->wnd, "Failed to get the Device Context", "VideoGL", MB_ICONERROR | MB_OK);
		return;
	}

	// Set the pixel format for the DC
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = g->iColorBits;
	pfd.cDepthBits = g->iDepthBits;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int iFormat = ChoosePixelFormat(gl->dc, &pfd);
	if (!SetPixelFormat(gl->dc, iFormat, &pfd))
	{
		MessageBox(gl->wnd, "Failed to set the pixel format!", "VideoGL", MB_ICONERROR);
		ReleaseDC(gl->wnd, gl->dc);
		return;
	}

	g->iColorBits = pfd.cColorBits;
	g->iDepthBits = pfd.cDepthBits;

	// create and enable the render context (RC)
	if (!(gl->rc = wglCreateContext(gl->dc)))
	{
		MessageBox(gl->wnd, "Failed to create the WGL context!", "VideoGL", MB_ICONERROR);
		ReleaseDC(gl->wnd, gl->dc);
		return;
	}

	wglMakeCurrent(gl->dc, gl->rc);

	//glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);

	glDepthFunc(GL_LEQUAL);
	glAlphaFunc(GL_GEQUAL, 0.75f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);//--Normal

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Lights
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lShine);
	glLightfv(GL_LIGHT0, GL_POSITION, lSun);
	glEnable(GL_LIGHT0);

	glClearDepth(1.0f);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glGenTextures(5, g_TextureID);

	void* SPECMAP;
	FILE* fp = fopen(g->sSpecularMap.c_str(), "rb");
	if (fp)
	{
		TARGAINFOHEADER tih;
		fread(&tih, sizeof(TARGAINFOHEADER), 1, fp);

		SPECMAP = new uint8_t[tih.tgaWidth * tih.tgaHeight * (tih.tgaBits / 8)];

		fread(SPECMAP, tih.tgaWidth * tih.tgaHeight * (tih.tgaBits / 8), 1, fp);

		fclose(fp);

		glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (tih.tgaBits == 16)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, SPECMAP);
		if (tih.tgaBits == 24)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, SPECMAP);
		if (tih.tgaBits == 32)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, SPECMAP);

		delete[] SPECMAP;
		SPECMAP = 0;
	}

	fp = fopen(g->sEnvironmentMap.c_str(), "rb");
	if (fp)
	{
		TARGAINFOHEADER tih;
		fread(&tih, sizeof(TARGAINFOHEADER), 1, fp);

		SPECMAP = new uint8_t[tih.tgaWidth * tih.tgaHeight * (tih.tgaBits / 8)];

		fread(SPECMAP, tih.tgaWidth * tih.tgaHeight * (tih.tgaBits / 8), 1, fp);

		fclose(fp);

		glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (tih.tgaBits == 16)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, SPECMAP);
		if (tih.tgaBits == 24)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, SPECMAP);
		if (tih.tgaBits == 32)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, SPECMAP);

		delete[] SPECMAP;
		SPECMAP = 0;
	}
	/******************** END OF SPECIAL EFFECTS ********************************/

	if (GLEW_EXT_texture_filter_anisotropic)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &g_Anisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_Anisotropy);
	}

	if (GLEW_EXT_texture_env_combine)
	{
		//MessageBox(hwnd, "GL_EXT_texture_env_combine Supported", "OpenGL", MB_OK);
	}

	// Shaders
	if (glewIsSupported("GL_VERSION_2_0"))
	{
		g_ShadersSupported = true;
		g_Program = glCreateProgram();
		g_VShader = glCreateShader(GL_VERTEX_SHADER);
		g_FShader = glCreateShader(GL_FRAGMENT_SHADER);

		GLchar* vertShader = 0;
		GLchar* fragShader = 0;
		GLint	vlen = 0;
		GLint	flen = 0;

		FILE* fp = fopen("vertex.txt", "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			vlen = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			vertShader = new GLchar[vlen];

			fread(vertShader, vlen, 1, fp);

			fclose(fp);
		}

		fp = fopen("fragment.txt", "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			flen = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			fragShader = new GLchar[flen];

			fread(fragShader, flen, 1, fp);

			fclose(fp);
		}

		const GLchar* vv = vertShader;
		const GLint	cvlen = vlen;

		glShaderSource(g_VShader, 1, &vv, &cvlen);
		glCompileShader(g_VShader);

		GLint status = GL_FALSE;

		glGetShaderiv(g_VShader, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLint infoLen = 0;
			GLchar infoStr[512];
			glGetShaderInfoLog(g_VShader, 512, &infoLen, infoStr);
			printf("Failed to compile the vert shader!\n%s\n", infoStr);
		}

		const GLchar* fv = fragShader;
		const GLint	cflen = flen;

		glShaderSource(g_FShader, 1, &fv, &cflen);
		glCompileShader(g_FShader);

		glGetShaderiv(g_FShader, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLint infoLen = 0;
			GLchar infoStr[512];
			glGetShaderInfoLog(g_FShader, 512, &infoLen, infoStr);
			printf("Failed to compile the frag shader!\n%s\n", infoStr);
		}

		glAttachShader(g_Program, g_VShader);
		glAttachShader(g_Program, g_FShader);
		glLinkProgram(g_Program);

		glGetProgramiv(g_Program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint infoLen = 0;
			GLchar infoStr[512];
			glGetProgramInfoLog(g_Program, 512, &infoLen, infoStr);
			printf("Failed to link the shader program!\n%s\n", infoStr);
		}

		glUseProgram(g_Program);

		g_Shaders_bLighting = glGetUniformLocation(g_Program, "bLighting");
		glUniform1i(g_Shaders_bLighting, false);

		g_Shaders_baseTexture = glGetUniformLocation(g_Program, "baseTexture");
		glUniform1i(g_Shaders_baseTexture, 0); // Texture Unit 0

		delete[] vertShader;
		delete[] fragShader;
	}

	g_DefaultFont = glGenLists(255);
	if (!g_DefaultFont)
	{
		MessageBox(hwnd, "Failed to generate font list.", "OpenGL", MB_OK);
	}

	HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SelectObject(g_DContext, (HFONT)font);
	SelectObject(g_DContext, (HFONT)font);

	wglUseFontBitmaps(g_DContext, 0, 255, g_DefaultFont);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 1.3333f, 0.1f, 1024.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int vert = 0;
	for (int x = 0; x < 32; x++)
	{
		GridArrayVerts[vert].x = (float)x - 16.0f;
		GridArrayVerts[vert].y = 0.0f;
		GridArrayVerts[vert].z = -16.0f;
		GridArrayColors[vert] = 0xFF202020;
		vert++;
		GridArrayVerts[vert].x = (float)x - 16.0f;
		GridArrayVerts[vert].y = 0.0f;
		GridArrayVerts[vert].z = 16.0f;
		GridArrayColors[vert] = 0xFF202020;
		vert++;
	}
	for (int y = 0; y < 32; y++)
	{
		GridArrayVerts[vert].x = -16.0f;
		GridArrayVerts[vert].y = 0.0f;
		GridArrayVerts[vert].z = (float)y - 16.0f;
		GridArrayColors[vert] = 0xFF202020;
		vert++;
		GridArrayVerts[vert].x = 16.0f;
		GridArrayVerts[vert].y = 0.0f;
		GridArrayVerts[vert].z = (float)y - 16.0f;
		GridArrayColors[vert] = 0xFF202020;
		vert++;
	}

}

VideoGL::~VideoGL()
{
	if (gl)
		delete gl;
}


void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	Globals* g = GlobalContainer::GlobalVar();
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    g_DContext = GetDC(hwnd);
    *hDC = g_DContext;

    if ( !g_DContext )
    {
        MessageBox(hwnd, "Failed to get the Device Context", "OpenGL", MB_ICONERROR | MB_OK);
    }

    /* Set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
				  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = g->iColorBits;
    pfd.cDepthBits = g->iDepthBits;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(g_DContext, &pfd);

    SetPixelFormat(g_DContext, iFormat, &pfd);

	g->iColorBits = pfd.cColorBits;
    g->iDepthBits = pfd.cDepthBits;

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext( g_DContext );

    wglMakeCurrent(g_DContext, *hRC);

	if ( glewInit() != GLEW_OK )
	{
		printf( "Failed to initialise GLEW!\n" );
	}

    //glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_COLOR_MATERIAL);

    glDepthFunc( GL_LEQUAL );
    glAlphaFunc( GL_GEQUAL, 0.75f );
    glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_DST_ALPHA );//--Normal

    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    // Lights
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lShine);
    glLightfv(GL_LIGHT0, GL_POSITION, lSun);
    glEnable(GL_LIGHT0);

    glClearDepth(1.0f);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glGenTextures(5,g_TextureID);

    void* SPECMAP;
    FILE *fp = fopen( g->sSpecularMap.c_str(),"rb" );
	if ( fp )
	{
		TARGAINFOHEADER tih;
		fread( &tih, sizeof( TARGAINFOHEADER ), 1, fp );

		SPECMAP = new uint8_t [ tih.tgaWidth * tih.tgaHeight * (tih.tgaBits/8) ];

		fread( SPECMAP, tih.tgaWidth * tih.tgaHeight * (tih.tgaBits/8), 1, fp );

		fclose(fp);

		glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if ( tih.tgaBits == 16 )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, SPECMAP);
		if ( tih.tgaBits == 24 )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, SPECMAP);
		if ( tih.tgaBits == 32 )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, SPECMAP);
		
		delete [] SPECMAP;
		SPECMAP = 0;
	}

    fp = fopen( g->sEnvironmentMap.c_str(),"rb");
	if ( fp )
	{
		TARGAINFOHEADER tih;
		fread( &tih, sizeof( TARGAINFOHEADER ), 1, fp );

		SPECMAP = new uint8_t [ tih.tgaWidth * tih.tgaHeight * (tih.tgaBits/8) ];

		fread( SPECMAP, tih.tgaWidth * tih.tgaHeight * (tih.tgaBits/8), 1, fp );

		fclose(fp);

		glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if ( tih.tgaBits == 16 )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, SPECMAP);
		if ( tih.tgaBits == 24 )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, SPECMAP);
		if ( tih.tgaBits == 32 )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tih.tgaWidth, tih.tgaHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, SPECMAP);
		
		delete [] SPECMAP;
		SPECMAP = 0;
	}
    /******************** END OF SPECIAL EFFECTS ********************************/

    if ( GLEW_EXT_texture_filter_anisotropic )
    {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &g_Anisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_Anisotropy);
    }

    if ( GLEW_EXT_texture_env_combine )
    {
        //MessageBox(hwnd, "GL_EXT_texture_env_combine Supported", "OpenGL", MB_OK);
    }

	// Shaders
	if ( glewIsSupported("GL_VERSION_2_0") )
	{
		g_ShadersSupported = true;
		g_Program = glCreateProgram();
		g_VShader = glCreateShader( GL_VERTEX_SHADER );
		g_FShader = glCreateShader( GL_FRAGMENT_SHADER );

		GLchar *vertShader = 0;
		GLchar *fragShader = 0;
		GLint	vlen = 0;
		GLint	flen = 0;

		FILE *fp = fopen( "vertex.txt", "rb" );
		if ( fp )
		{
			fseek( fp, 0, SEEK_END );
			vlen = ftell( fp );
			fseek( fp, 0, SEEK_SET );

			vertShader = new GLchar [ vlen ];

			fread( vertShader, vlen, 1, fp );

			fclose( fp );
		}

		fp = fopen( "fragment.txt", "rb" );
		if ( fp )
		{
			fseek( fp, 0, SEEK_END );
			flen = ftell( fp );
			fseek( fp, 0, SEEK_SET );

			fragShader = new GLchar [ flen ];

			fread( fragShader, flen, 1, fp );

			fclose( fp );
		}

		const GLchar* vv = vertShader;
		const GLint	cvlen = vlen;

		glShaderSource( g_VShader, 1, &vv, &cvlen );
		glCompileShader( g_VShader );

		GLint status = GL_FALSE;

		glGetShaderiv( g_VShader, GL_COMPILE_STATUS, &status );

		if ( status == GL_FALSE )
		{
			GLint infoLen = 0;
			GLchar infoStr[512];
			glGetShaderInfoLog( g_VShader, 512, &infoLen, infoStr );
			printf( "Failed to compile the vert shader!\n%s\n", infoStr );
		}

		const GLchar* fv = fragShader;
		const GLint	cflen = flen;

		glShaderSource( g_FShader, 1, &fv, &cflen );
		glCompileShader( g_FShader );

		glGetShaderiv( g_FShader, GL_COMPILE_STATUS, &status );

		if ( status == GL_FALSE )
		{
			GLint infoLen = 0;
			GLchar infoStr[512];
			glGetShaderInfoLog( g_FShader, 512, &infoLen, infoStr );
			printf( "Failed to compile the frag shader!\n%s\n", infoStr );
		}

		glAttachShader( g_Program, g_VShader );
		glAttachShader( g_Program, g_FShader );
		glLinkProgram( g_Program );

		glGetProgramiv( g_Program, GL_LINK_STATUS, &status );
		if ( status == GL_FALSE )
		{
			GLint infoLen = 0;
			GLchar infoStr[512];
			glGetProgramInfoLog( g_Program, 512, &infoLen, infoStr );
			printf( "Failed to link the shader program!\n%s\n", infoStr );
		}

		glUseProgram( g_Program );

		g_Shaders_bLighting = glGetUniformLocation( g_Program, "bLighting" );
		glUniform1i( g_Shaders_bLighting, false );

		g_Shaders_baseTexture = glGetUniformLocation( g_Program, "baseTexture" );
		glUniform1i( g_Shaders_baseTexture, 0 ); // Texture Unit 0

		delete [] vertShader;
		delete [] fragShader;
	}

    g_DefaultFont = glGenLists(255);
    if ( !g_DefaultFont )
    {
        MessageBox(hwnd, "Failed to generate font list.", "OpenGL", MB_OK);
    }

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SelectObject( g_DContext, (HFONT)font );
    SelectObject( g_DContext, (HFONT)font );

	wglUseFontBitmaps( g_DContext, 0, 255, g_DefaultFont);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,1.3333f,0.1f,1024.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	int vert = 0;
	for (int x=0; x<32; x++)
	{
		GridArrayVerts[ vert ].x = (float)x - 16.0f;
		GridArrayVerts[ vert ].y = 0.0f;
		GridArrayVerts[ vert ].z = -16.0f;
		GridArrayColors[ vert ] = 0xFF202020;
		vert++;
		GridArrayVerts[ vert ].x = (float)x - 16.0f;
		GridArrayVerts[ vert ].y = 0.0f;
		GridArrayVerts[ vert ].z = 16.0f;
		GridArrayColors[ vert ] = 0xFF202020;
		vert++;
	}
	for (int y=0; y<32; y++)
	{
		GridArrayVerts[ vert ].x = -16.0f;
		GridArrayVerts[ vert ].y = 0.0f;
		GridArrayVerts[ vert ].z = (float)y - 16.0f;
		GridArrayColors[ vert ] = 0xFF202020;
		vert++;
		GridArrayVerts[ vert ].x = 16.0f;
		GridArrayVerts[ vert ].y = 0.0f;
		GridArrayVerts[ vert ].z = (float)y - 16.0f;
		GridArrayColors[ vert ] = 0xFF202020;
		vert++;
	}

}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
	if ( g_ShadersSupported )
	{
		glDetachShader( g_Program, g_FShader );
		glDetachShader( g_Program, g_VShader );
		glDeleteProgram( g_Program );
		glDeleteShader( g_VShader );
		glDeleteShader( g_FShader );
	}

	glDeleteTextures(5,g_TextureID);
    glDeleteLists(g_DefaultFont, 255);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, unsigned long color)
{
    glBegin(GL_LINES);

    glColor4ubv((GLubyte*)&color);

    glVertex3f( x1,y1,z1 );
    glVertex3f( x2,y2,z2 );

    glEnd();
}

void DrawSphere(float radius, int segs, unsigned long color)
{
    glColor4ubv((GLubyte*)&color);
    GLUquadricObj *quadric = gluNewQuadric();
    gluSphere(quadric, radius, segs, segs);
    gluDeleteQuadric(quadric);
}

void c3ditDrawBones()
{
	glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

    // Bones
	if ( ga->bShowBones )
    for (int b=0; b<Model.mBoneCount; b++)
    {
        glPushMatrix();
        glTranslatef( g_Bones[b].x, g_Bones[b].y, g_Bones[b].z );

        DrawSphere( 5.0f, 16, 0xFF0080FF );

        glPopMatrix();

		if ( ga->bShowJoints )
        if (g_Bones[b].parent != -1)
        {
            int p = g_Bones[b].parent;
            DrawLine( g_Bones[b].x, g_Bones[b].y, g_Bones[b].z, g_Bones[p].x, g_Bones[p].y, g_Bones[p].z, 0xFF0000FF );
        }
    }
    glEnable(GL_DEPTH_TEST);
}

void RenderCARMesh()
{
    // Render Model

    glEnable(GL_TEXTURE_2D);

	glUseProgram( g_Program );
	glUniform1i( g_Shaders_bLighting, ga->bUseLighting );

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_Anisotropy);

    if ( ANIMPLAY && g_Animations[CUR_ANIM].FrameCount )
    {
        int AniTime = (g_Animations[CUR_ANIM].FrameCount * 1000) / g_Animations[CUR_ANIM].KPS;

        FTime += TimeDt;
        if ( FTime >= AniTime )
        {
			PlayWave( &g_Sounds[ Model.mAnimSFXTable[CUR_ANIM] ] );
			FTime = 0;
		}

        CUR_FRAME = ((g_Animations[CUR_ANIM].FrameCount-1) * (FTime) * 256) / AniTime;
        SplineD = CUR_FRAME & 0xFF;
        CUR_FRAME >>= 8;
        SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)CUR_FRAME );
    }
    if ( !ANIMPLAY )
    {
        CUR_FRAME = SendMessage(g_AniTrack, TBM_GETPOS, (WPARAM)0, (LPARAM)0 );
    }

    float k2 = (float)(SplineD) / 256.f;
    float k1 = 1.0f - k2;
    k1 /= 16.f;
    k2 /= 16.f;

    VERTEX l_RVerts[ MAX_VERTICES ];
	NORM l_NVerts[ MAX_VERTICES ];
	memset( l_NVerts, 0, sizeof(NORM)*MAX_VERTICES );

    if (Model.mAnimCount > 0 && g_Animations[CUR_ANIM].Data)
    for (int v=0; v<Model.mVertCount; v++)
    {
        short* adptr = &(g_Animations[CUR_ANIM].Data[CUR_FRAME * Model.mVertCount * 3]);
        l_RVerts[v].x = *(adptr + v*3+0) * k1 + *(adptr + (v+Model.mVertCount)*3+0) * k2;
        l_RVerts[v].y = *(adptr + v*3+1) * k1 + *(adptr + (v+Model.mVertCount)*3+1) * k2;
        l_RVerts[v].z = *(adptr + v*3+2) * k1 + *(adptr + (v+Model.mVertCount)*3+2) * k2;
    }
    else
    memcpy(l_RVerts, g_Verticies, sizeof(VERTEX) * Model.mVertCount);

	if ( Model.mAnimCount > 0 )
	{
		for (int f=0; f<Model.mFaceCount; f++)
		{
			int v1 = g_Triangles[f].v1;
			int v2 = g_Triangles[f].v2;
			int v3 = g_Triangles[f].v3;
			g_Normals[f] = ComputeNormals(l_RVerts[v1],l_RVerts[v2],l_RVerts[v3]);
		}
		
		for (int v=0; v<Model.mVertCount; v++)
		{
			unsigned nfaces = 0;
			for (int f=0; f<Model.mFaceCount; f++)
			{
				if ( g_Triangles[f].v1 == v ||
					 g_Triangles[f].v2 == v ||
					 g_Triangles[f].v3 == v )
				{
					l_NVerts[v] += g_Normals[f];
					nfaces++;
				}
			}
			
			l_NVerts[v] /= nfaces;

			float n = sqrtf( l_NVerts[v].x*l_NVerts[v].x + l_NVerts[v].y*l_NVerts[v].y + l_NVerts[v].z*l_NVerts[v].z );

			l_NVerts[v] /= n;
		}
	}
	else
    memcpy(l_NVerts, g_VNormals, sizeof(NORM) * Model.mVertCount);


    for (int t=0; t<Model.mFaceCount; t++)
    {
        if (g_Triangles[t].flags & sfDoubleSide) glDisable(GL_CULL_FACE);
        else glEnable(GL_CULL_FACE);

        if (g_Triangles[t].flags & sfOpacity) glEnable(GL_ALPHA_TEST);
        else glDisable(GL_ALPHA_TEST);

        int v1 = g_Triangles[t].v1;
        int v2 = g_Triangles[t].v2;
        int v3 = g_Triangles[t].v3;

        glBegin(GL_TRIANGLES);

        glColor4ub(255,255,255, 255);

        glNormal3fv(g_Normals[t].n);

		glNormal3fv(l_NVerts[v1].n);
        glTexCoord2f(g_Triangles[t].tx1/256.0f,g_Triangles[t].ty1/256.0f);
        glVertex3f(l_RVerts[v1].x,l_RVerts[v1].y,l_RVerts[v1].z);

        glNormal3fv(l_NVerts[v2].n);
        glTexCoord2f(g_Triangles[t].tx2/256.0f,g_Triangles[t].ty2/256.0f);
        glVertex3f(l_RVerts[v2].x,l_RVerts[v2].y,l_RVerts[v2].z);

        glNormal3fv(l_NVerts[v3].n);
        glTexCoord2f(g_Triangles[t].tx3/256.0f,g_Triangles[t].ty3/256.0f);
        glVertex3f(l_RVerts[v3].x,l_RVerts[v3].y,l_RVerts[v3].z);

        glEnd();
    }

	glUseProgram( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	for ( int i=0; i<g_TriSelection.size(); ++i )
	{
		unsigned f = g_TriSelection[i];
		unsigned v1 = g_Triangles[f].v1;
		unsigned v2 = g_Triangles[f].v2;
		unsigned v3 = g_Triangles[f].v3;
		
		glBegin( GL_TRIANGLES );

		glColor3f( 1.0f, 0.0f, 0.0f );
		glVertex3f( l_RVerts[v1].x, l_RVerts[v1].y, l_RVerts[v1].z );
		glVertex3f( l_RVerts[v2].x, l_RVerts[v2].y, l_RVerts[v2].z );
		glVertex3f( l_RVerts[v3].x, l_RVerts[v3].y, l_RVerts[v3].z );

		glEnd();
	}

	// Draw Normals
#if defined( _DEBUG ) && defined( DRAW_NORMALS )
	glBegin( GL_LINES );
	glColor3f( 0.0f, 0.0f, 1.0f );
	for ( int i=0; i<Model.mVertCount; ++i )
	{
		float nx = l_NVerts[i].x * 15.0f;
		float ny = l_NVerts[i].y * 15.0f;
		float nz = l_NVerts[i].z * 15.0f;
		glVertex3f( l_RVerts[i].x, l_RVerts[i].y, l_RVerts[i].z );
		glVertex3f( l_RVerts[i].x + nx, l_RVerts[i].y + ny, l_RVerts[i].z + nz );
	}
	glEnd();
#endif

    c3ditDrawBones();

    glEnable(GL_DEPTH_TEST);

    glLineWidth( 3.0f );
	GLint depthmask;
	glGetIntegerv( GL_DEPTH_WRITEMASK, &depthmask );
	glDepthMask( GL_FALSE );

	if ( ga->bShowWireframe )
    for (int t=0; t<Model.mFaceCount; t++)
    {
        int v1 = g_Triangles[t].v1;
        int v2 = g_Triangles[t].v2;
        int v3 = g_Triangles[t].v3;

        glBegin(GL_LINE_LOOP);

        glColor4ub(255,255,255, 255);

        glVertex3f(l_RVerts[v1].x,l_RVerts[v1].y,l_RVerts[v1].z);
        glVertex3f(l_RVerts[v2].x,l_RVerts[v2].y,l_RVerts[v2].z);
        glVertex3f(l_RVerts[v3].x,l_RVerts[v3].y,l_RVerts[v3].z);

        glEnd();
    }
	glDepthMask( depthmask );
    glLineWidth( 1.0f );

    glPointSize(4);
	if ( ga->bShowWireframe )
    {
        glBegin(GL_POINTS);
        for (int v=0; v<Model.mVertCount; v++)
        {
            glColor4ub(255,255,0, 255);

            /*vec3 V1(l_RVerts[v].x,l_RVerts[v].z,l_RVerts[v].y);
			vec3 CamP( cam.x, cam.z, cam.y );
			vec3 CamD(1.0f, 0.0f, 0.0f);//( cam.xt, cam.zt, cam.yt );
            if (rayIntersectsPoint(CamP,CamD,V1)) glColor4ub(255,0,0,255);*/

            glVertex3f(l_RVerts[v].x,l_RVerts[v].y,l_RVerts[v].z);
        }
        glEnd();
    }
    glPointSize(1);

    glEnable(GL_TEXTURE_2D);
	if ( ga->bUseLighting ) glEnable(GL_LIGHTING);
}

void RenderMesh()
{

	if ( ga==0 ) ga = GlobalContainer::GlobalVar();
    if (key['L'] && key[VK_SHIFT])
    {
        ga->bUseLighting = !ga->bUseLighting;

        if (ga->bUseLighting)
        {
            glEnable(GL_LIGHTING);
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 2);
            CheckMenuItem(hResMenu, IDF_LIGHT, MF_CHECKED | MF_BYCOMMAND);
        }
        else
        {
            glDisable(GL_LIGHTING);
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 2);
            CheckMenuItem(hResMenu, IDF_LIGHT, MF_UNCHECKED | MF_BYCOMMAND);

        }

        key['L'] = FALSE;
    }
    if (key['W'] && key[VK_SHIFT]) ga->bShowWireframe = !ga->bShowWireframe;

    glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lShine);
    glLightfv(GL_LIGHT0, GL_POSITION, lSun);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    if ( ga->bUseLighting ) glEnable(GL_LIGHTING);

    glPushMatrix();

    glScalef(0.1f,0.1f,0.1f);
    glBindTexture(GL_TEXTURE_2D,g_TextureID[0]);

    RenderCARMesh();

    glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	if ( ga->bShowGrid )
	{
		glPushMatrix();

		if ( CameraView == VIEW_RIGHT )
			glRotatef( 90.0f, 0.0f, 0.0f, 1.0f );
		if ( CameraView == VIEW_LEFT )
			glRotatef( 90.0f, 0.0f, 0.0f, 1.0f );
		if ( CameraView == VIEW_FRONT )
			glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
		if ( CameraView == VIEW_BACK )
			glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );

		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );

		glVertexPointer( 3, GL_FLOAT, sizeof(GLvertex), GridArrayVerts );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(DWORD), GridArrayColors );

		glDrawArrays( GL_LINES, 0, 64+64 );

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );

		glPopMatrix();
	}

    glPushMatrix();

    //Pivot point
	if ( ga->bShowAxis )
	{
		glDisable(GL_DEPTH_TEST);

		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(5.0f,0.0f,0.0f);

			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(0.0f,5.0f,0.0f);

			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(0.0f,0.0f,5.0f);
		glEnd();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		if ( ga->bUseLighting )
		glEnable(GL_LIGHTING);
	}

    glPopMatrix();

}

void DrawTextGL(float x, float y, float z, char* text, unsigned int color)
{
	// -> Outputs text to the OpenGL Scene
    GLenum Error = 0;

	if ( text == "" ) { return; }
	if ( !glIsList( g_DefaultFont ) ) { return; }

    glBindTexture(GL_TEXTURE_2D, 0);
	glDisable( GL_TEXTURE_2D );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glDisable( GL_CULL_FACE );

    SIZE sz;
    GetTextExtentPoint32( g_DContext, text, strlen(text), &sz );

	TEXTMETRIC tm;
	GetTextMetrics( g_DContext, &tm );
	sz.cy = tm.tmAscent;

	/*
	// Custom font generation
	glNewList( list+0, GL_COMPILE );

	glBegin( GL_TRIANGLES );
        glColor4ub( 0,0,255,255 );
        glVertex2f( x, y );
        glVertex2f( x+sz.cx, y );
        glVertex2f( x, y+sz.cy );
    glEnd();
    glTranslatef( sz.cx, 0, 0 );

    glEndList();
    glNewList( list+1, GL_COMPILE );

	glBegin( GL_TRIANGLES );
        glColor4ub( 0,255,0,255 );
        glVertex2f( x, y );
        glVertex2f( x+sz.cx, y );
        glVertex2f( x, y+sz.cy );
    glEnd();
    glTranslatef( sz.cx, 0, 0 );

    glEndList();
    glNewList( list+2, GL_COMPILE );

	glBegin( GL_TRIANGLES );
        glColor4ub( 255,255,0,255 );
        glVertex2f( x, y );
        glVertex2f( x+sz.cx, y );
        glVertex2f( x, y+sz.cy );
    glEnd();
    glTranslatef( sz.cx, 0, 0 );

    glEndList();*/

	//Actual text
    glColor4ubv( (GLubyte*)&color );
   // glColor4f( 1,1,1,1 );
    glRasterPos3f( x, y + ( sz.cy/2 ), z );

    glPushAttrib( GL_LIST_BIT );
    glListBase( g_DefaultFont );
    glCallLists( strlen(text), GL_UNSIGNED_BYTE, text );
    glPopAttrib();

}
