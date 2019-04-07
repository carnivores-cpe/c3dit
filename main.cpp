#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE	0x0700
#ifdef WINVER
#undef WINVER
#endif
#define WINVER 0x0501

#include "Globals.h"
#include "header.h"
#include "version.h"
#include <vector>
#include <string>

using namespace std;


string statMessages[] = {
    "Holding the right mouse button lets you orbit the object!",
    "Scrolling the mouse wheel will zoom in and out of the scene!",
    "Holding the middle mouse button will let you pan the camera!",
    "Left clicking the scene will refocus the keyboard input!",
    "Make sure to save your files!  Backup frequently!",
    "The cake is a lie..."
};


void loadCAR(void);
void loadCAR(char*);
void newScene();
void mouseE();
void keyboardE();
void exportTexture();
void exportModel();
void importTexture();
void importTexture(char*);
void PlayWave();
void LoadCARData(char*);
void LoadC2OData(char*);
void Load3DFData(char*);
void LoadOBJData(char*);
void SaveEZJSOBJData(char *);


BOOL CALLBACK AniDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SndDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CarDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HTREEITEM TreeView_AddNode(HWND, HTREEITEM, const char*);
HTREEITEM TreeView_AddResource(HWND, HTREEITEM, const char*, LPARAM);


enum
{
    TV_MESHES = 0,
    TV_TEXTURES = 1,
    TV_ANIMATIONS = 2,
    TV_SOUNDS = 3,
    TV_SPRITES = 4,
    TV_TRIANGLES = 5,
    TV_VERTICES = 6,
    TV_BONES = 7,
};


Globals		*GlobalData;
char		szTitle[256];
INIFILE		*gIniFile = NULL;
char		str[255];       //For messages/text
int			mx,my;
vector<HTREEITEM> g_TVItems;
HTREEITEM   rootNodes[12];
RECT		g_TVRect;


void InitializeCommonControls()
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = 0;
    icex.dwICC |= ICC_ANIMATE_CLASS;
    icex.dwICC |= ICC_BAR_CLASSES;
    icex.dwICC |= ICC_COOL_CLASSES;
    icex.dwICC |= ICC_DATE_CLASSES;
    icex.dwICC |= ICC_HOTKEY_CLASS;
    icex.dwICC |= ICC_INTERNET_CLASSES;
    icex.dwICC |= ICC_LINK_CLASS;
    icex.dwICC |= ICC_LISTVIEW_CLASSES;
    icex.dwICC |= ICC_NATIVEFNTCTL_CLASS;
    icex.dwICC |= ICC_PAGESCROLLER_CLASS;
    icex.dwICC |= ICC_PROGRESS_CLASS;
    icex.dwICC |= ICC_STANDARD_CLASSES;
    icex.dwICC |= ICC_TAB_CLASSES;
    icex.dwICC |= ICC_TREEVIEW_CLASSES;
    icex.dwICC |= ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);
}

