
#include "stdafx.h"

/*
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE	0x0700
#ifdef WINVER
#undef WINVER
#endif
#define WINVER 0x0501
*/

// Temporary
#define IDR_STATUSBAR 60000

#define GLOBALVAR_DEFINE
#include "EditorMain.h"
#undef GLOBALVAR_DEFINE

#include "IniFile.h"
#include "version.h"
#include "Global.h"
#include "Targa.h"
#include "Timer.h"
#include "Renderer.h"

#include <ShObjIdl_core.h>
//#include <ShObjIdl.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <string>
#include <thread>
#include <chrono>


std::vector<std::string> g_HintStrings = {
	"Holding the right mouse button lets you orbit the object!",
	"Scrolling the mouse wheel will zoom in and out of the scene!",
	"Holding the middle mouse button will let you pan the camera!",
	"Left clicking the scene will refocus the keyboard input!",
	"Make sure to save your files!  Backup frequently!",
	"There are keyboard shortcuts for most actions!",
	"Hold Z and press Up/Down to zoom in and out alternatively.",
	"Holding CTRL and clicking a triangle will add it to the selection!",
	"+/- on the NumPad will zoom in and out as well!",
	"Report bugs to the GitHub!",
	"The cake is a lie..."
};

char g_ProjectFileFormats[] = {
	"All supported projects (*.*)\0*.project;*.car\0"
	"C3Dit Project (*.project)\0*.project\0" // Preferred format as it stores the data in uncompressed full precision or source
	"Character File (*.car)\0*.car\0"
	//"Character Text File (*.cat)\0*.cat\0" // Tatem Games version of .CAR as a script
	"Carnivores 1 Object (*.c1o)\0*.c1o\0" // Only supported in case machf's editors are still used
	"Carnivores 2 Object (*.c2o)\0*.c2o\0" // Only supported in case machf's editors are still used
	"\0"
};

char g_ModelFileFormats[] = {
	"All supported models (*.*)\0*.3df;*.3dn;*.obj\0"
	"3D Model File (*.3df)\0*.3df\0"
	"3D Model File \'New\' (*.3dn)\0*.3dn\0"
	"Wavefront Object (*.obj)\0*.obj\0"
	"\0"
};

char g_AnimationFileFormats[] = {
	"All supported animations (*.*)\0*.vtl;*.fvtl\0"
	"Vertex Transformed List (*.vtl)\0*.vtl\0" // Stored positions are compressed to INT16
	"Flexible Vertex Transformed List (*.fvtl)\0*.fvtl\0" // Stored positions can be stored in different types
//	"Skeletal Animation Track (*.trk)\0*.trk\0" // TODO: Implement at a later date
	"\0"
};

char g_ImageFileFormats[] = {
	"All supported images (*.*)\0*.tga;*.bmp\0"
		"Portable Network Graphic PNG (*.png)\0*.png\0"
		"TrueVision Targa Image\0*.tga\0"
		"Windows Bitmap Image\0*.bmp\0"
		"CRT (*.crt)\0*.crt;*.crtlm\0"
		"\0"
};

char g_SoundFileFormats[] = {
	"Microsoft WAVE audio file (*.wav)\0*.wav\0"
	//	"Vorbis OGG (*.ogg)\0*.ogg\0" // TODO: Implement with STB in "Depends/stb"
		"\0"
};


// Declare the Dialog Procedures
INT_PTR CALLBACK DlgAboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgAnimationProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgErrorProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgSoundProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgObjectProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgSettingsProc(HWND, UINT, WPARAM, LPARAM);


Settings* g = nullptr;
std::unique_ptr<Timer> g_Timer;
HINSTANCE hInst = nullptr;
std::string g_Title = "Character 3D Editor";
POINT g_CursorPos = { 0, 0 };
std::string g_ExePath = "";
std::string g_WorkingPath = "";
std::string g_IniPath = "";
std::string g_ProjectPath = "";


/////////////////////////////////////////////////////////////////////////////////////
// Functions
bool SaveProject(const std::string&);
bool LoadProject(const std::string&);


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


/* Deprecated: Will be moved to a permanently docked window */
INT_PTR CALLBACK DlgFaceProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		for (int i = 0; i < 16; i++)
		{
			// TODO: get the flags from the selected faces
			//CheckDlgButton(hwnd, IDC_FACE_FLAG1 + i, (false) ? BST_CHECKED : BST_UNCHECKED);
		}
	}
	break;

	case WM_CLOSE:
	{
		//if (g_TriSelection.size() >= 1)
		//	SendMessage(hwnd, WM_COMMAND, (IDB_APPLY << 16) | (IDB_APPLY), (LPARAM)GetDlgItem(hwnd, IDB_APPLY));

		EndDialog(hwnd, IDCANCEL);
	}
	return true;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			// Apply the new settings
			uint16_t flags = 0u;

			for (int i = 0; i < 16; i++)
			{
				//if (IsDlgButtonChecked(hwnd, IDC_FACE_FLAG1 + i) == BST_CHECKED)
				//	flags |= 1 << i;
			}

			/*for (auto i = 0u; i < g_TriSelection.size(); i++)
			{
				unsigned f = g_TriSelection[i];
				g_Triangles[f].flags = flags;
			}*/
			EndDialog(hwnd, IDOK);
		}
		return true;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			return true;
		}
	}
	break;

	}

	return false;
}


