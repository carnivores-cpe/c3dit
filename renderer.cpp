
#include "Globals.h"
#include "header.h"
#include <vector>
#include <string>

using namespace std;

#define glIsSupported(ext) (bool(strstr((char*)glGetString(GL_EXTENSIONS),(char*)ext)!=NULL))


PFNGLACTIVETEXTUREARBPROC           glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC     glClientActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC         glMultiTexCoord2fARB;


typedef struct _GLvertex {
	float x,y,z;
} GLvertex;


int     A_Timer=0;
float   lAmbient[] = {0.05f, 0.05f, 0.05f, 1.0f};
float   lDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
float   lShine[]   = {1.0f, 0.0f, 0.0f, 1.0f};
float   lSun[]     = {0.0f, 0.0f, -1.0f, 0.0f};

GLvertex GridArrayVerts[64+64];
DWORD GridArrayColors[64+64];

int SplineD = 0;

HDC     g_DContext;
GLuint g_DefaultFont = 0;

Globals *Global = 0;


void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC, int cbits, int dbits )
{
	Global = GlobalVariables::SharedGlobalVariable();

    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    g_DContext = GetDC(hwnd);
    *hDC = g_DContext;

    if ( !g_DContext )
    {
        MessageBox(hwnd, "Failed to get the Device Context", "OpenGL", MB_OK);
    }

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
				  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = cbits;
    pfd.cDepthBits = dbits;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(g_DContext, &pfd);

    SetPixelFormat(g_DContext, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext( g_DContext );

    wglMakeCurrent(g_DContext, *hRC);

    //glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_COLOR_MATERIAL);

    //glDepthFunc(GL_LEQUAL);
    glDepthFunc( GL_LEQUAL );
    glAlphaFunc(GL_GEQUAL,0.75);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_DST_ALPHA);//--Normal

    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    // Lights
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lSun);
    glEnable(GL_LIGHT0);

    glClearDepth    (1.0);
    glClearColor    (0.0f, 0.0f, 0.0f, 1.0f);

    glGenTextures(3,g_TextureID);

    WORD SPECMAP[256*256];
	FILE *fp = fopen( Global->SpecularMap.c_str() ,"rb" );
	if ( fp )
	{
		fseek(fp,18,SEEK_SET);
		fread(SPECMAP,256*256*2,1,fp);
		fclose(fp);
		glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 256, 256, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, SPECMAP);
	}
	else printf( " OpenGL could not open: %s\n", Global->SpecularMap.c_str() );

    WORD ENVMAP[256*256];
    fp = fopen( Global->EnvironmentMap.c_str() ,"rb" );
	if ( fp )
	{
		fseek(fp,18,SEEK_SET);
		fread(ENVMAP,256*256*2,1,fp);
		fclose(fp);
		glBindTexture(GL_TEXTURE_2D, g_TextureID[2]);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 256, 256, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, ENVMAP);
	}
	else printf( " OpenGL could not open: %s\n", Global->EnvironmentMap.c_str() );
    /******************** END OF SPECIAL EFFECTS ********************************/

    if (glIsSupported("GL_EXT_texture_filter_anisotropic"))
    {
        /*float fAnisoX = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fAnisoX);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fAnisoX);*/
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (glIsSupported("GL_EXT_texture_env_combine"))
    {
        //MessageBox(hwnd, "GL_EXT_texture_env_combine Supported", "OpenGL", MB_OK);
    }

    if (glIsSupported("GL_ARB_multitexture"))
    {
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
        glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
        glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");

        GLint MaxTexLayers;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &MaxTexLayers);
        char str[256];
        sprintf(str,"MaxTexLayers: %d\n",MaxTexLayers);
        printf( " OpenGL:\n%s", str );
    }

    g_DefaultFont = glGenLists(255);
    if ( !g_DefaultFont )
    {
        MessageBox(hwnd, "Failed to generate font list.", "OpenGL", MB_OK);
    }

    /*HFONT font = CreateFont(-12,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
                OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
                FF_DONTCARE|DEFAULT_PITCH,"Courier New");*/
    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SelectObject( g_DContext, (HFONT)font );
    SelectObject( g_DContext, (HFONT)font );

	wglUseFontBitmaps( g_DContext,0,255, g_DefaultFont);
    //DeleteObject( font );


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