#ifdef _DEBUG
int main( int argc, char *argv[] )
{
	HINSTANCE hInstance = GetModuleHandle( NULL );
	LPSTR lpCmdLine = 0;
	int nCmdShow = SW_SHOW;
#else
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
#endif
    WNDCLASSEX  wcex;
    HWND        hwnd;
    HDC         hDC;
    HGLRC       hRC;
    MSG         msg;
    BOOL        bQuit = FALSE;
    RECT        rc;

	GlobalData = GlobalVariables::SharedGlobalVariable();

	char lpszExePath[MAX_PATH];
	GetModuleFileName( 0, lpszExePath, MAX_PATH );
	string gDirectory = lpszExePath;
	gDirectory.replace( gDirectory.find_last_of( "/\\" )+1, gDirectory.size() - gDirectory.find_last_of( "/\\" ) + 1, "" );
	
	gIniFile = new INIFILE( gDirectory + "settings.ini" );
			
	GlobalData->GameLocation = gIniFile->GetValueString( "General", "gamedir", "" );
	GlobalData->WorkingDirectory = gIniFile->GetValueString( "General", "workingdir", gDirectory );
	GlobalData->EnvironmentMap = gIniFile->GetValueString( "General", "envmap", "SPECULAR.TGA" );
	GlobalData->SpecularMap = gIniFile->GetValueString( "General", "specular", "ENVMAP.TGA" );

	WinX = gIniFile->GetValueInt( "Window", "left", 0 );
	WinY = gIniFile->GetValueInt( "Window", "top", 0 );
	WinW = gIniFile->GetValueInt( "Window", "width", 900 );
	WinH = gIniFile->GetValueInt( "Window", "height", 600 );

	GlobalData->BufferColorBits = gIniFile->GetValueInt( "OpenGL", "color", 16 );
	GlobalData->BufferDepthBits = gIniFile->GetValueInt( "OpenGL", "depth", 16 );
	GlobalData->UseMipMaps = gIniFile->GetValueInt( "OpenGL", "mipmaps", 1 );


    /* register window class */
	ZeroMemory( &wcex, sizeof( WNDCLASSEX ) );
    wcex.cbSize =			sizeof(WNDCLASSEX);
    wcex.style =			CS_OWNDC;
    wcex.lpfnWndProc =		WindowProc;
    wcex.hInstance =		hInstance;
    wcex.hIcon =			LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor =			LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground =	(HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName =		MAKEINTRESOURCE(IDR_MENU);
    wcex.lpszClassName =	"C3DIT_CLASS";
    wcex.hIconSm =			LoadIcon(NULL, IDI_APPLICATION);


    if (!RegisterClassEx(&wcex))
        return 0;

    InitializeCommonControls();

    /* create main window */
    sprintf(szTitle,"Carnivores 3D Editor [%u.%u.%u]", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE );
    hwnd = CreateWindowEx(  WS_EX_ACCEPTFILES,
                            wcex.lpszClassName,
                            szTitle,
                            WS_OVERLAPPEDWINDOW,
                            WinX, WinY,
                            WinW, WinH,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);

    //ShowWindow(hwnd, nCmdShow);
    g_hMain = hwnd;
	GetClientRect(hwnd,&rc);

    g_DrawArea = CreateWindow(
                     WC_STATIC,
                     NULL,
                     WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
                     rc.left,rc.top+16,rc.right-128,rc.bottom-48,
                     g_hMain,
                     NULL,
                     hInstance,
                     NULL
                 );

    g_AniTrack = CreateWindow(
                    TRACKBAR_CLASS,
                    NULL,
                    WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                    rc.left, rc.bottom-48, rc.right-128, 16,
                    g_hMain,
                    NULL,
                    hInstance,
                    NULL
                    );

    SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,1) );
    SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );

    g_FileView = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_TREEVIEW,
                     NULL,
                     WS_CHILD | WS_VISIBLE |
                     TVS_LINESATROOT | TVS_HASLINES |
                     TVS_HASBUTTONS | TVS_EDITLABELS,
                     rc.left+rc.right-128,rc.top+16,128,rc.bottom-32,
                     g_hMain,
                     NULL,
                     hInstance,
                     NULL
                 );

    SetRect(&g_TVRect, rc.right-128, 0, 128, 0);

    rootNodes[0] = (TreeView_AddNode(g_FileView, 0, "Mesh"));
    rootNodes[1] = (TreeView_AddNode(g_FileView, 0, "Textures"));
    rootNodes[2] = (TreeView_AddNode(g_FileView, 0, "Animations"));
    rootNodes[3] = (TreeView_AddNode(g_FileView, 0, "Sounds"));
    rootNodes[4] = (TreeView_AddNode(g_FileView, 0, "Sprites"));
    rootNodes[5] = (TreeView_AddNode(g_FileView, rootNodes[0], "Triangles"));
    rootNodes[6] = (TreeView_AddNode(g_FileView, rootNodes[0], "Vertices"));
    rootNodes[7] = (TreeView_AddNode(g_FileView, rootNodes[0], "Bones"));

    /* create the tool bar */
    gHTool = CreateWindowEx(0,
                            TOOLBARCLASSNAME,
                            NULL,
                            WS_CHILD | WS_VISIBLE | WS_BORDER,
                            0,0,0,0,
                            g_hMain,
                            (HMENU)IDM_TOOL,
                            hInstance,
                            NULL);

    /* create the status bar */
    gHStatus = CreateWindowEx(0,
                              STATUSCLASSNAME,
                              NULL,
                              WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                              0,0,0,0,
                              g_hMain,
                              (HMENU)IDM_STATUS,
                              hInstance,
                              NULL);

    /* useful for backwards compatability */
    SendMessage(gHTool,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

    /* prepare the toolbar buttons */
    TBBUTTON tbb[7];
    TBADDBITMAP tbab;

    tbab.hInst = hInstance;
    tbab.nID = ID_TB_IMAGE;

    SendMessage(gHTool, TB_ADDBITMAP, 4, (LPARAM)&tbab);

    /* Make and define the toolbar buttons */
    ZeroMemory(tbb, sizeof(tbb));
    tbb[0].iBitmap = 0;
    tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].idCommand = IDF_NEW;
    tbb[1].iBitmap = 1;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = TBSTYLE_BUTTON;
    tbb[1].idCommand = IDF_OPEN;
    tbb[2].iBitmap = 2;
    tbb[2].fsState = TBSTATE_ENABLED;
    tbb[2].fsStyle = TBSTYLE_BUTTON;
    tbb[2].idCommand = IDF_SAVEAS;

    tbb[3].iBitmap = 0;
    tbb[3].fsState = TBSTATE_ENABLED;
    tbb[3].fsStyle = TBSTYLE_SEP;
    tbb[3].idCommand = 0;

    tbb[4].iBitmap = 3;
    tbb[4].fsState = TBSTATE_ENABLED;
    tbb[4].fsStyle = TBSTYLE_BUTTON;
    tbb[4].idCommand = ID_ANIM_PLAY;
    tbb[5].iBitmap = 4;
    tbb[5].fsState = TBSTATE_ENABLED;
    tbb[5].fsStyle = TBSTYLE_BUTTON;
    tbb[5].idCommand = ID_ANIM_PAUSE;

    tbb[6].iBitmap = 0;
    tbb[6].fsState = TBSTATE_ENABLED;
    tbb[6].fsStyle = TBSTYLE_SEP;
    tbb[6].idCommand = 0;

    /*tbb[7].iBitmap = 5;
    tbb[7].fsState = TBSTATE_ENABLED;
    tbb[7].fsStyle = TBSTYLE_BUTTON;
    tbb[7].idCommand = ID_UPLOAD;*/

    SendMessage(gHTool, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);

    /* prepare the status bar */
    srand( timeGetTime() );
    int statwidths[] = {180, -1};
    SendMessage(gHStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int),(LPARAM)statwidths);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)"No Model!");
    SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[ rand() % 6 ].c_str()); //"Arrow keys rotate the camera..."

	int PrevW = WinW,
		PrevH = WinH;
	ShowWindow( g_hMain, SW_SHOW );

    /* enable OpenGL for the window */
    EnableOpenGL(g_DrawArea, &hDC, &hRC, GlobalData->BufferColorBits, GlobalData->BufferDepthBits );

	// There is a bug with ShowWindow, it sends a WM_SIZE without my permission
	SendMessage( g_hMain, WM_SIZE, 0, (LPARAM)MAKELPARAM(WinW,WinH) );
	WinW = PrevW;
	WinH = PrevH;
	
    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            // Get the mouse co-ords on the screen
            if (GetForegroundWindow()==g_hMain)
            {
                POINT ms;
                RECT rc;
                GetCursorPos(&ms);
                CurX = ms.x;
                CurY = ms.y;

                mouseE();
                keyboardE();

                /* OpenGL animation code goes here */

                glClearDepth(1.0f);
                glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Camera
                GetClientRect(g_DrawArea, &rc);
                glViewport( 0,0, rc.right-rc.left,rc.bottom-rc.top );

				if ( CameraView == VIEW_PERSPECTIVE )
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0f, (float)rc.right/(float)rc.bottom, 1.0f, 1000.0f);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					gluLookAt(cam.x,cam.z,cam.y,cam.xt,cam.zt,cam.yt,0.0f,1.0f,0.0f);
				}
				else if ( CameraView == VIEW_FIRSTPERSON )
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(60.0f, (float)rc.right/(float)rc.bottom, 1.0f, 1000.0f);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					gluLookAt(0,0,0, 0,0,1, 0.0f,1.0f,0.0f);
				}
				else
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glOrtho(0,rc.right, rc.bottom,0, 1000.0f, -1000.0f);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();

					glTranslatef( (rc.right/2) + ortho.xt, (rc.bottom/2) + ortho.yt, 0 );
					
					glRotatef( 180.0f, 0, 0, 1 );
					
					if ( CameraView == VIEW_RIGHT )
						glRotatef(-90.0f, 0, 1, 0 );
					if ( CameraView == VIEW_LEFT )
						glRotatef( 90.0f, 0,1,0 );
					if ( CameraView == VIEW_BACK )
						glRotatef( 180.0f, 0,1,0 );
					if ( CameraView == VIEW_FRONT )
						glRotatef( 0.0f, 0,1,0 );
					if ( CameraView == VIEW_TOP)
						glRotatef(-90.0f, 1,0,0 );
					if ( CameraView == VIEW_BOTTOM )
						glRotatef( 90.0f, 1,0,0 );


					glScalef( 16,16,16 );
				}

				double mvmatrix[16], projmatrix[16];
				int viewport[4];
				glGetIntegerv( GL_VIEWPORT, viewport );
				glGetDoublev( GL_MODELVIEW_MATRIX, mvmatrix );
				glGetDoublev( GL_PROJECTION_MATRIX, projmatrix );

                // Draw the 3d model (mesh) ... duh
                RenderMesh();

                // Draw text overlay
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(rc.left,rc.right,rc.bottom,rc.top, -1280,1280);

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

				if ( DRAW_AXES )
				{
					// -- Render a vertex in 2D space, mapped to the 3D location
					double _outX,_outY,_outZ;
					gluProject (5.5,0,0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
					DrawTextGL( (float)_outX, (float)rc.bottom-(float)_outY, 0, "X", 0xFF0000FF);
					gluProject (0,5.5,0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
					DrawTextGL( (float)_outX, (float)rc.bottom-(float)_outY, 0, "Y", 0xFF00FF00);
					gluProject (0,0,5.5, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
					DrawTextGL( (float)_outX, (float)rc.bottom-(float)_outY, 0, "Z", 0xFFFF0000);
				}

                if ( CameraView == VIEW_PERSPECTIVE ) DrawTextGL(13,13,0, "Perspective", 0xFF9000FF);
				if ( CameraView == VIEW_FIRSTPERSON ) DrawTextGL(13,13,0, "First Person", 0xFF9000FF);
				if ( CameraView == VIEW_LEFT ) DrawTextGL(13,13,0, "Left", 0xFF9000FF);
				if ( CameraView == VIEW_RIGHT ) DrawTextGL(13,13,0, "Right", 0xFF9000FF);
				if ( CameraView == VIEW_FRONT ) DrawTextGL(13,13,0, "Front", 0xFF9000FF);
				if ( CameraView == VIEW_BACK ) DrawTextGL(13,13,0, "Back", 0xFF9000FF);
				if ( CameraView == VIEW_TOP ) DrawTextGL(13,13,0, "Top", 0xFF9000FF);
				if ( CameraView == VIEW_BOTTOM ) DrawTextGL(13,13,0, "Bottom", 0xFF9000FF);

                if ( GetFocus() == g_hMain )
                {
                    glBegin( GL_LINE_LOOP );
                        glColor4ub(255,0,0,255);
                        glVertex2f( (float)rc.left+2,  (float)rc.top+3 );
                        glVertex2f( (float)rc.right-3, (float)rc.top+3 );
                        glVertex2f( (float)rc.right-3, (float)rc.bottom-3 );
                        glVertex2f( (float)rc.left+3,  (float)rc.bottom-3);
                    glEnd();
                }

                RealTime = timeGetTime();
                //RealTime/=4;
                TimeDt = RealTime - PrevTime;
                if (TimeDt<0) TimeDt = 10;
                if (TimeDt>10000) TimeDt = 10;
                if (TimeDt>1000) TimeDt = 1000;
                PrevTime = RealTime;

                if (GetTickCount()-LastTick >=1000)
                {
                    LastTick = GetTickCount();
                    FPS = Frames;
                    Frames = 0;
                }
                Frames++;

                //Make sure the title is modified
                if (Model.name[0]!=0)
                {
                    sprintf(str,"%s - %s - FPS: %d",szTitle,Model.name,FPS);
                    SetWindowText(g_hMain,str);
                }

                // Copy the buffer (Double Buffering)
                SwapBuffers(hDC);
                Sleep(1);
            }
            else
            {
                Sleep (100);
            }
        }
    }

    g_TVItems.clear();

	{
		gIniFile->SetValueString( "General", "gamedir", GlobalData->GameLocation );
		gIniFile->SetValueString( "General", "envmap", GlobalData->EnvironmentMap );
		gIniFile->SetValueString( "General", "specular", GlobalData->SpecularMap );
		gIniFile->SetValueInt( "General", "software", false );

		gIniFile->SetValueInt( "Window", "left", WinX );
		gIniFile->SetValueInt( "Window", "top", WinY );
		gIniFile->SetValueInt( "Window", "width", WinW );
		gIniFile->SetValueInt( "Window", "height", WinH );

		gIniFile->SetValueInt( "OpenGL", "color", GlobalData->BufferColorBits );
		gIniFile->SetValueInt( "OpenGL", "depth", GlobalData->BufferDepthBits );
		gIniFile->SetValueInt( "OpenGL", "mipmaps", GlobalData->UseMipMaps );
	}

	delete gIniFile;

    /* shutdown OpenGL */
    DisableOpenGL(g_DrawArea, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(g_FileView);
    DestroyWindow(g_DrawArea);
    DestroyWindow(hwnd);

    /* unregister class */
    UnregisterClass( wcex.lpszClassName, wcex.hInstance );

    return msg.wParam;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        HMENU hMenu, hResMenu;
        hMenu = GetMenu(g_hMain);
        hResMenu = GetSubMenu(hMenu, 1);
        CheckMenuItem(hResMenu, IDF_LIGHT, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_BONES, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_JOINTS, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_BONES, MF_CHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_JOINTS, MF_CHECKED | MF_BYCOMMAND);
		UpdateWindow( g_hMain );

        g_hAniDlg = CreateDialog(GetModuleHandle(NULL),
                                 MAKEINTRESOURCE(ID_dlg_ANI),hwnd, AniDlgProc);
        g_hSndDlg = CreateDialog(GetModuleHandle(NULL),
                                 MAKEINTRESOURCE(ID_dlg_SND),hwnd, SndDlgProc);
        g_hCarDlg = CreateDialog(GetModuleHandle(NULL),
                                 MAKEINTRESOURCE(ID_DLG_CAR),hwnd, CarDlgProc);
		g_hAbout  = CreateDialog(GetModuleHandle(NULL),
                                 MAKEINTRESOURCE(ID_ABOUT_DIALOG),hwnd, AboutDlgProc);

    }
    break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
    {
        WinW = LOWORD(lParam);
        WinH = HIWORD(lParam);
        //glViewport( 0, 0, LOWORD(lParam), HIWORD(lParam) );
        RECT rcTool,rcStatus,rcClient,rc;
        int iToolHeight,iStatusHeight;

        // Get Rects
        GetClientRect(g_hMain, &rcClient);
        GetWindowRect(g_FileView, &g_TVRect);

        // Size toolbar and get height
        gHTool = GetDlgItem(hwnd, IDM_TOOL);
        SendMessage(gHTool, TB_AUTOSIZE, 0, 0);
        GetWindowRect(gHTool, &rcTool);
        iToolHeight = rcTool.bottom - rcTool.top;

        // Size status bar and get height
        gHStatus = GetDlgItem(hwnd, IDM_STATUS);
        SendMessage(gHStatus, WM_SIZE, 0, 0);
        GetWindowRect(gHStatus, &rcStatus);
        iStatusHeight = rcStatus.bottom - rcStatus.top;

        // Calculate remaining height and size draw area
        SetRect(&rc, rcClient.left, rcClient.top+iToolHeight, rcClient.right - 200, rcClient.bottom-iToolHeight-iStatusHeight);
        MoveWindow(g_DrawArea, rc.left,rc.top, rc.right, rc.bottom-32, TRUE);

        MoveWindow(g_AniTrack, rc.left,rc.top+rc.bottom-32,rc.right,32, TRUE);

        // File Components View
        SetRect(&rc, rc.left+rc.right, rc.top, 200, rc.bottom);
        MoveWindow(g_FileView, rc.left,rc.top,rc.right,rc.bottom, TRUE);

        // Resize OpenGL
        RECT rcGL;
        GetClientRect(g_DrawArea, &rcGL);
    }
    break;

    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lParam)->code)
        {
            case TVN_SELCHANGED:
            {
                LPNM_TREEVIEW nmtv = (LPNM_TREEVIEW)lParam;
                HTREEITEM ParItem = TreeView_GetParent(g_FileView, nmtv->itemNew.hItem);

                if (ParItem == rootNodes[TV_TRIANGLES])
                {
                    // -- Selection
                    /*
                    g_TriSelection.push_back( ((UINT)nmtv->itemNew.lParam - (UINT)&g_Triangles[0]) / sizeof(TRIANGLE) );
                    */
                }

                if (ParItem == rootNodes[TV_ANIMATIONS]) // Animations
                {
                    CUR_ANIM = ((UINT)nmtv->itemNew.lParam - (UINT)&g_Animations[0]) / sizeof(TVtl);
					CUR_FRAME = 0;
                    SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, ( g_Animations[CUR_ANIM].FrameCount-1 ) ) );
                    SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );
                }

                if (ParItem == rootNodes[TV_SOUNDS]) // Animations
                {
                    CUR_SOUND = ((UINT)nmtv->itemNew.lParam - (UINT)&g_Sounds[0]) / sizeof(SOUND);
                }
            }
            break;

            case NM_RCLICK:
            {
                LPNMHDR nmh = (LPNMHDR)lParam;
                if (nmh->hwndFrom == g_FileView)
                {
                    printf("NM_RCLICK:\n");

                    TVHITTESTINFO   tvhti;
                    HTREEITEM       SelItem, ParItem;
                    POINT           p;

                    GetCursorPos(&tvhti.pt);
                    ScreenToClient(g_FileView, &tvhti.pt);

                    SelItem = TreeView_HitTest(g_FileView, &tvhti);//MAX_VERTICES
                    ParItem = TreeView_GetParent(g_FileView, SelItem);

                    if (ParItem == rootNodes[TV_BONES]) // Mesh
                    {
                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, "Remove");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (SelItem == rootNodes[TV_ANIMATIONS]) // Animations
                    {
                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_CLEAR_ANIM, "Clear");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_ADD_ANIM, "Add");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (SelItem == rootNodes[TV_SOUNDS]) // Sounds
                    {
                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_CLEAR_SOUND, "Clear");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_ADD_SOUND, "Add");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (ParItem == rootNodes[TV_ANIMATIONS]) // Animations
                    {
                        TVITEM ti;
                        ZeroMemory(&ti, sizeof(TVITEM));
                        ti.hItem = SelItem;
                        ti.mask = TVIF_HANDLE | TVIF_PARAM;
                        TreeView_GetItem(g_FileView, &ti);

                        CUR_ANIM = ((UINT)ti.lParam - (UINT)&g_Animations[0]) / sizeof(TVtl);
                        printf("SelAnim = %u\n", CUR_ANIM);

                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_RENAME_ANIM, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_APLAY, "Play");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_REMOVE_ANIM, "Remove");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_INSERT_ANIM, "Insert");
						InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_PROP_ANIM, "Properties");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (ParItem == rootNodes[TV_SOUNDS]) // Sounds
                    {
                        TVITEM ti;
                        ZeroMemory(&ti, sizeof(TVITEM));
                        ti.hItem = SelItem;
                        ti.mask = TVIF_HANDLE | TVIF_PARAM;
                        TreeView_GetItem(g_FileView, &ti);

                        CUR_SOUND = ((UINT)ti.lParam - (UINT)&g_Sounds[0]) / sizeof(SOUND);

                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_RENAME_SOUND, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_SPLAY, "Play");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, "Remove");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, "Insert");
						InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_PROP_SOUND, "Properties");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                }
            }
            break;

            case TVN_ENDLABELEDIT:
            {
                LPNMTVDISPINFO ed = (LPNMTVDISPINFO)lParam;
                TVITEM ti;

                ZeroMemory(&ti, sizeof(TVITEM));
                ti.hItem = ed->item.hItem;
                ti.mask = TVIF_HANDLE | TVIF_PARAM;

                TreeView_GetItem(g_FileView, &ti);

                // -- Animations
                if ((LPARAM)&g_Animations[0] <= ti.lParam <= (LPARAM)&g_Animations[Model.num_anims])
                {
                    size_t len = strlen(ed->item.pszText);
                    TVtl* lpAnim = (TVtl*)ti.lParam;
                    strncpy( lpAnim->Name, ed->item.pszText, (len<=32) ? strlen(ed->item.pszText) : 32 );
                }
                // -- Sounds
                if ((LPARAM)&g_Sounds[0] <= ti.lParam <= (LPARAM)&g_Sounds[Model.num_sounds])
                {
                    size_t len = strlen(ed->item.pszText);
                    SOUND* lpSound = (SOUND*)ti.lParam;
                    strncpy( lpSound->Name, ed->item.pszText, (len<=32) ? strlen(ed->item.pszText) : 32 );
                }

                ti = ed->item;
                TreeView_SetItem(g_FileView, &ti);
            }
            break;

        }
    }
    break;

    case WM_MOVE:
        WinX = LOWORD(lParam);
        WinY = HIWORD(lParam);
        break;

    case WM_DESTROY:
		{
			DestroyWindow(embedTest);
			DestroyWindow(g_hAniDlg);
			DestroyWindow(g_hSndDlg);
			DestroyWindow(g_hCarDlg);
		}
        return 0;

    case WM_COMMAND:
    {
        switch (wParam)
        {
        case MPM_ADD_ANIM:
			{
				strcpy( g_Animations[ Model.num_anims ].Name, "BLANK" );
				g_TVItems.push_back( TreeView_AddResource( g_FileView, rootNodes[ TV_ANIMATIONS ], g_Animations[ Model.num_anims ].Name, LPARAM( &g_Animations[ Model.num_anims ] ) ));
				++Model.num_anims;
			}
            break;
		case MPM_ADD_SOUND:
			{
				strcpy( g_Sounds[ Model.num_sounds ].Name, "BLANK" );
				g_TVItems.push_back( TreeView_AddResource( g_FileView, rootNodes[ TV_SOUNDS ], g_Sounds[ Model.num_sounds ].Name, LPARAM( &g_Sounds[ Model.num_sounds ] ) ));
				++Model.num_sounds;
			}
			break;

        case IDF_NEW:
                newScene();
            break;

        case IDF_OPEN:
                loadCAR();
            break;

        case IDF_SAVEAS:
                SaveProject();
            break;

        case IDF_ABOUT:
			{
				ShowWindow( g_hAbout, SW_SHOW );
			}
			break;

        case IDF_UPDATE:
        {

        }
        break;

        case IDF_EXIT:
            PostQuitMessage(0);
            break;

        case IDF_FLAGS:
        {
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 1);

            COLOR = !COLOR;
            if (COLOR)
                CheckMenuItem(hResMenu, IDF_FLAGS, MF_CHECKED | MF_BYCOMMAND);
            else
                CheckMenuItem(hResMenu, IDF_FLAGS, MF_UNCHECKED | MF_BYCOMMAND);
        }
        break;

        case IDF_WIRE:
        {
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 1);

            WIREFRAME = !WIREFRAME;
            if (WIREFRAME)
                CheckMenuItem(hResMenu, IDF_WIRE, MF_CHECKED | MF_BYCOMMAND);
            else
                CheckMenuItem(hResMenu, IDF_WIRE, MF_UNCHECKED | MF_BYCOMMAND);
        }
        break;

        case IDF_LIGHT:
        {
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 1);

            LIGHT = !LIGHT;
            if (LIGHT)
            {
                glEnable(GL_LIGHTING);
                CheckMenuItem(hResMenu, IDF_LIGHT, MF_CHECKED | MF_BYCOMMAND);
            }
            else
            {
                glDisable(GL_LIGHTING);
                CheckMenuItem(hResMenu, IDF_LIGHT, MF_UNCHECKED | MF_BYCOMMAND);
            }
        }
        break;

		case IDV_BONES:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( DRAW_BONES == true )
				DRAW_BONES = false;
			else
				DRAW_BONES = true;

			if (DRAW_BONES)
			{
				CheckMenuItem(hResMenu, IDV_BONES, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_BONES, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_JOINTS:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);
			
			if ( DRAW_JOINTS == true )
				DRAW_JOINTS = false;
			else
				DRAW_JOINTS = true;

			if (DRAW_JOINTS)
			{
				CheckMenuItem(hResMenu, IDV_JOINTS, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_JOINTS, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_AXES:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( DRAW_AXES == true )
				DRAW_AXES = false;
			else
				DRAW_AXES = true;

			if (DRAW_AXES)
			{
				CheckMenuItem(hResMenu, IDV_AXES, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_AXES, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_GRID:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( DRAW_GRID == true )
				DRAW_GRID = false;
			else
				DRAW_GRID = true;

			if (DRAW_GRID)
			{
				CheckMenuItem(hResMenu, IDV_GRID, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_GRID, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_SPECULAR:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( DRAW_SPECULAR == true )
				DRAW_SPECULAR = false;
			else
				DRAW_SPECULAR = true;

			if (DRAW_SPECULAR)
			{
				CheckMenuItem(hResMenu, IDV_SPECULAR, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_SPECULAR, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_ENVMAP:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( DRAW_ENVMAP == true )
				DRAW_ENVMAP = false;
			else
				DRAW_ENVMAP = true;

			if (DRAW_ENVMAP)
			{
				CheckMenuItem(hResMenu, IDV_ENVMAP, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_ENVMAP, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

        case IDT_FLIPTRIS:
        {
            for (int t=0; t<Model.num_tris; t++)
            {
                int v1 = g_Triangles[t].v1;
                int v2 = g_Triangles[t].v2;
                int v3 = g_Triangles[t].v3;
                g_Triangles[t].v1 = v3;
                g_Triangles[t].v2 = v2;
                g_Triangles[t].v3 = v1;

                int tx1 = g_Triangles[t].tx1;
                int ty1 = g_Triangles[t].ty1;
                int tx2 = g_Triangles[t].tx2;
                int ty2 = g_Triangles[t].ty2;
                int tx3 = g_Triangles[t].tx3;
                int ty3 = g_Triangles[t].ty3;
                g_Triangles[t].tx1 = tx3;
                g_Triangles[t].ty1 = ty3;
                g_Triangles[t].tx2 = tx2;
                g_Triangles[t].ty2 = ty2;
                g_Triangles[t].tx3 = tx1;
                g_Triangles[t].ty3 = ty1;
            }

            // Normals
            bool vlist[MAX_VERTICES];
            ZeroMemory(vlist,sizeof(bool));
            for (int f=0; f<Model.num_tris; f++)
            {
                int v1 = g_Triangles[f].v1;
                int v2 = g_Triangles[f].v2;
                int v3 = g_Triangles[f].v3;
                g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
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
        break;

		case IDT_FLIPUV:
		{
			for ( int i=0; i<Model.num_tris; ++i )
			{
				g_Triangles[i].ty1 = 256 - g_Triangles[i].ty1;
				g_Triangles[i].ty2 = 256 - g_Triangles[i].ty2;
				g_Triangles[i].ty3 = 256 - g_Triangles[i].ty3;
			}
		}
		break;

        case IDF_EXTEX:
            exportTexture();
            break;

        case IDF_EXMOD:
            exportModel();
            break;

        case IDF_IMMOD:
            importModel();
            break;

        case IDF_IMTEX:
            importTexture();
            break;

		case MPM_PROP_ANIM:
        case IDF_ANI:
        {
            ShowWindow(g_hAniDlg, SW_SHOW);
            if (Model.num_anims>0)
            {
                unsigned int KPS,AniTime,FrameCount;

                KPS =           g_Animations[CUR_ANIM].KPS;
                FrameCount =    g_Animations[CUR_ANIM].FrameCount;
				AniTime =       KPS?(FrameCount * 1000) / KPS:0;

                SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
                SetDlgItemInt(g_hAniDlg, ID_AKPS, g_Animations[CUR_ANIM].KPS, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_ATIME, AniTime, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_AFRM, g_Animations[CUR_ANIM].FrameCount, FALSE);
            }
        }
        break;

		case MPM_PROP_SOUND:
        case IDF_SND:
        {
            ShowWindow(g_hSndDlg, SW_SHOW);
            sprintf(str,"%d Kbs", (int)g_Sounds[CUR_SOUND].len/1000);

            SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[CUR_SOUND].Name);
            SetDlgItemText(g_hSndDlg, ID_SIZE, str);
        }
        break;

        case IDF_PROPERTIES:
        {
            char sz[256];
            ShowWindow(g_hCarDlg, SW_SHOW);

            SetDlgItemText(g_hCarDlg, ID_NAME, Model.name);
            sprintf(sz, "%d", Model.oInfo.Radius);
            SetDlgItemText(g_hCarDlg, ID_O_RADIUS, sz);
            sprintf(sz, "%d", Model.oInfo.YLo);
            SetDlgItemText(g_hCarDlg, ID_O_YLO, sz);
            sprintf(sz, "%d", Model.oInfo.YHi);
            SetDlgItemText(g_hCarDlg, ID_O_YHI, sz);
        }
        break;

        case IDB_SPLAY:
        {
            PlayWave(&g_Sounds[CUR_SOUND]);
        }
        break;



		case IDV_PERSPECTIVE:
			CameraView = VIEW_PERSPECTIVE;
			break;
		case IDV_PERSPECTIVE+1:
			CameraView = VIEW_LEFT;
			break;
		case IDV_PERSPECTIVE+2:
			CameraView = VIEW_RIGHT;
			break;
		case IDV_PERSPECTIVE+3:
			CameraView = VIEW_FRONT;
			break;
		case IDV_PERSPECTIVE+4:
			CameraView = VIEW_BACK;
			break;
		case IDV_PERSPECTIVE+5:
			CameraView = VIEW_TOP;
			break;
		case IDV_PERSPECTIVE+6:
			CameraView = VIEW_BOTTOM;
			break;
		case IDV_PERSPECTIVE+7:
			CameraView = VIEW_FIRSTPERSON;
			break;

        case ID_ANIM_PLAY:
        {
            ANIMPLAY = TRUE;
        }
        break;

        case ID_ANIM_PAUSE:
        {
            ANIMPLAY = FALSE;
        }
        break;

        break;
        }
    }
    break;

    case WM_MOUSEWHEEL:
    {
        float zDelta = (float)(GET_WHEEL_DELTA_WPARAM(wParam)) / (float)(WHEEL_DELTA);
        cam.dist -= zDelta;
    }
    break;

    case WM_KEYDOWN:
    {
        key[wParam]=TRUE;

        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
    }
    break;

    case WM_KEYUP:
        key[wParam]=FALSE;
        break;

    case WM_DROPFILES:
    {
        HDROP drop = (HDROP) wParam;
        char file[MAX_PATH];

        for (UINT i=0; i<DragQueryFile(drop, 0xFFFFFFFF, file, MAX_PATH); i++)
        {
            if (!DragQueryFile(drop, i, file, MAX_PATH)) continue;
			//MessageBox(hwnd, file, "Dropped", MB_OK);
        }

        DragQueryFile(drop, 0, file, MAX_PATH);

        DragFinish(drop);

		char temp[MAX_PATH];
		strcpy( temp, file );
		_strupr( temp );

		if ( strstr(temp, ".CAR") || strstr(temp, ".3DF") || strstr(temp, ".C2O") || strstr(temp, ".OBJ"))
        {
			loadCAR(file);
		}
		if ( strstr(temp, ".BMP") || strstr(temp, ".TGA") || strstr(temp, ".DIB"))
        {
			importTexture(file);
		}
    }
    break;

    case WM_LBUTTONDOWN:
        mouse[0]=TRUE;
        break;
    case WM_LBUTTONUP:
        mouse[0]=FALSE;
        break;
    case WM_MBUTTONDOWN:
        mouse[1]=TRUE;
        break;
    case WM_MBUTTONUP:
        mouse[1]=FALSE;
        break;
    case WM_RBUTTONDOWN:
        mouse[2]=TRUE;
        break;
    case WM_RBUTTONUP:
        mouse[2]=FALSE;
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

BOOL CALLBACK AniDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            g_AniDlgTrack = CreateWindow(
                    TRACKBAR_CLASS,
                    NULL,
                    WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                    rc.left+5, rc.top+144, rc.right-10, 24,
                    hwnd,
                    NULL,
                    GetModuleHandle(NULL),
                    NULL
                    );

            SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,g_Animations[CUR_ANIM].FrameCount-1) );
            SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );
        }
        break;

        case WM_SHOWWINDOW:
        {

            SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,g_Animations[CUR_ANIM].FrameCount-1) );
            SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );
        }
        break;

    case WM_CLOSE:
        ShowWindow(g_hAniDlg, SW_HIDE);
        break;

        case WM_NOTIFY:
        {

        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_CLOSE:
			{
				GetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name, 32);
				g_Animations[CUR_ANIM].KPS = GetDlgItemInt(g_hAniDlg, ID_AKPS, NULL, FALSE);
				ShowWindow(g_hAniDlg, SW_HIDE);

				TVITEM ti;
				HTREEITEM hti = TreeView_GetSelection( g_FileView );

				ti.hItem = hti;
				ti.pszText = g_Animations[ CUR_ANIM ].Name;
				ti.cchTextMax = strlen( g_Animations[ CUR_ANIM ].Name );
				ti.lParam = LPARAM( &g_Animations[ CUR_ANIM ] );
				ti.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;

                TreeView_SetItem(g_FileView, &ti);
			}
			break;

        case IDB_SAVE:
			{
				if (!SaveFileDlg("Vertex Table (*.vtl)\0*.vtl\0", "vtl"))
					break;

				FILE *afp;
				afp = fopen(fileName,"wb");
				if (afp==NULL)
					break;
				fwrite(&Model.num_verts,1,4,afp);
				fwrite(&g_Animations[CUR_ANIM].KPS,1,4,afp);
				fwrite(&g_Animations[CUR_ANIM].FrameCount,1,4,afp);
				//fwrite(&g_Animations[CUR_ANIM].Frame,(6*Model.num_verts)*g_Animations[CUR_ANIM].FrameCount,1,afp);
				fwrite( g_Animations[CUR_ANIM].Data, Model.num_verts * g_Animations[CUR_ANIM].FrameCount * 3, sizeof(short), afp );
				fclose(afp);
			}
			break;

        case IDB_OPEN:
			{
				if (!OpenFileDlg("Vertex Table (*.vtl)\0*.vtl\0", "vtl"))
					break;

				FILE *afp = fopen(fileName,"rb");

				if (afp==NULL)
					break;

				long num_verts = 0;

				fread(&num_verts,1,4,afp);

				if (num_verts != Model.num_verts)
				{
					fclose(afp);
					break;
				}

				fread(&g_Animations[CUR_ANIM].KPS,1,4,afp);
				fread(&g_Animations[CUR_ANIM].FrameCount,1,4,afp);

				if ( g_Animations[CUR_ANIM].Data == 0 )
				{
					delete [] g_Animations[CUR_ANIM].Data;
				}

				g_Animations[CUR_ANIM].Data = new short[ Model.num_verts * g_Animations[CUR_ANIM].FrameCount * 3 ];

				fread( g_Animations[CUR_ANIM].Data, Model.num_verts * g_Animations[CUR_ANIM].FrameCount * 3, sizeof(short), afp );

				fclose(afp);

				unsigned int time = (g_Animations[CUR_ANIM].FrameCount * 1000) / g_Animations[CUR_ANIM].KPS;

				SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
				SetDlgItemInt(g_hAniDlg, ID_AKPS, g_Animations[CUR_ANIM].KPS, FALSE);
				SetDlgItemInt(g_hAniDlg, ID_ATIME, time, FALSE);
				SetDlgItemInt(g_hAniDlg, ID_AFRM, g_Animations[CUR_ANIM].FrameCount, FALSE);
			}
			break;

		case IDF_UPDATE:
			{
				GetDlgItemText( g_hAniDlg, ID_NAME, g_Animations[ CUR_ANIM ].Name, 32 );
				g_Animations[ CUR_ANIM ].KPS = GetDlgItemInt( g_hAniDlg, ID_AKPS, NULL, FALSE );

				TVITEM ti;
				HTREEITEM hti = TreeView_GetSelection( g_FileView );

				ti.hItem = hti;
				ti.pszText = g_Animations[ CUR_ANIM ].Name;
				ti.cchTextMax = strlen( g_Animations[ CUR_ANIM ].Name );
				ti.lParam = LPARAM( &g_Animations[ CUR_ANIM ] );
				ti.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;

                TreeView_SetItem(g_FileView, &ti);
			}
			break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}


BOOL CALLBACK SndDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_CLOSE:
        ShowWindow(g_hSndDlg, SW_HIDE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_CLOSE:
			{
				GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[CUR_SOUND].Name, 32);
				ShowWindow(g_hSndDlg, SW_HIDE);
			}
			break;

        case IDB_OPEN:
			{
				importSound();
			}
			break;

        case IDB_SAVE:
			{
				exportSound();
			}
			break;

		case IDF_UPDATE:
			{
				GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[ CUR_SOUND ].Name, 32);
				//HTREEITEM hti = TreeView_GetSelection( g_FileView );
			}
			break;

        case IDB_SPLAY:
			{
				PlayWave(&g_Sounds[CUR_SOUND]);
			}
			break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK CarDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_CLOSE:
    {
        ShowWindow(g_hCarDlg, SW_HIDE);
        GetDlgItemText(g_hCarDlg, ID_NAME, Model.name, 24);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
		case 2:
			break;
            /*case IDB_CLOSE:
            {
                GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name, 32);
                ShowWindow(g_hSndDlg, SW_HIDE);
            }
            break;*/
		default:
			break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
	case WM_INITDIALOG:
		{
			HWND hcontrol = GetDlgItem( hwnd, ID_ABOUT_TEXT );
			
			// set the about text
			sprintf( str, "C3Dit (Designer 3 - D3)\r\nCarnivores 3D Editor - Version: %u.%u.%u\r\nBy James Ogden\r\nContact: rexhunter99@gmail.com", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE );
			SetWindowText( hcontrol, str );

			// Resize and position the control
			MoveWindow( hwnd, (GetSystemMetrics(SM_CXSCREEN)/2)-128,(GetSystemMetrics(SM_CYSCREEN)/2)-64, 256,128, true );

			RECT rc;
			GetClientRect( hwnd, &rc );

			MoveWindow( hcontrol, rc.left+5,rc.top+5,rc.right-10,rc.bottom-24-5, true );

			hcontrol = GetDlgItem( hwnd, ID_ABOUT_OK );

			MoveWindow( hcontrol, (rc.right/2)-20,rc.bottom-22, 40,20, true );
		}
		break;

	case WM_SHOWWINDOW:
		{
			// Resize and position the control
			MoveWindow( hwnd, (GetSystemMetrics(SM_CXSCREEN)/2)-128,(GetSystemMetrics(SM_CYSCREEN)/2)-64, 256,128, true );
		}
		break;

    case WM_CLOSE:
		{
			ShowWindow(hwnd, SW_HIDE);
		}
		break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case ID_ABOUT_OK:
            {
                ShowWindow(hwnd, SW_HIDE);
            }
            break;
		default:
			break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}



void LoadCARData(char* fname)
{
    FILE *fp = fopen(fname,"rb");
    if (fp==NULL)
    {
        MessageBox(NULL, "There was an error when opening the file.",
                   "FILE ERROR",
                   MB_ICONERROR | MB_OK);
    }
    ISCAR = TRUE;
    ISC2O = FALSE;

    newScene();

    //--Header
    fread(Model.name,1,24,fp);
    fseek(fp,24,SEEK_SET);
    fread(Model.msc,1,8,fp);
    fseek(fp,32,SEEK_SET);
    fread(&Model.num_anims,1,4,fp);
    fread(&Model.num_sounds,1,4,fp);
    fread(&Model.num_verts,1,4,fp);
    fread(&Model.num_tris,1,4,fp);
    fread(&Model.bytes_tex,1,4,fp);

    //Triangles
    for (int t=0; t<Model.num_tris; t++)
    {
        fread(&g_Triangles[t],64,1,fp);

        char str[256];
        sprintf(str,"Triangles #%d",t+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM( &g_Triangles[t] ) ));
    }

    //Verticies
    float hiZ=0.0f;
    for (int v=0; v<Model.num_verts; v++)
    {
        fread(&g_Verticies[v],16,1,fp);

        char str[256];
        sprintf(str,"Vertices #%d",v+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM( &g_Verticies[v] ) ));

        if (g_Verticies[v].mY>hiZ)
            hiZ = g_Verticies[v].mY;
    }

    bool vlist[MAX_VERTICES];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.num_tris; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
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

    // Set the camera to look at the middle of the model
    cam.zt = (hiZ/10.0f)/2.0f;

    //Texture
    fread(&g_Texture,Model.bytes_tex,1,fp);
    int TexH = (Model.bytes_tex / 2) / 256;

    g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TEXTURES], "Diffuse", LPARAM( g_Texture ) ));
    //TreeView_SetImageList

    for (int i=0; i<Model.bytes_tex/2; i++)
    {
        WORD w = g_Texture[i];
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 1;
        if (!B && !G && !R)
            A = 0;
        g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
    }

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 256, int(ceilf(float(TexH) / 256.0f) * 256), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g_Texture);

    //Animations
    for (int a=0; a<Model.num_anims; a++)
    {
        fread(&g_Animations[a].Name,32,1,fp);
        fread(&g_Animations[a].KPS,1,4,fp);
        fread(&g_Animations[a].FrameCount,1,4,fp);

        g_Animations[a].Data = new short[Model.num_verts * g_Animations[a].FrameCount * 3];
        fread( g_Animations[a].Data, Model.num_verts * g_Animations[a].FrameCount * 3, sizeof(short), fp );

        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[2], g_Animations[a].Name, LPARAM( &g_Animations[a] ) ));
    }
    SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,g_Animations[0].FrameCount-1) );
    SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );

    //Sound effects
    for (int s=0; s<Model.num_sounds; s++)
    {
        fread(&g_Sounds[s].Name,32,1,fp);
        fread(&g_Sounds[s].len,1,4,fp);

        if (g_Sounds[s].data) delete [] g_Sounds[s].data;
        g_Sounds[s].data = new BYTE[g_Sounds[s].len];

        fread(g_Sounds[s].data,g_Sounds[s].len,1,fp);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[3], g_Sounds[s].Name, LPARAM( &g_Sounds[s] ) ));
    }

    //Animation-Sound Table
    fread(&Model.AnimFX,64,4,fp);

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u", (UINT)Model.num_tris);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);

    // Enable Menu items
    /*HMENU hMenu, hResMenu;
    hMenu = GetMenu(g_hMain);
    hResMenu = GetSubMenu(hMenu, 1);
    EnableMenuItem(hResMenu, IDF_IMTEX, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_IMMOD, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_EXTEX, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_EXMOD, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_ANI, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_SND, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_PROPERTIES, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    hResMenu = GetSubMenu(hMenu, 0);
    EnableMenuItem(hResMenu, IDF_SAVEAS, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));*/

	UpdateWindow(g_FileView);
    UpdateWindow(g_hMain);
}