bool LoadProject(const std::string& filepath)
{
	std::stringstream ss;
	ss << std::uppercase << filepath;
	std::string fileext = ss.str().substr(ss.str().find_last_of('.'), std::string::npos);

	g_Project.Clear();

	bool b = true;

	if (!fileext.compare(".PROJECT")) {
		b = LoadProjectFile(filepath);
	}
	else if (!fileext.compare(".CAR")) {
		b = LoadCARFile(filepath);
	}
	else if (!fileext.compare(".3DF")) {
		b = Load3DFFile(filepath);
	}
	else if (!fileext.compare(".C2O")) {
		b = LoadC2OFile(filepath);
	}
	else if (!fileext.compare(".OBJ")) {
		b = LoadOBJFile(filepath);
	}
	else { /* TODO: MessageBox() here instead of each file */ return false; }

	if (b) {
		g->sProjectFilePath = filepath;
		g_ProjectPath = filepath;
	}

	return b;
}


bool SaveProject(const std::string& filepath)
{
	if (filepath.empty())
		return false;

	std::stringstream ss;
	ss << std::uppercase << filepath;
	std::string fileext = ss.str().substr(ss.str().find_last_of('.'), std::string::npos);

	bool b = true;

	if (!fileext.compare(".PROJECT")) {
		b = SaveProjectFile(filepath);
	}
	else if (!fileext.compare(".CAR")) {
		b = SaveCARFile(filepath);
	}
	else if (!fileext.compare(".3DF")) {
		b = Save3DFFile(filepath);
	}
	/*else if (!fileext.compare(".C1O")) {
		b = SaveC1OFile(filepath);
	}*/
	else if (!fileext.compare(".C2O")) {
		b = SaveC2OFile(filepath);
	}
	else if (!fileext.compare(".OBJ")) {
		b = SaveOBJFile(filepath);
	}
	else { return false; }

	if (b) {
		g->sProjectFilePath = filepath;
		g_ProjectPath = filepath;
	}

	return true;
}


void keyboardE()
{
	/*uint8_t KeyState[256];
	std::memset(KeyState, 0, 256);

	if (!GetKeyboardState(KeyState))
	{
		return;
	}

	if (KeyState[VK_SUBTRACT])
		g_Camera.Dist += 0.5f;

	if (KeyState[VK_ADD])
		g_Camera.Dist -= 0.5f;

	if (KeyState[VK_LEFT])
		g_Camera.Yaw += 1.0f;

	if (KeyState[VK_RIGHT])
		g_Camera.Yaw -= 1.0f;

	if (KeyState[VK_UP])
	{
		if (KeyState['Z'])
			g_Camera.Dist -= 0.25f;
		else
			g_Camera.Pitch += 1.0f;
	}

	if (KeyState[VK_DOWN])
	{
		if (KeyState['Z'])
			g_Camera.Dist += 0.25f;
		else
			g_Camera.Pitch -= 1.0f;
	}

	g_Camera.x = g_Camera.TargetX + lengthdir_x(-g_Camera.Dist, g_Camera.Yaw, g_Camera.Pitch);
	g_Camera.y = g_Camera.TargetY + lengthdir_y(-g_Camera.Dist, g_Camera.Yaw, g_Camera.Pitch);
	g_Camera.z = g_Camera.TargetZ + lengthdir_z(-g_Camera.Dist, g_Camera.Pitch);
	*/
}

int LastMidMouseX = 0;
int LastMidMouseY = 0;

void mouseE()
{
	/*if (mouse[0])
	{
		POINT cms = { CurX, CurY };
		ScreenToClient(g_hMain, &cms);

		RECT rc;
		GetWindowRect(g_DrawArea, &rc);

		if (cms.x > rc.left && cms.x < rc.right &&
			cms.y > rc.top && cms.y < rc.bottom)
		{
			SetFocus(g_hMain);
		}

		GetClientRect(g_hMain, &rc);
		int CH = rc.bottom;
	}
	if (mouse[1])     // Middle mouse
	{
		if (LastMidMouseX == 0 && LastMidMouseY == 0)
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

			if (g_Camera.Type == VIEW_ORTHOGRAPHIC)
			{
				float distx = (float)(pn.x - LastMidMouseX);
				float disty = (float)(pn.y - LastMidMouseY);
				g_Camera.TargetX += distx;
				g_Camera.TargetY += disty;
			}
			else if (g_Camera.Type == VIEW_PERSPECTIVE)
			{
				float disth = (float)(pn.x - LastMidMouseX) / 100.0f;
				float distz = (float)(pn.y - LastMidMouseY) / 100.0f;
				g_Camera.TargetX += lengthdir_x(disth, -g_Camera.Yaw - 90, 0);
				g_Camera.TargetY += lengthdir_y(disth, -g_Camera.Yaw + 90, 0);
				g_Camera.TargetZ += lengthdir_z(distz, -g_Camera.Pitch - 90);
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
	if (mouse[2])     // Right mouse
	{
		if (mx != CurX || my != CurY)
		{
			g_Camera.Yaw += (CurX - mx);
			g_Camera.Pitch += (CurY - my);

			if (g_Camera.Pitch >= 89) g_Camera.Pitch = 89;
			if (g_Camera.Pitch <= -89) g_Camera.Pitch = -89;

			mx = CurX;
			my = CurY;
		}
	}*/
}

/*NORM ComputeNormals(VERTEX vt1, VERTEX vt2, VERTEX vt3)
{
	float nx, ny, nz;
	float m;
	NORM norm, a, b;

	a.x = vt1.x - vt2.x;
	a.y = vt1.y - vt2.y;
	a.z = vt1.z - vt2.z;

	b.x = vt3.x - vt2.x;
	b.y = vt3.y - vt2.y;
	b.z = vt3.z - vt2.z;

	// Cross Product
	nx = (a.y) * (b.z) - (b.y) * (a.z);
	ny = (a.z) * (b.x) - (b.z) * (a.x);
	nz = (a.x) * (b.y) - (b.x) * (a.y);

	// Magnitude
	m = sqrtf(nx * nx + ny * ny + nz * nz);

	// Normalize
	nx /= m;
	ny /= m;
	nz /= m;

	// Store
	norm.n[0] = nx;
	norm.n[1] = nz;
	norm.n[2] = -ny;

	return norm;
}*/