void RenderCARMesh()
{
    // Render Model

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);

    if ( ANIMPLAY && g_Animations[CUR_ANIM].FrameCount )
    {
        int AniTime = (g_Animations[CUR_ANIM].FrameCount * 1000) / g_Animations[CUR_ANIM].KPS;

        FTime += TimeDt;
        if ( FTime >= AniTime )
        FTime = 0;

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

    VERTEX l_RVerts[MAX_VERTICES];

    if (Model.num_anims > 0 && !strstr( g_Animations[CUR_ANIM].Name, "BLANK" ))
	{
		for (int v=0; v<Model.num_verts; v++)
		{
			short* adptr = &(g_Animations[CUR_ANIM].Data[CUR_FRAME * Model.num_verts * 3]);
			l_RVerts[v].mX = *(adptr + v*3+0) * k1 + *(adptr + (v+Model.num_verts)*3+0) * k2;
			l_RVerts[v].mY = *(adptr + v*3+1) * k1 + *(adptr + (v+Model.num_verts)*3+1) * k2;
			l_RVerts[v].mZ = *(adptr + v*3+2) * k1 + *(adptr + (v+Model.num_verts)*3+2) * k2;
		}

		// Normals
		if ( LIGHT )
		{
			bool vlist[MAX_VERTICES];
			ZeroMemory(vlist,sizeof(bool));
			for (int f=0; f<Model.num_tris; f++)
			{
				int v1 = g_Triangles[f].v1;
				int v2 = g_Triangles[f].v2;
				int v3 = g_Triangles[f].v3;
				g_Normals[f] = ComputeNormals(l_RVerts[v1],l_RVerts[v2],l_RVerts[v3]);
				if (!vlist[v1])
				{
					g_VNormals[v1] = g_Normals[f];
					vlist[v1] = true;
				}
				else
				{
					g_VNormals[v1] += g_Normals[f];
					g_VNormals[v1] /= 2;
				}
				if (!vlist[v2])
				{
					g_VNormals[v2] = g_Normals[f];
					vlist[v2] = true;
				}
				else
				{
					g_VNormals[v2] += g_Normals[f];
					g_VNormals[v2] /= 2;
				}
				if (!vlist[v3])
				{
					g_VNormals[v3] = g_Normals[f];
					vlist[v3] = true;
				}
				else
				{
					g_VNormals[v3] += g_Normals[f];
					g_VNormals[v3] /= 2;
				}
			}
		}
	}
    else
    {
		memcpy(l_RVerts, g_Verticies, sizeof(VERTEX) * Model.num_verts);
	}

	/*glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	gluPerspective( 45, 1.3333f, 1, 100.0f );*/


    for (int t=0; t<Model.num_tris; t++)
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

        //glNormal3fv(g_Normals[t].n);
        glNormal3fv(g_VNormals[v1].n);
        glTexCoord2f(g_Triangles[t].tx1/256.0f,g_Triangles[t].ty1/256.0f);
        glVertex3f(l_RVerts[v1].mX,l_RVerts[v1].mY,l_RVerts[v1].mZ);

        glNormal3fv(g_VNormals[v2].n);
        glTexCoord2f(g_Triangles[t].tx2/256.0f,g_Triangles[t].ty2/256.0f);
        glVertex3f(l_RVerts[v2].mX,l_RVerts[v2].mY,l_RVerts[v2].mZ);

        glNormal3fv(g_VNormals[v3].n);
        glTexCoord2f(g_Triangles[t].tx3/256.0f,g_Triangles[t].ty3/256.0f);
        glVertex3f(l_RVerts[v3].mX,l_RVerts[v3].mY,l_RVerts[v3].mZ);

        glEnd();
    }

	// Specular
	if ( DRAW_SPECULAR )
	{
		glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

		glEnable(GL_BLEND);
		glBlendFunc( GL_DST_ALPHA, GL_ONE );

		for (int t=0; t<Model.num_tris; ++t)
		{
			if (g_Triangles[t].flags & sfDoubleSide) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);

			int v1 = g_Triangles[t].v1;
			int v2 = g_Triangles[t].v2;
			int v3 = g_Triangles[t].v3;

			glBegin(GL_TRIANGLES);

			glColor4ub(255,255,255, 255);

			if (g_Triangles[t].flags & sfPhong)
			{
				glTexCoord2f( 100,100 );

				glNormal3fv(g_VNormals[v1].n);
				//glTexCoord2f(g_Triangles[t].tx1/256.0f,g_Triangles[t].ty1/256.0f);
				glVertex3f(l_RVerts[v1].mX,l_RVerts[v1].mY,l_RVerts[v1].mZ);

				glNormal3fv(g_VNormals[v2].n);
				//glTexCoord2f(g_Triangles[t].tx2/256.0f,g_Triangles[t].ty2/256.0f);
				glVertex3f(l_RVerts[v2].mX,l_RVerts[v2].mY,l_RVerts[v2].mZ);

				glNormal3fv(g_VNormals[v3].n);
				//glTexCoord2f(g_Triangles[t].tx3/256.0f,g_Triangles[t].ty3/256.0f);
				glVertex3f(l_RVerts[v3].mX,l_RVerts[v3].mY,l_RVerts[v3].mZ);
			}
			glEnd();
		}
	}

	// EnvMap
	if ( DRAW_ENVMAP )
	{
		glBindTexture(GL_TEXTURE_2D, g_TextureID[2]);
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

		glEnable(GL_BLEND);
		glBlendFunc( GL_DST_ALPHA, GL_ONE );

		for (int t=0; t<Model.num_tris; t++)
		{
			if (g_Triangles[t].flags & sfDoubleSide) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);

			int v1 = g_Triangles[t].v1;
			int v2 = g_Triangles[t].v2;
			int v3 = g_Triangles[t].v3;

			glBegin(GL_TRIANGLES);

			glColor4ub(255,255,255, 255);

			if (g_Triangles[t].flags & sfEnvMap)
			{
				glTexCoord2f( 0,0 );

				glNormal3fv(g_VNormals[v1].n);
				//glTexCoord2f(g_Triangles[t].tx1/256.0f,g_Triangles[t].ty1/256.0f);
				glVertex3f(l_RVerts[v1].mX,l_RVerts[v1].mY,l_RVerts[v1].mZ);

				glNormal3fv(g_VNormals[v2].n);
				//glTexCoord2f(g_Triangles[t].tx2/256.0f,g_Triangles[t].ty2/256.0f);
				glVertex3f(l_RVerts[v2].mX,l_RVerts[v2].mY,l_RVerts[v2].mZ);

				glNormal3fv(g_VNormals[v3].n);
				//glTexCoord2f(g_Triangles[t].tx3/256.0f,g_Triangles[t].ty3/256.0f);
				glVertex3f(l_RVerts[v3].mX,l_RVerts[v3].mY,l_RVerts[v3].mZ);
			}
			glEnd();
		}
	}

	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glEnable(GL_TEXTURE_2D);
    glBindTexture ( GL_TEXTURE_2D, 0 );
    glDisable(GL_BLEND);

    // Bones
    glDisable(GL_DEPTH_TEST);
	if ( DRAW_BONES )
    for (int b=0; b<Model.num_bones; b++)
    {
        glPushMatrix();
        glTranslatef( -g_Bones[b].x, g_Bones[b].y, g_Bones[b].z );

        DrawSphere( 5.0f, 16, 0xFF0080FF );

        glPopMatrix();

		if ( DRAW_JOINTS )
        if (g_Bones[b].parent != -1)
        {
            int p = g_Bones[b].parent;
            DrawLine( -g_Bones[b].x, g_Bones[b].y, g_Bones[b].z, -g_Bones[p].x, g_Bones[p].y, g_Bones[p].z, 0xFF0000FF );
        }
    }
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth( 2.0f );
	GLint depthmask;
	glGetIntegerv( GL_DEPTH_WRITEMASK, &depthmask );
	glDepthMask( GL_FALSE );

    if (WIREFRAME)
    for (int t=0; t<Model.num_tris; t++)
    {
        int v1 = g_Triangles[t].v1;
        int v2 = g_Triangles[t].v2;
        int v3 = g_Triangles[t].v3;

        glBegin(GL_LINE_LOOP);

        glColor4ub(255,255,255, 255);

        glVertex3f(l_RVerts[v1].mX,l_RVerts[v1].mY,l_RVerts[v1].mZ);
        glVertex3f(l_RVerts[v2].mX,l_RVerts[v2].mY,l_RVerts[v2].mZ);
        glVertex3f(l_RVerts[v3].mX,l_RVerts[v3].mY,l_RVerts[v3].mZ);
        //glVertex3f(-g_Verticies[v3].x,g_Verticies[v3].z,g_Verticies[v3].y);

        glEnd();
    }
	glDepthMask( depthmask );
    glEnable(GL_TEXTURE_2D);
    if(LIGHT)glEnable(GL_LIGHTING);
    glLineWidth( 1.0f );


    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glPointSize(4);
    if (WIREFRAME)
    {
        glBegin(GL_POINTS);
        for (int v=0; v<Model.num_verts; v++)
        {
            glColor4ub(255,255,0, 255);

            //EZvector V1(g_Verticies[v].x,g_Verticies[v].z,g_Verticies[v].y);
            //if (rayIntersectsPoint(CamP,CamD,V1)) glColor4ub(255,0,0,255);

            glVertex3f(l_RVerts[v].mX,l_RVerts[v].mY,l_RVerts[v].mZ);
        }
        glEnd();
    }
    glPointSize(1);
    glEnable(GL_TEXTURE_2D);
    if (LIGHT)
    glEnable(GL_LIGHTING);
}

void RenderMesh()
{
    if (key['L'] && key[VK_SHIFT])
    {
        LIGHT=!LIGHT;

        if (LIGHT)
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
    if (key['W'] && key[VK_SHIFT]) WIREFRAME = !WIREFRAME;

    glLightfv(GL_LIGHT0, GL_AMBIENT, lAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lDiffuse);
    //glLightfv(GL_LIGHT0, GL_SHININESS, lShine);
    glLightfv(GL_LIGHT0, GL_POSITION, lSun);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    if (LIGHT) glEnable(GL_LIGHTING);

    glPushMatrix();

    glScalef(0.1f,0.1f,0.1f);
    glBindTexture(GL_TEXTURE_2D,g_TextureID[0]);

    RenderCARMesh();

    glPopMatrix();


	if ( DRAW_GRID )
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

		DrawLine( +16, 0, -16, +16, 0, +16, 0xFF202020 );
		DrawLine( -16, 0, +16, +16, 0, +16, 0xFF202020 );

		glPopMatrix();
	}

    glPushMatrix();

    //Pivot point
	if ( DRAW_AXES )
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

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
		if (LIGHT)
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