void LoadC2OData(char *fname)
{
    FILE *fp = fopen(fname,"rb");
    if (fp==NULL)
    {
        MessageBox(NULL, "There was an error when opening the file.",
                   "FILE ERROR",
                   MB_ICONERROR | MB_OK);
    }

    ISCAR = TRUE;
    ISC2O = FALSE;

	newScene();

    //--Header
    sprintf(Model.name, "<Object>");
    fread(&Model.oInfo,1,48,fp);
    fread(&Model.oInfo.res,1,16,fp);
    fread(&Model.num_verts,1,4,fp);
    fread(&Model.num_tris ,1,4,fp);
    fread(&Model.num_bones,1,4,fp);
    fread(&Model.bytes_tex,1,4,fp);

    //Triangles
    for (int t=0; t<Model.num_tris; t++)
    {
        fread(&g_Triangles[t],64,1,fp);

        char str[256];
        sprintf(str,"Triangles #%d",t+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM( &g_Triangles[t] ) ));
    }

    //Verticies
	float hiZ=0.0f;
    for (int v=0; v<Model.num_verts; v++)
    {
        fread(&g_Verticies[v],sizeof(VERTEX),1,fp);

        char str[256];
        sprintf(str,"Vertices #%d",v+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM( &g_Verticies[v] ) ));

        if (g_Verticies[v].mY>hiZ)
            hiZ = g_Verticies[v].mY;
    }

    // Normals
    bool vlist[MAX_VERTICES];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.num_tris; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
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

    // Set the camera to look at the middle of the model
    cam.zt = (hiZ/10.0f)/2.0f;

    //Bones
    for (int b=0; b<Model.num_bones; b++)
    {
        fread(&g_Bones[b],sizeof(BONE),1,fp);

        char str[256];
        //sprintf(str,"Bone #%d",b+1);
        sprintf(str,"%s",g_Bones[b].name);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_BONES], str, LPARAM( &g_Bones[b] ) ));
    }

    //Texture
    fread(&g_Texture,Model.bytes_tex,1,fp);

	g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TEXTURES], "Diffuse", LPARAM( g_Texture ) ));

    for (int i=0; i<Model.bytes_tex/2; i++)
    {
        WORD w = g_Texture[i];
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 1;
        if (!B && !G && !R)
            A = 0;
        g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
    }

	int TexH = (Model.bytes_tex / 2) / 256;

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 256, int(ceilf(float(TexH) / 256.0f) * 256), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g_Texture);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 3);

    //Sprite
    fread(&g_Sprite,128*128,2,fp);

	g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_SPRITES], "LoD #1", LPARAM( g_Sprite ) ));

    /*glBindTexture(GL_TEXTURE_2D, g_TextureID[1]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 128, 128, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g_Sprite);*/

    //Animations
    /*
        We can skip these for now since Objects don't
        have animations (though they support having animations)
    */

    fclose(fp);

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u", (UINT)Model.num_tris);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
}