std::string OpenFileDlg(HWND parent, const char* filter, const std::string& ext, const std::string& title)
{
	// TODO: use an std::vector of std::pairs of std::strings for `filter`, construct the c-string in here
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	std::memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = parent;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext.c_str();

	if (GetOpenFileName(&ofn))
	{
		return std::string(szFileName);
	}
	else
		return "";
}


std::string SaveFileDlg(HWND parent, const char* filter, const std::string& ext, const std::string& title)
{
	// TODO: use an std::vector of std::pairs of std::strings for `filter`, construct the c-string in here
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	std::memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME); // SEE NOTE BELOW
	ofn.hwndOwner = parent;
	ofn.lpstrFilter = filter;
	//if (!filename.empty())
	//	ofn.lpstrFileTitle = title.c_str();
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext.c_str();

	if (GetSaveFileName(&ofn))
	{
		return std::string(szFileName);
	}

	return "";


	/*
	MSDN lists GetSaveFilename and co to be superseded by IFileDialog, in the future we should use this
	*/
	//IFileDialog* pfd;
}


void PlayWave(CSound& snd)
{
	// We have to save the .WAV file temporarily

	if (snd.Length == 0u || snd.Data == nullptr)
		return;

	if (snd.Save("temp_sample.wav"))
		PlaySound("temp_sample.wav", NULL, SND_ASYNC);
}


void UpdateMenuChecks(int menu_item = 0, bool checked = false)
{
	static std::map<int, bool*> menuchecks;

	if (menuchecks.empty()) {
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_LIGHTING, &g->bUseLighting));
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_WIREFRAME, &g->bShowWireframe));
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_SOLID, &g->bShowSolid));
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_GRID, &g->bShowGrid));
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_AXIS, &g->bShowAxis));
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_BONES, &g->bShowBones));
		menuchecks.insert(std::make_pair<int, bool*>(ID_VIEW_JOINTS, &g->bShowJoints));
		std::cout << "menuchecks.size() = " << menuchecks.size() << std::endl;
	}

	unsigned int f = 0;

	if (menu_item != 0) {
		if (checked) f = MF_CHECKED;
		else         f = MF_UNCHECKED;
		CheckMenuItem(GetSubMenu(GetMenu(g_hMain), 1), menu_item, f);
	}
	else for (auto const& item : menuchecks) {
		if (*item.second) f = MF_CHECKED;
		else              f = MF_UNCHECKED;
		if (CheckMenuItem(GetSubMenu(GetMenu(g_hMain), 1), item.first, f) == -1)
		{
			std::cerr << "UpdateMenuChecks(" << item.first << ", " << (*item.second) << ") : CheckMenuItem failed!" << std::endl;
		}
	}
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	hMenu = GetMenu(g_hMain);

	switch (uMsg)
	{
	case WM_CREATE: {
		//UpdateMenuChecks();
	} return false;

	case WM_CLOSE: {
		// TODO: check if the scene is saved, if not, ask if we want to quit without saving
		//if ( MessageBox(hwnd, "The project has unsaved changes, are you sure you want to exit?", "Unsaved changes", MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		PostQuitMessage(0);
	} break;

	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		RECT rcTool, rcStatus, rcClient, rc;
		int iToolHeight, iStatusHeight;

		// Get Rects
		GetClientRect(g_hMain, &rcClient);

		// Size toolbar and get height
		gHTool = GetDlgItem(hwnd, IDR_TOOLBAR1);
		SendMessage(gHTool, TB_AUTOSIZE, 0, 0);
		GetWindowRect(gHTool, &rcTool);
		iToolHeight = rcTool.bottom - rcTool.top;

		// Size status bar and get height
		gHStatus = GetDlgItem(hwnd, IDR_STATUSBAR);
		SendMessage(gHStatus, WM_SIZE, 0, 0);
		GetWindowRect(gHStatus, &rcStatus);
		iStatusHeight = rcStatus.bottom - rcStatus.top;

		// Calculate remaining height and size draw area
		SetRect(&rc, rcClient.left, rcClient.top + iToolHeight, rcClient.right - 200, rcClient.bottom - (iToolHeight + iStatusHeight + 32));
		MoveWindow(g_DrawArea, rc.left, rc.top, rc.right, rc.bottom, true);

		MoveWindow(g_AniTrack, rc.left, rc.top + rc.bottom, rc.right, 32, true);

		// File Components View
		//SetRect(&rc, rc.left + rc.right, rc.top, 200, rc.bottom);
		//MoveWindow(g_FileView, rc.left, rc.top, rc.right, rc.bottom, true);

		// Resize OpenGL
		RECT rcGL;
		GetClientRect(g_DrawArea, &rcGL);

	} break;

	case WM_NOTIFY: {
		LPNMHDR nmh = reinterpret_cast<LPNMHDR>(lParam);

		switch (nmh->code)
		{
		case NM_CLICK: {
			DEBUGONLYPRINT("WinProc() : WM_NOTIFY.NM_CLICK(id:" << nmh->idFrom << ")");
		} break;

		case NM_RCLICK: {
			DEBUGONLYPRINT("WinProc() : WM_NOTIFY.NM_RCLICK(id:" << nmh->idFrom << ")");
		} break;
		}
	} break; //case WM_NOTIFY

	//case WM_MOVE:
	//	break;

	case WM_DESTROY: {
		DestroyWindow(embedTest);
	} return false;

	case WM_INITMENUPOPUP: {
		//std::cout << "WM_INITMENUPOPUP : lParam(" << (LOWORD(lParam)) << ", " << (HIWORD(lParam)) << ")" << std::endl;
		if ((HMENU)wParam == GetSubMenu(hMenu, 1) && LOWORD(lParam) == 1)
		{
			UpdateMenuChecks();
		}
	} return false;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEW: g_Project.Clear(); break;
		case ID_FILE_OPEN: LoadProject(OpenFileDlg(g_hMain, g_ProjectFileFormats, "project")); break;
		case ID_FILE_SAVE: SaveProject((g_ProjectPath.empty()) ? SaveFileDlg(g_hMain, g_ProjectFileFormats, "project") : g_ProjectPath); break;
		case ID_FILE_SAVEAS: SaveProject(SaveFileDlg(g_hMain, g_ProjectFileFormats, "project")); break;

		case ID_FILE_RECENTFILES:
			break;

		case ID_FILE_EXIT:
			PostQuitMessage(0);
			SendMessage(g_hMain, WM_CLOSE, 0, 0);
			break;

		case ID_VIEW_SOLID:
		{
			// Enable Menu items
			HMENU hResMenu;
			hResMenu = GetSubMenu(hMenu, 1);

			g->bShowFaces = !g->bShowFaces;
			if (g->bShowFaces)
				CheckMenuItem(hResMenu, wParam, MF_CHECKED | MF_BYCOMMAND);
			else
				CheckMenuItem(hResMenu, wParam, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_VIEW_WIREFRAME:
		{
			// Enable Menu items
			HMENU hResMenu;
			hResMenu = GetSubMenu(hMenu, 1);

			g->bShowWireframe = !g->bShowWireframe;
			if (g->bShowWireframe)
				CheckMenuItem(hResMenu, ID_VIEW_WIREFRAME, MF_CHECKED | MF_BYCOMMAND);
			else
				CheckMenuItem(hResMenu, ID_VIEW_WIREFRAME, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_VIEW_LIGHTING:
		{
			// Enable Menu items
			HMENU hResMenu;
			hResMenu = GetSubMenu(hMenu, 1);

			g->bUseLighting = !g->bUseLighting;
			if (g->bUseLighting)
				CheckMenuItem(hResMenu, ID_VIEW_LIGHTING, MF_CHECKED | MF_BYCOMMAND);
			else
				CheckMenuItem(hResMenu, ID_VIEW_LIGHTING, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_VIEW_BONES:
		{
			// Enable Menu items
			HMENU hResMenu;
			hResMenu = GetSubMenu(hMenu, 1);

			g->bShowBones = !g->bShowBones;
			if (g->bShowBones)
				CheckMenuItem(hResMenu, ID_VIEW_BONES, MF_CHECKED | MF_BYCOMMAND);
			else
				CheckMenuItem(hResMenu, ID_VIEW_BONES, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_VIEW_JOINTS:
		{
			// Enable Menu items
			HMENU hResMenu;
			hResMenu = GetSubMenu(hMenu, 1);

			g->bShowJoints = !g->bShowJoints;
			if (g->bShowJoints)
				CheckMenuItem(hResMenu, ID_VIEW_JOINTS, MF_CHECKED | MF_BYCOMMAND);
			else
				CheckMenuItem(hResMenu, ID_VIEW_JOINTS, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_VIEW_GRID:
		{
			// Enable Menu items
			HMENU hResMenu;
			hResMenu = GetSubMenu(hMenu, 1);

			g->bShowGrid = !g->bShowGrid;
			if (g->bShowGrid) CheckMenuItem(hResMenu, ID_VIEW_GRID, MF_CHECKED | MF_BYCOMMAND);
			else              CheckMenuItem(hResMenu, ID_VIEW_GRID, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_VIEW_AXIS:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			g->bShowAxis = !g->bShowAxis;
			if (g->bShowAxis)
				CheckMenuItem(hResMenu, ID_VIEW_AXIS, MF_CHECKED | MF_BYCOMMAND);
			else
				CheckMenuItem(hResMenu, ID_VIEW_AXIS, MF_UNCHECKED | MF_BYCOMMAND);
		}
		break;

		case ID_TEXTURE_FLIPUV:
		{
			std::vector<CFace>& faces = g_Project.Model.Faces;
			for (auto t = 0u; t < g_Project.Model.Faces.size(); t++)
			{
				/*int v1 = faces[t].v1;
				int v2 = faces[t].v2;
				int v3 = faces[t].v3;
				faces[t].v1 = v3;
				faces[t].v2 = v2;
				faces[t].v3 = v1;*/

				CFace::_tcoord tcoord[3];

				for (int i = 0; i < 3; i++)
				{
					tcoord[i].x = faces[t].tcoord[i].x;
					tcoord[i].y = faces[t].tcoord[i].y;
				}
				for (int i = 0; i < 3; i++)
				{
					faces[t].tcoord[0].x = tcoord[3 - i - 1].x;
					faces[t].tcoord[0].y = tcoord[3 - i - 1].y;
				}
			}

			// Normals
			/*bool vlist[1024];
			std::memset(vlist, 0, sizeof(bool) * 1024);
			for (auto f = 0u; f < g_Project.Model.Faces.size(); f++)
			{
				int v1 = g_Project.Model.Faces[f].v1;
				int v2 = g_Project.Model.Faces[f].v2;
				int v3 = g_Project.Model.Faces[f].v3;
				g_Normals[f] = ComputeNormals(g_Project.Model.Vertices[v1], g_Project.Model.Vertices[v2], g_Project.Model.Vertices[v3]);
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
			}*/
		}
		break;

		case ID_TEXTURE_EXPORT: { g_Project.Texture.Save(SaveFileDlg(hwnd, g_ImageFileFormats, "tga")); } break;
		case ID_TEXTURE_IMPORT: { g_Project.Texture.Load(OpenFileDlg(hwnd, g_ImageFileFormats, "tga")); } break;

			/*case IDF_TOOLS:
			{
				ShowWindow(g_hTools, SW_SHOW);
			}
			break;*/

		case ID_TOOLS_ANIMATION:
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ANIMATION), hwnd, DlgAnimationProc);
			/*ShowWindow(g_hAniDlg, SW_SHOW);
			if (Model.mAnimCount > 0)
			{
				unsigned int KPS, AniTime, FrameCount;

				KPS = g_Animations[g_CurrentAnimation].KPS;
				FrameCount = g_Animations[g_CurrentAnimation].FrameCount;
				if (KPS > 0)
					AniTime = (FrameCount * 1000) / KPS;

				SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[g_CurrentAnimation].Name);
				SetDlgItemInt(g_hAniDlg, ID_AKPS, g_Animations[g_CurrentAnimation].KPS, FALSE);
				SetDlgItemInt(g_hAniDlg, ID_ATIME, AniTime, FALSE);
				SetDlgItemInt(g_hAniDlg, ID_AFRM, g_Animations[g_CurrentAnimation].FrameCount, FALSE);
			}*/
		}
		break;

		case ID_TOOLS_SOUND:
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SOUND), hwnd, DlgSoundProc);
			/*ShowWindow(g_hSndDlg, SW_SHOW);
			sprintf(str, "%d Kbs", (int)g_Sounds[Snd].len / 1000);

			SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
			SetDlgItemText(g_hSndDlg, ID_SIZE, str);*/
		}
		break;

		case ID_MODEL_OBJECTINFO:
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_OBJECT_PROPERTIES), hwnd, DlgObjectProc);
		}
		break;

		/*case IDB_SPLAY:
		{
			PlayWave(&g_Sounds[Snd]);
		}
		break;*/

		case ID_CAMERA_PERSPECTIVE: g_Camera.Type = VIEW_PERSPECTIVE; break;
		case ID_CAMERA_ORTHOGRAPHIC: g_Camera.Type = VIEW_ORTHOGRAPHIC; break;
		case ID_CAMERA_FIRSTPERSON: g_Camera.Type = VIEW_FIRSTPERSON; break;
			// TODO: Implement functions
		case ID_CAMERA_LEFT:
		case ID_CAMERA_RIGHT:
		case ID_CAMERA_TOP:
		case ID_CAMERA_BOTTOM:
		case ID_CAMERA_FRONT:
		case ID_CAMERA_BACK:
			break;

			//case ID_ANIMATION_PLAY: ANIMPLAY = TRUE; break;
			//case ID_ANIMATION_PAUSE: ANIMPLAY = FALSE; break;

		case ID_EDITOR_SETTINGS: {
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, DlgSettingsProc);
		} break;

		case ID_EDITOR_UPDATE:
		{
			// TODO: Run a check for updates then retrieve
		}
		break;

		case ID_HELP_YOUTUBE: {} break; // Rename this to 'Tutorial Videos'?
		case ID_HELP_DISCORD: {} break;
		case ID_HELP_GITHUB: {} break;
		case ID_HELP_ABOUT: { DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, DlgAboutProc); } break;

		////////////////////////////////////////////////////////////////////////////////////
		// END OF MENU IDENTIFIERS

		//case ID_TOOLS_VERTEXMODE: { HWND hItem = (HWND)lParam; } break;
		//case ID_TOOLS_FACEMODE: { HWND hItem = (HWND)lParam; } break;
		//case ID_TOOLS_ELEMENTSMODE: { HWND hItem = (HWND)lParam; } break;

		}
	}
	break;

	case WM_MOUSEWHEEL:
	{
		float zDelta = (float)(GET_WHEEL_DELTA_WPARAM(wParam)) / (float)(WHEEL_DELTA);
		g_Camera.Dist -= zDelta;
	}
	break;

	case WM_KEYDOWN:
	{
		bool ctrl = GetKeyState(VK_CONTROL) & 0x8000;
		//key[wParam] = TRUE;

		switch (wParam)
		{
		case VK_F9: // Emergency close
			if (ctrl) {
				std::cerr << "CTRL + F9 emergency program exit!" << std::endl;
				PostQuitMessage(0);
			}
			break;
		}
	}
	break;

	/*case WM_KEYUP:
		key[wParam] = FALSE;
		break;*/

	case WM_DROPFILES:
	{
		HDROP drop = (HDROP)wParam;
		std::vector<std::string> files;
		char filepath[256];

		for (UINT i = 0; i < DragQueryFile(drop, 0xFFFFFFFF, filepath, 256); i++)
		{
			if (!DragQueryFile(drop, i, filepath, 256)) continue;
			DragQueryFile(drop, i, filepath, 256);
			files.push_back(filepath);
		}

		DragFinish(drop);
		LoadProject(files.at(0));
	}
	break;

	/*case WM_LBUTTONDOWN:
		mouse[0] = TRUE;
		break;
	case WM_LBUTTONUP:
		mouse[0] = FALSE;
		break;
	case WM_MBUTTONDOWN:
		mouse[1] = TRUE;
		break;
	case WM_MBUTTONUP:
		mouse[1] = FALSE;
		break;
	case WM_RBUTTONDOWN:
		mouse[2] = TRUE;
		break;
	case WM_RBUTTONUP:
		mouse[2] = FALSE;
		break;*/

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return false;
}