void newScene()
{
    //--Empty TreeView
    for (UINT i=0; i<g_TVItems.size(); i++)
    {
        TreeView_DeleteItem(g_FileView, g_TVItems[i]);
    }
    g_TVItems.clear();

    //Empty the texture
    memset(g_Texture, 255, 256*1024*2 );
	glBindTexture( GL_TEXTURE_2D, g_TextureID[0] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, 256, 256, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g_Texture );

    //Empty the vertices
    ZeroMemory(g_Verticies,sizeof(VERTEX)*MAX_VERTICES);

    //Empty the triangles
    ZeroMemory(g_Triangles,sizeof(TRIANGLE)*MAX_TRIANGLES);

    //Empty the header
    ZeroMemory(&Model,sizeof(MODEL_HEADER));

	ANIMPLAY = false;
    CUR_ANIM = 0;
    CUR_FRAME = 0;
    CUR_SOUND = 0;

	SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)"No Model!");
    SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[ rand() % 6 ].c_str());

    cam.x = -24;
    cam.y = -24;
    cam.z = 24;
    cam.xt = 0;
    cam.yt = 0;
    cam.zt = 0;
    cam.dist = 24;
    cam.yaw = 0;
    cam.pitch = 0;
}


void loadCAR()
{
    /* loadCAR()
    * Loads one of the supported geometry mediums to a blank scene.
    * Supports:
        CAR - Character Format
        C2O - Carnivores 2 Object Format
        3DF - Action Forms 3D File Format
        OBJ - Wavefront OBJ Format
    */
    if (!OpenFileDlg("All Supported (*.car;*.c2o;*.3df;*.obj;)\0*.car;*.c2o;*.3df;*.obj\0"
                     "Character files (*.car)\0*.car\0"
                     "Carnivores 2 Object (*.c2o)\0*.c2o\0"
                     "3D File (*.3df)\0*.3df\0"
                     "Wavefront OBJ File (.obj)\0*.obj\0", "CAR"))
    return;

    newScene();

    if (strstr(_strlwr(fileName),".car")!=NULL)
    {
        LoadCARData(fileName);
    }
    if (strstr(_strlwr(fileName),".c2o")!=NULL)
    {
        LoadC2OData(fileName);
    }
	if (strstr(_strlwr(fileName),".c1o")!=NULL)
    {
        //LoadC1OData(fileName);
    }
    if (strstr(_strlwr(fileName),".3df")!=NULL)
    {
        Load3DFData(fileName);
    }
    if (strstr(strlwr(fileName),".obj")!=NULL)
    {
        LoadOBJData(fileName);
    }

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u",(UINT)Model.num_tris);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
    MessageBox(g_hMain,sstr,"triangles",MB_OK);
    return;
}

void loadCAR(char *fname)
{
    /* loadCAR()
    * Loads one of the supported geometry mediums to a blank scene.
    * Supports:
        CAR - Character Format
        C2O - Carnivores 2 Object Format
        3DF - Action Forms 3D File Format
        OBJ - Wavefront OBJ Format
    */

    newScene();

    if (strstr(_strlwr(fname),".car"))
    {
        LoadCARData(fname);
    }
    if (strstr(_strlwr(fname),".c2o"))
    {
        LoadC2OData(fname);
    }
    if (strstr(_strlwr(fname),".3df"))
    {
        Load3DFData(fname);
    }
    if (strstr(_strlwr(fname),".obj"))
    {
        LoadOBJData(fname);
    }

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u",(UINT)Model.num_tris);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
    return;
}

void SaveCARData(char *fname)
{
    FILE *fp = fopen(fname,"wb");

    // -- C3Dit watermark
    sprintf(Model.msc, "C3DIT");

    //--Header
    fwrite(Model.name,1,24,fp);
    fwrite(Model.msc,1,8,fp);
    fwrite(&Model.num_anims,1,4,fp);
    fwrite(&Model.num_sounds,1,4,fp);
    fwrite(&Model.num_verts,1,4,fp);
    fwrite(&Model.num_tris,1,4,fp);
    fwrite(&Model.bytes_tex,1,4,fp);

    //Triangles
    for (int t=0; t<Model.num_tris; t++)
    {
        fwrite(&g_Triangles[t],64,1,fp);
    }

    //Verticies
    for (int v=0; v<Model.num_verts; v++)
    {
        fwrite(&g_Verticies[v],16,1,fp);
    }

    //Texture
    for (int i=0; i<256*256; i++)
    {
        WORD w = g_Texture[i];
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 0;
        g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
    }
    fwrite(&g_Texture,Model.bytes_tex,1,fp);

    //Animations
    for (int a=0; a<Model.num_anims; a++)
    {
        fwrite(&g_Animations[a].Name,32,1,fp);
        fwrite(&g_Animations[a].KPS,1,4,fp);
        fwrite(&g_Animations[a].FrameCount,1,4,fp);
        fwrite( g_Animations[a].Data, Model.num_verts * g_Animations[a].FrameCount * 3, sizeof(short), fp );
        //fwrite(&g_Animations[a].Frame,(6*Model.num_verts)*g_Animations[a].FrameCount,1,fp);
    }

    //Sound effects
    for (int s=0; s<Model.num_sounds; s++)
    {
        fwrite(&g_Sounds[s].Name,32,1,fp);
        fwrite(&g_Sounds[s].len,1,4,fp);
        fwrite(g_Sounds[s].data,g_Sounds[s].len,1,fp);
    }

    //Animation-Sound Table
    fwrite(&Model.AnimFX,64,4,fp);

    //Close the file
    fclose(fp);
}


void SaveOBJData(char *fname)
{
    // -- Saves the mesh as a triangluated Wavefront OBJ
    FILE *fp = fopen(fname,"w");

    fprintf(fp, "# Created by C3Dit\n");
    fprintf(fp, "# C3Dit (c) Rexhunter99 2009\n\n");
    fprintf(fp, "# C3Dit - Wavefront OBJ Exporter Version: 1.0\n\n");

    fprintf(fp, "# Vertices: %u\n", (UINT)Model.num_verts);
    fprintf(fp, "# Triangles: %u\n", (UINT)Model.num_tris);

    fprintf(fp, "\ng %s\n\n", Model.name);

    for (int v=0; v<Model.num_verts; v++)
    {
        fprintf(fp, "v %f %f %f\n", g_Verticies[v].mX, g_Verticies[v].mZ, g_Verticies[v].mY );
    }

    for (int t=0; t<Model.num_tris; t++)
    {
        fprintf(fp, "vt %f %f 0.0\n",(float)g_Triangles[t].tx1/256.0f,(float)g_Triangles[t].ty1/256.0f);
        fprintf(fp, "vt %f %f 0.0\n",(float)g_Triangles[t].tx2/256.0f,(float)g_Triangles[t].ty2/256.0f);
        fprintf(fp, "vt %f %f 0.0\n",(float)g_Triangles[t].tx3/256.0f,(float)g_Triangles[t].ty3/256.0f);
    }

    for (int t=0; t<Model.num_tris; t++)
    {
        fprintf(fp, "f %u/%u %u/%u %u/%u\n",
                                        g_Triangles[t].v1+1, (t*3)+1,
                                        g_Triangles[t].v2+1, (t*3)+2,
                                        g_Triangles[t].v3+1, (t*3)+3 );
    }

    fclose(fp);

    return;
}