#ifndef NDEBUG
int main(int argc, char* argv[])
{
	std::string sCommandLine = "";
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	int nCmdShow = SW_SHOW;

	sCommandLine.append(argv[0]);
	for (int i = 1; i < argc; i++)
	{
		sCommandLine += ' ';
		sCommandLine.append(argv[i]);
	}

	LPSTR lpCmdLine = const_cast<char*>(sCommandLine.c_str());
#else
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
#endif
	hInst = hInstance;
	WNDCLASSEX  wc;
	HWND		&hwnd = g_hMain;
	MSG         msg;
	bool        bQuit = false;
	RECT        rc = { 0,0,0,0 };
	std::unique_ptr<Renderer> video(nullptr);
	g_Timer.reset(new Timer());

	g = GlobalContainer::GlobalVar();

	try {

		// Get the executable path
		uint32_t module_string_size = 0u;
		g_ExePath.reserve(MAX_PATH);
		g_ExePath.resize(MAX_PATH);

		if (!(module_string_size = GetModuleFileName(nullptr, g_ExePath.data(), MAX_PATH))) {
			std::cerr << "GetModuleFileName() failed!  Code: " << GetLastError() << std::endl;
		}
		
		g_ExePath.resize(module_string_size);
		g_ExePath.shrink_to_fit();
		std::cout << "g_ExePath = \'" << g_ExePath << "\'" << std::endl;

		// Make the settings path
		g_IniPath = g_ExePath.substr(0, g_ExePath.find_last_of('.') + 1);
		g_IniPath += "ini";
		std::cout << "g_IniPath = \'" << g_IniPath << "\'" << std::endl;

		// Make the working path
		g_WorkingPath = g_ExePath.substr(0, g_ExePath.find_last_of('\\') + 1);
		std::cout << "g_WorkingPath = \'" << g_WorkingPath << "\'" << std::endl;

		// Load the settings
		std::unique_ptr<IniFile> ini(new IniFile);
		if (ini) {
			if (ini->Open(g_IniPath))
			{
				g->sGameDirectory = ini->GetKey<std::string>("General", "GameDir", "C:\\Program Files (x86)\\Carnivores 2\\");
				g->sGameDirectory = ini->GetKey<std::string>("General", "ProjectDir", "C:\\Program Files (x86)\\Carnivores 2\\");

				rc.left = ini->GetKey<signed>( "Window", "Left", 100 );
				rc.top = ini->GetKey<signed>( "Window", "Top", 80 );
				rc.right = rc.left + ini->GetKey<signed>( "Window", "Width", 800 );
				rc.bottom = rc.top + ini->GetKey<signed>( "Window", "Height", 600 );

				g->sRecentFile[0] = ini->GetKey<std::string>("RecentFiles", "File1", "");
				g->sRecentFile[1] = ini->GetKey<std::string>("RecentFiles", "File2", "");
				g->sRecentFile[2] = ini->GetKey<std::string>("RecentFiles", "File3", "");
				g->sRecentFile[3] = ini->GetKey<std::string>("RecentFiles", "File4", "");
				g->sRecentFile[4] = ini->GetKey<std::string>("RecentFiles", "File5", "");

				std::string RendererDriver = ini->GetKey<std::string>("Renderer", "API", "Direct3D 9");
				g->iColorBits = ini->GetKey<signed>("Renderer", "ColorBufferBits", 24);
				g->iDepthBits = ini->GetKey<signed>("Renderer", "DepthBufferBits", 16);
				g->sEnvironmentMapFile = ini->GetKey<std::string>("Renderer", "EnvironmentMapFile", "HUNTDAT\\FX\\ENVMAP.TGA");
				g->bSpecularReflection = ini->GetKey<bool>("Renderer", "SpecularMapping", true);
				g->bEnvironmentMapping = ini->GetKey<bool>("Renderer", "EnvironmentMapping", true);
			}
			else
			{
				// Write a default one
				ini->SetKey<std::string>("General", "GameDir", g->sGameDirectory);
				ini->SetKey<std::string>("General", "ProjectDir", g->sProjectDirectory);
				ini->SetKey<bool>("General", "OldFormatWarnings", g->bOldFormatWarnings);
				ini->SetKey<bool>("General", "EnforceVanillaLimits", g->bEnforceGameLimits);

				ini->SetKey<signed>("Window", "Left", CW_USEDEFAULT);
				ini->SetKey<signed>("Window", "Top", CW_USEDEFAULT);
				ini->SetKey<signed>("Window", "Width", 800);
				ini->SetKey<signed>("Window", "Height", 600);

				for (int i = 0; i < 5; i++)
				{
					std::stringstream ss;
					ss << "File" << (i + 1);
					ini->SetKey<std::string>("RecentFiles", ss.str(), g->sRecentFile[i]);
				}

				ini->SetKey<std::string>("Renderer", "API", "Direct3D 9");
				ini->SetKey<signed>("Renderer", "ColorBufferBits", g->iColorBits);
				ini->SetKey<signed>("Renderer", "DepthBufferBits", g->iDepthBits);
				ini->SetKey<std::string>("Renderer", "EnvironmentMapFile", g->sEnvironmentMapFile);
				ini->SetKey<bool>("Renderer", "EnvironmentMapping", g->bEnvironmentMapping);
				ini->SetKey<bool>("Renderer", "SpecularMapping", g->bSpecularReflection);

				if (!ini->Save(g_IniPath)) {
					std::cerr << "Failed to save the INI file!" << std::endl;
				}
			}
		}

		//TODO: Move this
		g_Camera.x = -24.0f;
		g_Camera.y = 0.0f;
		g_Camera.z = -24.0f;
		g_Camera.TargetX = 0.0f;
		g_Camera.TargetY = 0.0f;
		g_Camera.TargetZ = 0.0f;
		g_Camera.Dist = 24.0f;

		// Register window class
		std::memset(&wc, 0, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
		wc.lpszClassName = "C3DIT_CLASS";


		if (!RegisterClassEx(&wc))
		{
			uint32_t errorcode = GetLastError();
			// TODO: Do windows error checking
			throw std::runtime_error("RegisterClassEx() failed.");
			return 1;
		}

		InitializeCommonControls();

		/* create main window */
		g_Title += " " VERSION_STR;
		hwnd = CreateWindowEx(
			WS_EX_ACCEPTFILES,
			wc.lpszClassName,
			g_Title.c_str(),
			WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
			CW_USEDEFAULT, 0,
			800, 600,
			HWND_DESKTOP,
			NULL,
			hInstance,
			NULL);

		if (!IsWindow(hwnd))
		{
			throw std::runtime_error("Failed to create the main window!");
			return 1;
		}

		g_DrawArea = CreateWindow(
			WC_STATIC,
			NULL,
			WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
			rc.left, rc.top + 16, rc.right - 128, rc.bottom - 48,
			hwnd,
			NULL,
			hInstance,
			NULL
		);

		g_AniTrack = CreateWindow(
			TRACKBAR_CLASS,
			NULL,
			WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
			rc.left, rc.bottom - 48, rc.right - 128, 16,
			hwnd,
			NULL,
			hInstance,
			NULL
		);

		SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 1));
		SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);

		// -- Create the tool bar
		gHTool = CreateWindowEx(0,
			TOOLBARCLASSNAME,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			0, 0, 0, 0,
			g_hMain,
			(HMENU)IDR_TOOLBAR1,
			hInstance,
			NULL);

		// -- Create the status bar
		gHStatus = CreateWindowEx(0,
			STATUSCLASSNAME,
			NULL,
			WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
			0, 0, 0, 0,
			g_hMain,
			(HMENU)IDR_STATUSBAR,
			hInstance,
			NULL);

		/*
		// -- (MSDN)Useful for backwards compatability
		SendMessage(gHTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

		// -- Prepare the toolbar buttons
		TBBUTTON tbb[7];
		TBADDBITMAP tbab;
		tbab.hInst = hInstance;
		tbab.nID = ID_TB_IMAGE;

		SendMessage(gHTool, TB_ADDBITMAP, 4, (LPARAM)&tbab);

		// -- Make and define the toolbar buttons
		std::memset(tbb, 0, sizeof(TBBUTTON) * 7);
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

		SendMessage(gHTool, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON), (LPARAM)&tbb);
		*/

		// -- Prepare the status bar
		srand(g_Timer->GetTime() & 0xFFFFFFFF);
		int statwidths[] = { 180, -1 };
		SendMessage(gHStatus, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);
		//SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)"No Model!");
		//SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[rand() % 6].c_str());

		//ShowWindow(g_hMain, SW_SHOW);

		GetWindowRect(hwnd, &rc);
		std::cout << __LINE__ << " Window.Rect = {" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "}" << std::endl;

		POINT window_pos = { rc.left, rc.top };
		ScreenToClient(HWND_DESKTOP, &window_pos);
		SetWindowPos(hwnd, HWND_TOP, window_pos.x, window_pos.y, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);

		GetWindowRect(hwnd, &rc);
		std::cout << __LINE__ << " Window.Rect = {" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "}" << std::endl;

		UpdateWindow(hwnd);

		// -- Enable the video driver
		video.reset(new Renderer(g_DrawArea));

		g_Project.PrevTime = g_Timer->GetTime();

		while (true)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					break;
				}
				else {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else {
				// Get the mouse co-ords on the screen
				if (GetForegroundWindow() == hwnd)
				{
					POINT ms;
					RECT rc;
					GetCursorPos(&ms);
					CurX = ms.x;
					CurY = ms.y;

					mouseE();
					keyboardE();

					float bg_rgba[] = { 0.f, 0.f, 0.f, 1.f };
					video->ClearColorBuffer(bg_rgba);
					video->ClearDepthBuffer(1.0f);

					// Camera
					GetClientRect(g_DrawArea, &rc);

					if (g_Camera.Type == VIEW_PERSPECTIVE)
					{
						/*glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluPerspective(45.0f, (float)rc.right / (float)rc.bottom, 1.0f, 1000.0f);

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						gluLookAt(cam.x, cam.z, cam.y, cam.xt, cam.zt, cam.yt, 0.0f, 1.0f, 0.0f);*/
					}
					else if (g_Camera.Type == VIEW_FIRSTPERSON)
					{
						/*glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluPerspective(60.0f, (float)rc.right / (float)rc.bottom, 0.1f, 1000.0f);

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						gluLookAt(0, 0, 0, 0, 0, 1, 0.0f, 1.0f, 0.0f);*/
					}
					else if (g_Camera.Type == VIEW_ORTHOGRAPHIC)
					{
						/*glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluOrtho();

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						gluLookAt(cam.x, cam.z, cam.y, cam.xt, cam.zt, cam.yt, 0.0f, 1.0f, 0.0f);*/
					}

					/*double mvmatrix[16], projmatrix[16];
					int viewport[4];
					glGetIntegerv(GL_VIEWPORT, viewport);
					glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
					glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);*/

					// Draw the 3d model
					//RenderMesh();

					// Draw text overlay
					/*glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glOrtho(rc.left, rc.right, rc.bottom, rc.top, -1280, 1280);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();*/

					// -- Render a vertex in 2D space, mapped to the 3D location
					if (g->bShowAxis)
					{
						// TODO remake gluProject
						/*double _outX, _outY, _outZ;

						gluProject(5.5, 0, 0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
						DrawTextGL((float)_outX, (float)rc.bottom - _outY, 0, "X", 0xFF0000FF);
						gluProject(0, 5.5, 0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
						DrawTextGL((float)_outX, (float)rc.bottom - _outY, 0, "Y", 0xFF00FF00);
						gluProject(0, 0, 5.5, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
						DrawTextGL((float)_outX, (float)rc.bottom - _outY, 0, "Z", 0xFFFF0000);*/
					}

					/*switch (CameraView)
					{
					case VIEW_PERSPECTIVE: video->UI.DrawText(13, 13, 0, "Perspective", 0xFF9000FF); break;
					case VIEW_FIRSTPERSON: video->UI.DrawText(13, 13, 0, "First Person", 0xFF9000FF); break;
					case VIEW_ORTHOGRAPHIC: video->UI.DrawText(13, 13, 0, "Orthographic", 0xFF9000FF); break;
					}*/

					// -- Draw focus rectangle
					if (GetFocus() == hwnd)
					{
						/*glBegin(GL_LINE_LOOP);
						glColor4ub(255, 0, 0, 255);
						glVertex2f((float)rc.left + 2, (float)rc.top + 3);
						glVertex2f((float)rc.right - 3, (float)rc.top + 3);
						glVertex2f((float)rc.right - 3, (float)rc.bottom - 3);
						glVertex2f((float)rc.left + 3, (float)rc.bottom - 3);
						glEnd();*/
					}

					int64_t time = g_Timer->GetTime();
					g_Project.RealTime = time;
					g_Project.TimeDt = time - g_Project.PrevTime;
					if (g_Project.TimeDt < 0) g_Project.TimeDt = 10;
					if (g_Project.TimeDt > 10000) g_Project.TimeDt = 10;
					if (g_Project.TimeDt > 1000) g_Project.TimeDt = 1000;
					g_Project.PrevTime = g_Project.RealTime;

					// -- Draw the FPS
					/*std::stringstream ss;
					ss << "FPS: " << FPS;
					video->UI.DrawText(10, 10, ss.str(), 0xFF000);*/

					// Copy the buffer (Double Buffering)
					video->SwapBuffers();;

					// Frame limiter to 60 FPS
					int64_t t = g_Timer->GetTime();
					int64_t t_diff = t - g_Project.LastTick;

					if (t_diff < (1000L / 60L)) {
						std::this_thread::sleep_for(std::chrono::milliseconds((1000L / 60L) - t_diff));
					}

					g_Project.LastTick = g_Timer->GetTime();
				}
				else // ! Foreground Window
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}
			}
		}
	}
	catch (std::runtime_error& re) {
		std::cerr << "A Runtime Error has occured!\n\t" << re.what() << std::endl;
	}
	catch (std::exception& e) {
		// Some report
		std::cerr << "Unhandled Standard C++ Exception - Caught in main()" << std::endl;
		std::cerr << e.what() << std::endl;
	}

	//RECT rc;
	GetWindowRect(g_hMain, &rc);
	std::cout << __LINE__ << " Window.Rect = {" << rc.left << "," << rc.top << "," << rc.right << "," << rc.bottom << "}" << std::endl;

	// -- Save the program settings to INI file
	std::unique_ptr<IniFile> ini(new IniFile);
	if (ini) {
		ini->SetKey<std::string>("General", "GameDir", "C:\\Program Files (x86)\\Carnivores 2\\");
		ini->SetKey<std::string>("General", "ProjectDir", "Projects\\");
		ini->SetKey<bool>("General", "OldFormatWarnings", true);
		ini->SetKey<bool>("General", "EnforceVanillaLimits", false);

		ini->SetKey<signed>("Window", "Left", rc.left);
		ini->SetKey<signed>("Window", "Top", rc.top);
		ini->SetKey<signed>("Window", "Width", rc.right - rc.left);
		ini->SetKey<signed>("Window", "Height", rc.bottom - rc.top);

		for (int i = 0; i < 5; i++)
		{
			std::stringstream ss;
			ss << "File" << (i + 1);
			ini->SetKey<std::string>("RecentFiles", ss.str(), g->sRecentFile[i]);
		}

		ini->SetKey<std::string>("Renderer", "API", "Direct3D 9");
		ini->SetKey<signed>("Renderer", "ColorBufferBits", g->iColorBits);
		ini->SetKey<signed>("Renderer", "DepthBufferBits", g->iDepthBits);
		ini->SetKey<std::string>("Renderer", "EnvironmentMapFile", g->sEnvironmentMapFile);
		ini->SetKey<bool>("Renderer", "EnvironmentMapping", g->bEnvironmentMapping);
		ini->SetKey<bool>("Renderer", "SpecularMapping", g->bSpecularReflection);

		struct _dummy { uint32_t a; uint16_t b; };
		struct _dummy dummy = { 1024, 255 };
		ini->SetKeyStruct("General", "dummy", &dummy, sizeof(dummy));

		if (!ini->Save(g_IniPath)) {
			std::cerr << "Failed to save the INI file!" << std::endl;
		}
	}

	// Shutdown Renderer
	if (video)
		video.reset(nullptr);

	// -- Destroy the window explicitly
	if (g_FileView) DestroyWindow(g_FileView);
	if (g_DrawArea) DestroyWindow(g_DrawArea);
	if (hwnd) DestroyWindow(hwnd);

	// --Unregister class
	UnregisterClass(wc.lpszClassName, wc.hInstance);

	return msg.wParam;
}