void SaveEZJSOBJData(char *fname)
{
    // -- Saves the mesh as an EpicZenVideo-JS primitive script
    FILE *fp = fopen(fname,"w");

    fprintf(fp, "function Create_%s_Primitive()\n{\n", Model.name);

    fprintf(fp, "\tmesh = new EZprimitive( EZPT_TRIANGLES );\n", Model.name);

    for (int t=0; t<Model.num_tris; t++)
    {
        fprintf(fp, "\tmesh.AddVertex( %f, %f, %f, [1.0, 1.0, 1.0, 1.0], null, [ %f, %f ] );\n",
			g_Verticies[ g_Triangles[t].v1 ].mX,
			g_Verticies[ g_Triangles[t].v1 ].mY,
			g_Verticies[ g_Triangles[t].v1 ].mZ,
			g_Triangles[t].tx1 / 255.0f,
			g_Triangles[t].ty1 / 255.0f);
		fprintf(fp, "\tmesh.AddVertex( %f, %f, %f, [1.0, 1.0, 1.0, 1.0], null, [ %f, %f ] );\n",
			g_Verticies[ g_Triangles[t].v2 ].mX,
			g_Verticies[ g_Triangles[t].v2 ].mY,
			g_Verticies[ g_Triangles[t].v2 ].mZ,
			g_Triangles[t].tx2 / 255.0f,
			g_Triangles[t].ty2 / 255.0f);
		fprintf(fp, "\tmesh.AddVertex( %f, %f, %f, [1.0, 1.0, 1.0, 1.0], null, [ %f, %f ] );\n",
			g_Verticies[ g_Triangles[t].v3 ].mX,
			g_Verticies[ g_Triangles[t].v3 ].mY,
			g_Verticies[ g_Triangles[t].v3 ].mZ,
			g_Triangles[t].tx3 / 255.0f,
			g_Triangles[t].ty3 / 255.0f);
    }

	fprintf( fp, "\tmesh.End();\n" );
	fprintf( fp, "}\n" );

    fclose(fp);

    return;
}

void LoadOBJData(char *fname)
{
    // -- Supports Triangulated (preferred) or quadulated geometry

    UINT num_texcoords = 0;
    vector<VERTEX> l_TexCoord;

    // -- Clear the scene
    newScene();

    FILE *fp = fopen(fname,"r");

    while (!feof(fp))
    {
        // # == comment
        // \n == newline
        // \r\n == newline
        // \0 == newline
        // v == vertex
        // vn == vertex normal
        // vt == vertex texture coord
        // f == face

        char str[256];
        fgets(str, 256, fp);
        // -- End Of Line reached

        char *tok = strtok( str, " " );
        //tok = strtok( NULL, " " );

        if (tok[0] == '#') continue;
        if (tok[0] == '\n') continue;
		if (tok[0] == '\0') continue;
        // skip unused tokens
        if (tok[0] == 's') continue;

        if (strcmp(tok, "mtllib")==0) continue;
        if (strcmp(tok, "usemtl")==0) continue;

        // Group
        if (tok[0] == 'g' || tok[0] == 'o')
        {
            tok = strtok( NULL, " " );

            strncpy(Model.name, tok, 32);
        }

        // Vertex
        if (tok[0] == 'v' && tok[1] == '\0')
        {
            if ( Model.num_verts >= MAX_VERTICES ) continue;

            g_Verticies[Model.num_verts].mX = atof( tok = strtok( NULL, " " ) );
            g_Verticies[Model.num_verts].mY = atof( tok = strtok( NULL, " " ) );
            g_Verticies[Model.num_verts].mZ = atof( tok = strtok( NULL, " " ) );
            char name[256];
            sprintf(name,"Vertices #%u", (UINT)Model.num_verts);
            g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], name, LPARAM( &g_Verticies[Model.num_verts] ) ));
            Model.num_verts++;
        }

        // Texture Coord
        if (tok[0] == 'v' && tok[1] == 't')
        {
            VERTEX Coords;
            Coords.mX = atof( tok = strtok( NULL, " " ) );
            Coords.mY = atof( tok = strtok( NULL, " " ) );
            l_TexCoord.push_back( Coords );

            num_texcoords++;
        }

        // Normal
        if (tok[0] == 'v' && tok[1] == 'n')
        {
            continue;
        }

        // Face
        if (tok[0] == 'f' && tok[1] == '\0')
        {
            if ( Model.num_tris >= MAX_VERTICES ) continue;
            char* slash;

            tok = strtok( NULL, " " );
			if ( tok == 0 ) continue;
            g_Triangles[Model.num_tris].v1 = atoi(&tok[0]) - 1;
            if ( tok )
            if ( slash = strchr(tok, '/') )
            {
                g_Triangles[Model.num_tris].tx1 = l_TexCoord[atoi(&slash[1]) - 1].mX * 256;
                g_Triangles[Model.num_tris].ty1 = l_TexCoord[atoi(&slash[1]) - 1].mY * 256;
            }

            tok = strtok( NULL, " " );
            g_Triangles[Model.num_tris].v2 = atoi(&tok[0]) - 1;
            if ( tok )
            if ( slash = strchr(tok, '/') )
            {
                g_Triangles[Model.num_tris].tx2 = l_TexCoord[atoi(&slash[1]) - 1].mX * 256;
                g_Triangles[Model.num_tris].ty2 = l_TexCoord[atoi(&slash[1]) - 1].mY * 256;
            }

            tok = strtok( NULL, " " );
            g_Triangles[Model.num_tris].v3 = atoi(&tok[0]) - 1;
            if ( tok )
            if ( slash = strchr(tok, '/') )
            {
                g_Triangles[Model.num_tris].tx3 = l_TexCoord[atoi(&slash[1]) - 1].mX * 256;
                g_Triangles[Model.num_tris].ty3 = l_TexCoord[atoi(&slash[1]) - 1].mY * 256;
            }

            // -- Add-To-TreeView
            char name[256];
            sprintf(name,"Triangles #%u", (UINT)Model.num_tris);
            g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], name, LPARAM( &g_Triangles[Model.num_tris] ) ));
            Model.num_tris++;

            tok = strtok( NULL, " " );
            if ( tok != NULL && tok[0] != '\n' )
            {
                g_Triangles[Model.num_tris].v1 = g_Triangles[Model.num_tris-1].v2;
                g_Triangles[Model.num_tris].v2 = atoi(&tok[0]) - 1;
                g_Triangles[Model.num_tris].v3 = g_Triangles[Model.num_tris-1].v3;

                if ( tok )
                if ( slash = strtok(tok, "/") )
                {
                    g_Triangles[Model.num_tris].tx1 = g_Triangles[Model.num_tris-1].tx2;
                    g_Triangles[Model.num_tris].ty1 = g_Triangles[Model.num_tris-1].ty2;
                    g_Triangles[Model.num_tris].tx2 = l_TexCoord[atoi(&slash[1]) - 1].mX * 256;
                    g_Triangles[Model.num_tris].ty2 = l_TexCoord[atoi(&slash[1]) - 1].mY * 256;
                    g_Triangles[Model.num_tris].tx3 = g_Triangles[Model.num_tris-1].tx3;
                    g_Triangles[Model.num_tris].ty3 = g_Triangles[Model.num_tris-1].ty3;
                }

                // -- Add-To-TreeView
                char name[256];
                sprintf(name,"Triangles #%u", (UINT)Model.num_tris);
                g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], name, LPARAM( &g_Triangles[Model.num_tris] ) ));
                Model.num_tris++;
            }
        }
    }

    // Normals
    // were crashing tool :/

    fclose(fp);

    return;
}


void Load3DFData(char *fname)
{
    newScene();

    FILE *fp = fopen(fname,"rb");

    if ( !fp )
    {
        MessageBox( g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION );
        return;
    }

    //--Header
    fread(&Model.num_verts,1,4,fp);
    fread(&Model.num_tris,1,4,fp);
    fread(&Model.num_bones,1,4,fp);
    fread(&Model.bytes_tex,1,4,fp);

    if ( Model.num_verts == 0 )
    {
        MessageBox( g_hMain, "There are no vertices in this file!", "File Error", MB_ICONEXCLAMATION );
        return;
    }
    if ( Model.num_tris == 0 )
    {
        MessageBox( g_hMain, "There are no triangles in this file!", "File Error", MB_ICONEXCLAMATION );
        return;
    }


    //Triangles
    for (int t=0; t<Model.num_tris; t++)
    {
        fread(&g_Triangles[t],64,1,fp);
    }

    //Verticies
	float hiZ = 0.0f;
    for (int v=0; v<Model.num_verts; v++)
    {
        fread(&g_Verticies[v],16,1,fp);

		if (g_Verticies[v].mY>hiZ)
            hiZ = g_Verticies[v].mY;
    }
	cam.zt = (hiZ/10.0f)/2.0f;

    //Normals
    bool vlist[MAX_VERTICES];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.num_tris; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
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

    //Bones
    for (int b=0; b<Model.num_bones; b++)
    {
        fread(&g_Bones[b],sizeof(BONE),1,fp);
    }

    //Texture
    fread(g_Texture,Model.bytes_tex,1,fp);

	g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TEXTURES], "Diffuse", LPARAM( g_Texture ) ));

	for (int i=0; i<Model.bytes_tex/2; i++)
    {
        WORD w = g_Texture[i];
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 1;
        if (!B && !G && !R)
            A = 0;
        g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
    }

    int TexH = (Model.bytes_tex/2) / 256;

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 256, int(ceilf(float(TexH) / 256.0f) * 256), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g_Texture);

    //Close the file
    fclose(fp);
}

void Save3DFData(char *fname)
{
    //Pre-processing:
    if (Model.num_bones == 0)
    {
        bool bone[32];
        ZeroMemory(&bone, sizeof(bool) * 32);
        for (int v=0; v<Model.num_verts; v++)
        {
            if (bone[g_Verticies[v].mBone]==false)
            {
                bone[g_Verticies[v].mBone]=true;
                sprintf(g_Bones[Model.num_bones].name,"Bone #%u", (UINT)Model.num_bones);
                g_Bones[Model.num_bones].x=0;
                g_Bones[Model.num_bones].z=0;
                g_Bones[Model.num_bones].y = Model.num_bones * 2.0f;
                g_Bones[Model.num_bones].parent = -1;
                g_Bones[Model.num_bones].unknown = 0;
                Model.num_bones++;
            }
        }
    }
    //End Pre-processing

    FILE *fp = fopen(fname,"wb");

    //--Header
    fwrite(&Model.num_verts,1,4,fp);
    fwrite(&Model.num_tris,1,4,fp);
    fwrite(&Model.num_bones,1,4,fp);
    fwrite(&Model.bytes_tex,1,4,fp);

    //Triangles
    for (int t=0; t<Model.num_tris; t++)
    {
        fwrite(&g_Triangles[t],64,1,fp);
    }

    //Verticies
    for (int v=0; v<Model.num_verts; v++)
    {
        fwrite(&g_Verticies[v],16,1,fp);
    }

    //Bones
    for (int b=0; b<Model.num_bones; b++)
    {
        fwrite(g_Bones[b].name,1,32,fp);
        fwrite(&g_Bones[b].x,1,4,fp);
        fwrite(&g_Bones[b].y,1,4,fp);
        fwrite(&g_Bones[b].z,1,4,fp);
        fwrite(&g_Bones[b].parent,1,2,fp);
        fwrite(&g_Bones[b].unknown,1,2,fp);
    }

    //Texture
    for (int i=0; i<256*256; i++)
    {
        WORD w = g_Texture[i];
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 1;
        g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
    }
    fwrite(&g_Texture,Model.bytes_tex,1,fp);

    //Close the file
    fclose(fp);
}

void SaveC2OData(char *fname)
{
    FILE *fp = fopen(fname,"wb");

	if ( g_Animations[0].FrameCount ) Model.oInfo.flags |= ofANIMATED;

    //--Header
    fwrite(&Model.oInfo,1,48,fp);
    fwrite(&Model.oInfo.res,1,16,fp);
    fwrite(&Model.num_verts,1,4,fp);
    fwrite(&Model.num_tris ,1,4,fp);
    fwrite(&Model.num_bones,1,4,fp);
    fwrite(&Model.bytes_tex,1,4,fp);

    //Triangles
    for (int t=0; t<Model.num_tris; t++)
    {
        fwrite(&g_Triangles[t],64,1,fp);
    }

    //Verticies
    for (int v=0; v<Model.num_verts; v++)
    {
        fwrite(&g_Verticies[v],sizeof(VERTEX),1,fp);
    }

    //Bones
    for (int b=0; b<Model.num_bones; b++)
    {
        fwrite(&g_Bones[b],sizeof(BONE),1,fp);
    }

    //Texture
    for (int i=0; i<Model.bytes_tex; i++)
    {
        WORD w = g_Texture[i];
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 0;
        g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
    }
    fwrite(&g_Texture, Model.bytes_tex,1,fp);

    //Sprite
	if ( !(Model.oInfo.flags & ofNOBMP) )
    fwrite(&g_Sprite,128*128,2,fp);

    //Animation
	if ( g_Animations[0].FrameCount )
	{
		fwrite( &g_Animations[0].Name, 32, 1, fp );
		fwrite( &g_Animations[0].KPS, 4, 1, fp );
		fwrite( &g_Animations[0].FrameCount, 4, 1, fp );
		fwrite( g_Animations[0].Data, g_Animations[0].FrameCount * Model.num_verts * 6, 1, fp );
	}

    //Close the file
    fclose(fp);
}

void SaveProject()
{
    if (!SaveFileDlg("All Supported (*.car;*.c2o;*.3df;*.obj;)\0*.car;*.c2o;*.3df;*.obj\0"
                     "Character files (*.car)\0*.car\0"
                     "Carnivores 2 Object (*.c2o)\0*.c2o\0"
                     "3D File (*.3df)\0*.3df\0"
                     "Wavefront OBJ File (.obj)\0*.obj\0"
					 "EpicZen Javascript (.js)\0*.js\0", "car"))
    return;

    //--Find out what kind of file this is
    if (strstr(_strlwr(fileName),".car")!=NULL)
    {
        SaveCARData(fileName);
    }
    if (strstr(_strlwr(fileName),".c2o")!=NULL)
    {
        SaveC2OData(fileName);
    }
    if (strstr(_strlwr(fileName),".3df")!=NULL)
    {
        Save3DFData(fileName);
    }
    if (strstr(_strlwr(fileName),".obj")!=NULL)
    {
        SaveOBJData(fileName);
    }
	if ( strstr(_strlwr(fileName),".js") )
	{
		SaveEZJSOBJData(fileName);
	}
}


void keyboardE()
{
    if ( key[VK_SUBTRACT] )
        cam.dist += 0.5f;

    if ( key[VK_ADD] )
        cam.dist -= 0.5f;

    if ( key[VK_LEFT] )
        cam.yaw += 1.0f;

    if ( key[VK_RIGHT] )
        cam.yaw -= 1.0f;

    if ( key[VK_UP] )
	{
        if ( key['Z'] )
			cam.dist -= 0.25f;
		else
			cam.pitch += 1.0f;
	}

    if ( key[VK_DOWN] )
	{
		if ( key['Z'] )
			cam.dist += 0.25f;
		else
			cam.pitch -= 1.0f;
	}

    cam.x = cam.xt+lengthdir_x(-cam.dist, cam.yaw, cam.pitch);
    cam.y = cam.yt+lengthdir_y(-cam.dist, cam.yaw, cam.pitch);
    cam.z = cam.zt+lengthdir_z(-cam.dist, cam.pitch);
}

int LastMidMouseX = 0;
int LastMidMouseY = 0;

void mouseE()
{
    if ( mouse[0] )
    {
        POINT cms = { CurX, CurY };
        ScreenToClient(g_hMain,&cms);

        RECT rc;
        GetWindowRect(g_DrawArea,&rc);

        if ( cms.x > rc.left && cms.x < rc.right &&
             cms.y > rc.top && cms.y < rc.bottom )
        {
            SetFocus( g_hMain );
        }

        GetClientRect(g_hMain,&rc);
        int CH = rc.bottom;
    }
    if ( mouse[1] )     // Middle mouse
    {
		if ( LastMidMouseX == 0 && LastMidMouseY == 0 )
		{
			POINT pn;
			GetCursorPos(&pn);
			LastMidMouseX = pn.x;
			LastMidMouseY = pn.y;
		}
		else
        {
			POINT pn;
			GetCursorPos(&pn);

			if ( CameraView != VIEW_PERSPECTIVE )
			{
				float distx = (float)(pn.x - LastMidMouseX);
				float disty = (float)(pn.y - LastMidMouseY);
				ortho.xt += distx;
				ortho.yt += disty;
			}
			else
			{
				float disth = (float)(pn.x - LastMidMouseX) / 100.0f;
				float distz = (float)(pn.y - LastMidMouseY) / 100.0f;
				cam.xt += lengthdir_x( disth, -cam.yaw-90, 0 );
				cam.yt += lengthdir_y( disth, -cam.yaw+90, 0 );
				cam.zt += lengthdir_z( distz, -cam.pitch-90 );
			}

			LastMidMouseX = pn.x;
			LastMidMouseY = pn.y;
		}
    }
	else
	{
		LastMidMouseX = 0;
		LastMidMouseY = 0;
	}
    if ( mouse[2] )     // Right mouse
    {
        if ( mx!=CurX || my!=CurY )
        {
            cam.yaw += (CurX-mx);
            cam.pitch += (CurY-my);

            if (cam.pitch>=89) cam.pitch=89;
            if (cam.pitch<=-89) cam.pitch=-89;

            mx = CurX;
            my = CurY;
        }
    }
}


/** Math functions **/
float lengthdir_x(float len,float yaw,float pitch)
{
    return(sin(degtorad(yaw-90))*cos(degtorad(pitch))*len);
}


float lengthdir_y(float len,float yaw,float pitch)
{
    return(cos(degtorad(yaw-90))*cos(degtorad(pitch))*len);
}


float lengthdir_z(float len, float pitch)
{
    return(sin(degtorad(pitch))*len);
}

float degtorad(float deg)
{
    return deg*pi/180;
}

NORM ComputeNormals(VERTEX vt1, VERTEX vt2, VERTEX vt3)
{
    float nx,ny,nz;
    float m;
    NORM norm;

    // Dot Product
    nx = (vt2.mY-vt1.mY)*(vt3.mZ-vt1.mZ)-(vt3.mY-vt1.mY)*(vt2.mZ-vt1.mZ);
    ny = (vt2.mZ-vt1.mZ)*(vt3.mX-vt1.mX)-(vt3.mZ-vt1.mZ)*(vt2.mX-vt1.mX);
    nz = (vt2.mX-vt1.mX)*(vt3.mY-vt1.mY)-(vt3.mX-vt1.mX)*(vt2.mY-vt1.mY);

    // Magnitude
    m = sqrtf(nx*nx + ny*ny + nz*nz);

    // Normalize
    nx /= m;
    ny /= m;
    nz /= m;

    // Store
    norm.n[0] = nx;
    norm.n[1] = nz;
    norm.n[2] = -ny;

    return norm;
}

/*bool rayIntersectsPoint(EZvector &Origin, EZvector &Direction, const EZvector &Point)
{
    EZvector P(Origin),D(Direction),V(Point);

    EZvector vss = V - P;
    vss.Normalize();

    float TestDist = sqrtf( powf(D.x-vss.x,2) + powf(D.z-vss.z,2) + powf(D.z-vss.z,2) );
    if ( TestDist < MinDistance )
    {
        //MinDistance = TestDist;
        return true;
    }
    return false;
}*/

/*bool rayIntersectsTriangle( EZvector &Origin, EZvector &Direction,
                            EZvector &v1, EZvector &v2, EZvector &v3)
{
    EZvector p(Origin),d(Direction);
    EZvector e1,e2,h,s,q;
    EZvector V0(v1),V1(v2),V2(v3);
    float a,f,u,v,t;

    e1 = V1 - V0;
    e2 = V2 - V0;

    d.CrossProduct(&h,e2);

    a = e1.DotProduct(h);

    //if (a > -0.00001 && a < 0.00001) return(false);
    if (a == 0.0f) return false;

    f = 1.0f / a;

    s = p - V0;

    u = f * (s.DotProduct(h));

    if (u < 0.0 || u > 1.0) return(false);

    s.CrossProduct(&q,e1);

    v = f * d.DotProduct(q);

    if (v < 0.0 || u + v > 1.0) return(false);

    // at this stage we can compute t to find out where
    // the intersection point is on the line

    t = f * e2.DotProduct(q);

    if (t > 0.00001f) return(true);
    else return (false);
}*/

HTREEITEM TreeView_AddNode(HWND tree, HTREEITEM parent, const char *name)
{
    TV_INSERTSTRUCT tvis;
    TVITEM tvi;

    tvis.hParent = ((parent)?parent:TVI_ROOT);
    tvis.hInsertAfter = TVI_LAST;
    tvi.mask = TVIF_TEXT;
    tvi.pszText = (char*)name;
    tvi.cchTextMax = strlen(name);
    tvis.item = tvi;
    return TreeView_InsertItem(tree,&tvis);
}

HTREEITEM TreeView_AddResource(HWND tree, HTREEITEM parent, const char *name, LPARAM lParam )
{
    TV_INSERTSTRUCT tvis;
    TVITEM tvi;

    //printf("lParam: %d\n", lParam);

    tvis.hParent = parent;
    tvis.hInsertAfter = TVI_LAST;
    tvi.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.pszText = (char*)name;
    tvi.cchTextMax = strlen(name);
    tvi.lParam = lParam;
    tvis.item = tvi;

    return TreeView_InsertItem(tree,&tvis);
}

BOOL OpenFileDlg(const char *filter, const char *ext)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = g_hMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = ext;

    if (GetOpenFileName(&ofn))
    {
        for (int i=0; i<260; i++)
            fileName[i] = szFileName[i];

        return TRUE;
    }
    else
        return FALSE;
}

BOOL SaveFileDlg(const char *filter, const char *ext)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = g_hMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = ext;

    if (GetSaveFileName(&ofn))
    {
        for (int i=0; i<260; i++)
            fileName[i] = szFileName[i];

        return TRUE;
    }
    return FALSE;
}

BOOL SaveFileDlg(char *filter, char *ext, char *filename)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = g_hMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFileTitle = filename;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = ext;

    if (GetSaveFileName(&ofn))
    {
        for (int i=0; i<260; i++)
            fileName[i] = szFileName[i];

        return TRUE;
    }
    return FALSE;
}

void importSound()
{
    if (!OpenFileDlg("WAVE sound file (.wav)\0*.wav\0", "wav"))
        return;

    FILE *sfp;
    sfp = fopen(fileName,"rb");
    if (sfp==NULL) return;
    long freq,bpas,length;
    short pcm,channels,blocks,bp;

    //--Header
    fseek(sfp,20,SEEK_CUR);
    fread(&pcm,1,2,sfp);        //Type of Compression
    fread(&channels,1,2,sfp);   //Number of channels
    fread(&freq,1,4,sfp);       //Sound Frequency
    fread(&bpas,1,4,sfp);       //Bytes per Average Second
    fread(&blocks,1,2,sfp);     //Block Alignment
    fread(&bp,1,2,sfp);         //Bit Precision (Bits per Block)
    fseek(sfp,4,SEEK_CUR);
    fread(&length,1,4,sfp);     //Length of data

    //--Header check
    if (pcm!=1)
    {
        MessageBox( g_hMain,
                    "The sound file is not an uncompressed PCM waveform sound!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (channels!=1)
    {
        MessageBox( g_hMain,
                    "The sound file is not a Mono sound!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (freq!=22050)
    {
        MessageBox( g_hMain,
                    "The sound file does not have a frequency of 22050 kHz!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (blocks!=2)
    {
        MessageBox( g_hMain,
                    "The sound file does not have a block size of 2 bytes!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (bp!=16)
    {
        MessageBox( g_hMain,
                    "The sound file is not a 16-bit sound!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }

    //--Read waveform data
    g_Sounds[CUR_SOUND].len = length;
    delete [] g_Sounds[CUR_SOUND].data;
    g_Sounds[CUR_SOUND].data = new BYTE[g_Sounds[CUR_SOUND].len];
    fread(g_Sounds[CUR_SOUND].data,g_Sounds[CUR_SOUND].len,1,sfp);

    //--Close the file
    fclose(sfp);

    //--Update the window information
    sprintf(str,"%d Kbs", (int)g_Sounds[CUR_SOUND].len/1000);
    SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[CUR_SOUND].Name);
    SetDlgItemText(g_hSndDlg, ID_SIZE, str);
}

void importTexture(char *file)
{
	FILE *ifp;
    WORD w=0,h=0;
    BYTE bpp=0;
	char* lwrFileName = strlwr(file);

    ifp = fopen(file,"rb");
    if (ifp==NULL) return;

    if (strstr(lwrFileName,".tga"))
    {
        fseek(ifp, 12, SEEK_SET);
        fread(&w,1,2,ifp);
        fread(&h,1,2,ifp);
        bpp = fgetc(ifp);
        fseek(ifp, 18, SEEK_SET);
    }
    if (strstr(lwrFileName,".bmp") || strstr(lwrFileName,".dib"))
    {
        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bih;

        fread(&bfh, sizeof(BITMAPFILEHEADER), 1, ifp);
        fread(&bih, sizeof(BITMAPINFOHEADER), 1, ifp);

        w = bih.biWidth;
        h = bih.biHeight;
        bpp = bih.biBitCount;
    }

    printf("W: %u, H: %u, BPP: %u\n", w, h, bpp);

    //--Error catch
    if (w!=256)
    {
        MessageBox(g_hMain, "The image file does not have a width of 256!", "Error", MB_OK | MB_ICONERROR);
        fclose(ifp);
        return;
    }
    if ( bpp!=16 && bpp!=24 && bpp!=32 )
    {
        MessageBox(g_hMain, "The image file is not a 16-bit, 24-bit or 32-bit image!", "Error", MB_OK | MB_ICONERROR);
        fclose(ifp);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

    if (bpp==16)
    {
        fread(&g_Texture,w*h,2,ifp);
        for (int i=0; i<w*h; i++)
        {
            WORD w = g_Texture[i];
            int B = ((w>> 0) & 31);
            int G = ((w>> 5) & 31);
            int R = ((w>>10) & 31);
            int A = 1;
            if (!B && !G && !R)
                A = 0;
            g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 256,256,0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g_Texture);
    }
    if (bpp==24)
    {

        for (int i=0; i<w*h; i++)
        {
            int B = ((BYTE)fgetc(ifp)/8) & 31;
            int G = ((BYTE)fgetc(ifp)/8) & 31;
            int R = ((BYTE)fgetc(ifp)/8) & 31;
            int A = 1;
            if (!B && !G && !R)
                A = 0;
            g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
        }
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB5_A1,256,256,0,GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV,g_Texture);
    }
    if (bpp==32)
    {
        for (int i=0; i<w*h*2; i++)
        {
            int B = ((BYTE)fgetc(ifp)/8) & 31;
            int G = ((BYTE)fgetc(ifp)/8) & 31;
            int R = ((BYTE)fgetc(ifp)/8) & 31;
            int A = ((BYTE)fgetc(ifp)/255) & 1;
            if (!B && !G && !R)
                A = 0;
            else
                A = 1;
            g_Texture[i] = (B) + (G<<5) + (R<<10) + (A<<15);
        }
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB5_A1,256,256,0,GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV,g_Texture);
    }

    Model.bytes_tex = w * (h*2);

    fclose(ifp);
}

void importTexture()
{
    //if (!OpenFileDlg("TGA image (.tga)\0*.tga\0Windows Bitmap (.bmp)\0*.bmp\0", "tga"))
    if (!OpenFileDlg("All Supported Files\0*.bmp;*.tga;*.dib\0TGA image (.tga)\0*.tga\0Windows Bitmap (.bmp)\0*.bmp;*.dib\0", "tga"))
        return;

	importTexture( fileName );
}

void exportSound()
{
    if (!SaveFileDlg("WAVE sound file (.wav)\0*.wav\0", "wav"))
        return;

    FILE *sfp;
    sfp = fopen(fileName,"wb");
    if (sfp==NULL)
        return;
    long lval;
    short sval;

    /*fputc('R',sfp);
    fputc('I',sfp);
    fputc('F',sfp);
    fputc('F',sfp);*/
	fwrite("RIFF", 4, 1, sfp );
    lval = g_Sounds[CUR_SOUND].len+(44-8);
    fwrite(&lval,1,4,sfp);
    fputc('W',sfp);
    fputc('A',sfp);
    fputc('V',sfp);
    fputc('E',sfp);
    fputc('f',sfp);
    fputc('m',sfp);
    fputc('t',sfp);
    fputc(' ',sfp);
    lval = 16;
    fwrite(&lval,1,4,sfp);
    sval = 1;
    fwrite(&sval,1,2,sfp);
    sval = 1;
    fwrite(&sval,1,2,sfp);
    lval = 22050;
    fwrite(&lval,1,4,sfp);
    lval = 22050 * (1 * 16 / 8);
    fwrite(&lval,1,4,sfp);
    sval = (1 * 16 / 8);
    fwrite(&sval,1,2,sfp);
    sval = 16;
    fwrite(&sval,1,2,sfp);
    fputc('d',sfp);
    fputc('a',sfp);
    fputc('t',sfp);
    fputc('a',sfp);
    lval = g_Sounds[CUR_SOUND].len;
    fwrite(&lval,1,4,sfp);
    fwrite( g_Sounds[CUR_SOUND].data,g_Sounds[CUR_SOUND].len,1,sfp);
    fclose(sfp);
}

void exportTexture()
{
    if (!SaveFileDlg("TGA image (.tga)\0*.tga\0Windows Bitmap Image (.bmp)\0*.bmp\0", "tga"))
        return;

    FILE *ifp;
    int w = 256;
    int h = (Model.bytes_tex/2) / w;

    ifp = fopen(fileName,"wb");
    if (ifp==NULL) return;

    if (strstr(_strupr(fileName),".TGA")!=NULL)
    {
        TARGAINFOHEADER tih; /* targa info-header */
        // = {0,0,2, 0,0,0, 0,0,w,h,16,0};

        ZeroMemory(&tih, sizeof(TARGAINFOHEADER));
        tih.tgaIdentsize = 0;
        tih.tgaColourmaptype = 0;
        tih.tgaImagetype = 2; //TGA_UNCOMPRESSED
        tih.tgaWidth = w;
        tih.tgaHeight = h;
        tih.tgaBits = 16;

        fwrite(&tih, 18, 1, ifp);
    }
    if (strstr(_strupr(fileName),".BMP")!=NULL)
    {
        BITMAPFILEHEADER hdr;       /* bitmap file-header */
        BITMAPINFOHEADER bmi;       /* bitmap info-header */

        bmi.biSize = sizeof(BITMAPINFOHEADER);
        bmi.biWidth = w;
        bmi.biHeight = h;
        bmi.biPlanes = 1;
        bmi.biBitCount = 16;
        bmi.biCompression = BI_RGB;
        bmi.biSizeImage = 256*h*2;
        bmi.biClrImportant = 0;
        bmi.biClrUsed = 0;

        hdr.bfType = 0x4d42;
        hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + bmi.biSize + bmi.biSizeImage);
        hdr.bfReserved1 = 0;
        hdr.bfReserved2 = 0;
        hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + bmi.biSize;

        fwrite(&hdr,sizeof(BITMAPFILEHEADER),1,ifp);
        fwrite(&bmi,sizeof(BITMAPINFOHEADER),1,ifp);
    }

    fwrite(&g_Texture, Model.bytes_tex, 1, ifp);
    fclose(ifp);
}

void exportModel()
{
    if (!SaveFileDlg("3D Model File (.3DF)\0*.3df\0Wavefront .OBJ (.obj)\0*.obj\0", "3df"))
        return;

    if (strstr(strupr(fileName),".3DF")!=NULL)
    {
        Save3DFData(fileName);
        return;
    }

    return;
}

void importModel()
{
    if (!OpenFileDlg("3D File (*.3DF)\0*.3DF\0Wavefront OBJ File (.obj)\0*.obj\0", "3DF"))
        return;

    if (strstr(strlwr(fileName),".3df")!=NULL)
    {
        Load3DFData(fileName);
    }

    if (strstr(strlwr(fileName),".obj")!=NULL)
    {
        LoadOBJData(fileName);
    }

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %ld",Model.num_tris);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
    MessageBox(g_hMain,sstr,"triangles",MB_OK);
}

void PlayWave(SOUND *sptr)
{
    /* We have to save the .WAV file temporarily >.> */
    FILE *wfp = fopen("C:\\c3dit_temp34567.wav","wb");
    long lval = 0;
    short sval = 0;

    fputc('R',wfp);
    fputc('I',wfp);
    fputc('F',wfp);
    fputc('F',wfp);
    lval = sptr->len+(44-8);
    fwrite(&lval,1,4,wfp);
    fputc('W',wfp);
    fputc('A',wfp);
    fputc('V',wfp);
    fputc('E',wfp);
    fputc('f',wfp);
    fputc('m',wfp);
    fputc('t',wfp);
    fputc(' ',wfp);
    lval = 16;
    fwrite(&lval,1,4,wfp);
    sval = 1;
    fwrite(&sval,1,2,wfp);
    sval = 1;
    fwrite(&sval,1,2,wfp);
    lval = 22050;
    fwrite(&lval,1,4,wfp);
    lval = 22050 * (1 * 16 / 8);
    fwrite(&lval,1,4,wfp);
    sval = (1 * 16 / 8);
    fwrite(&sval,1,2,wfp);
    sval = 16;
    fwrite(&sval,1,2,wfp);
    fputc('d',wfp);
    fputc('a',wfp);
    fputc('t',wfp);
    fputc('a',wfp);
    lval = sptr->len;
    fwrite(&lval,1,4,wfp);

    fwrite(sptr->data,sptr->len,1,wfp);
    fclose(wfp);

    PlaySound("C:\\c3dit_temp34567.wav",NULL,SND_ASYNC);
}
