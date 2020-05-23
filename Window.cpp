
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
#include "C3Dit.h"
#undef GLOBALVAR_DEFINE

#include "IniFile.h"
#include "version.h"
#include "Global.h"
#include "Targa.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <string>


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


// Declare the Dialog Procedures
INT_PTR CALLBACK DlgAboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgAnimationProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgErrorProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgSoundProc(HWND, UINT, WPARAM, LPARAM);


enum TreeViewEnum
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


Globals* g = nullptr;
HINSTANCE hInst = nullptr;
std::string			g_Title = "Character 3D Editor";
POINT				g_CursorPos;
std::vector<HTREEITEM>      g_TVItems;
std::array<HTREEITEM, 12>   rootNodes;
RECT				g_TVRect;
std::string         g_ExePath = "";
std::string         g_WorkingPath = "";
std::string         g_IniPath = "";
std::string         g_ProjectPath = "";
int					g_CurrentAnimation = 0;
int					g_CurrentSound = 0;


/////////////////////////////////////////////////////////////////////////////////////
// Functions
bool SaveProject(const std::string& projectpath);
bool LoadProject(const std::string& projectpath);


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


INT_PTR CALLBACK DlgAnimationProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		// -- Populate the sound effect combobox
		HWND ani_list = GetDlgItem(hwnd, IDC_SOUND);
		ComboBox_ResetContent(ani_list);

		for (auto anim : g_Scene.Animations)
			ComboBox_AddString(ani_list, anim.Name.c_str());

		if (g_Scene.Animations.size())
			ComboBox_SetCurSel(ani_list, g_CurrentAnimation);

		// TODO: handle std::out_of_range exception here
		CVertexAnimation& anim = g_Scene.Animations.at(g_CurrentAnimation);

		SetDlgItemText(hwnd, IDC_NAME, anim.Name.c_str());
		SetDlgItemInt(hwnd, IDC_KPS, anim.KPS, true);
		//SetDlgItemInt(hwnd, IDC_TIME, 0u, FALSE); // Time length of animation
		SetDlgItemInt(hwnd, IDC_DATA_LENGTH, anim.FrameCount, false); // Frame Count

		// -- Populate the sound effect combobox
		HWND snd_list = GetDlgItem(hwnd, IDC_SOUND);
		ComboBox_ResetContent(snd_list);
		ComboBox_AddString(snd_list, "No sound");

		for (auto sound : g_Scene.SoundEffects)
			ComboBox_AddString(snd_list, sound.Name.c_str());

		ComboBox_SetCurSel(snd_list, anim.SoundFX + 1);
	}
	break;

	case WM_CLOSE: EndDialog(hwnd, IDCANCEL); return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL: EndDialog(hwnd, IDCANCEL); return true;
		case IDOK: {
			// Apply the AnimSFX table update
			/*Model.mAnimSFXTable[g_CurrentAnimation] = ComboBox_GetCurSel(GetDlgItem(hwnd, IDCB_SOUND));
			GetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[g_CurrentAnimation].Name, 32);
			g_Animations[g_CurrentAnimation].KPS = GetDlgItemInt(g_hAniDlg, ID_AKPS, NULL, FALSE);*/
			EndDialog(hwnd, IDOK);
		} return true;

		case IDC_SAVE:
		{
			std::string filepath = SaveFileDlg(hwnd, "Vertex Table (*.vtl)\0*.vtl\0", "vtl");

			if (filepath.empty())
				break;

			std::ofstream f(filepath, std::ios::binary);
			if (f.is_open()) {
				CVertexAnimation& anim = g_Scene.Animations.at(g_CurrentAnimation);
				size_t size = g_Scene.Model.Vertices.size() * anim.FrameCount * 3;

				f.write((char*)g_Scene.Model.Vertices.size(), 4);
				f.write((char*)anim.KPS, 4);
				f.write((char*)anim.FrameCount, 4);
				f.write((char*)anim.Data, size);
				f.close();
			}
			else {
				MessageBox(hwnd, "Unable to open the file for reading!", "File", MB_ICONEXCLAMATION);
			}
		}
		break;

		case IDC_OPEN:
		{
			std::string filename = OpenFileDlg(hwnd, "Vertex Table (*.vtl)\0*.vtl\0Animation (*.ani)\0*.ani\0", "vtl");

			if (filename.empty())
				break;

			/*FILE* afp;

			if ((afp = fopen(fileName, "rb")) == NULL)
			{
				MessageBox(hwnd, "Failed to open the file for reading.\r\nDoes it exist?", "File Error", MB_ICONWARNING);
				break;
			}

			if (strstr(strlwr(fileName), ".ani") != NULL)
			{
				long mVertCount = 0;
				fread(&g_Animations[g_CurrentAnimation].KPS, 1, 4, afp);
				fread(&g_Animations[g_CurrentAnimation].FrameCount, 1, 4, afp);
				fread(&mVertCount, 1, 4, afp);
				if (mVertCount != Model.mVertCount)
				{
					MessageBox(hwnd, "The vertex count in the animation does not match the model.", "File Error", MB_ICONWARNING);
					fclose(afp);
					break;
				}
				if (g_Animations[g_CurrentAnimation].Data) delete[] g_Animations[g_CurrentAnimation].Data;
				g_Animations[g_CurrentAnimation].Data = new short[Model.mVertCount * g_Animations[g_CurrentAnimation].FrameCount * 3];
				fread(g_Animations[g_CurrentAnimation].Data, Model.mVertCount * g_Animations[g_CurrentAnimation].FrameCount * 3, sizeof(uint16_t), afp);
			}
			else if (strstr(strlwr(fileName), ".vtl") != NULL)
			{
				long mVertCount = 0;
				fread(&mVertCount, 1, 4, afp);
				if (mVertCount != Model.mVertCount)
				{
					MessageBox(hwnd, "The vertex count in the animation does not match the model.", "File Error", MB_ICONWARNING);
					fclose(afp);
					break;
				}
				fread(&g_Animations[g_CurrentAnimation].KPS, 1, 4, afp);
				fread(&g_Animations[g_CurrentAnimation].FrameCount, 1, 4, afp);
				if (g_Animations[g_CurrentAnimation].Data) delete[] g_Animations[g_CurrentAnimation].Data;
				g_Animations[g_CurrentAnimation].Data = new short[Model.mVertCount * g_Animations[g_CurrentAnimation].FrameCount * 3];
				fread(g_Animations[g_CurrentAnimation].Data, Model.mVertCount * g_Animations[g_CurrentAnimation].FrameCount * 3, sizeof(uint16_t), afp);
			}

			fclose(afp);

			unsigned int time = (g_Animations[g_CurrentAnimation].FrameCount * 1000) / g_Animations[g_CurrentAnimation].KPS;

			SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[g_CurrentAnimation].Name);
			SetDlgItemInt(g_hAniDlg, ID_AKPS, g_Animations[g_CurrentAnimation].KPS, FALSE);
			SetDlgItemInt(g_hAniDlg, ID_ATIME, time, FALSE);
			SetDlgItemInt(g_hAniDlg, ID_AFRM, g_Animations[g_CurrentAnimation].FrameCount, FALSE);*/
		}
		break;
		}
		break;
	}

	return false;
}


INT_PTR CALLBACK DlgSoundProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		// -- Populate the sound effect combobox
		HWND snd_list = GetDlgItem(hwnd, IDC_SELECTION);
		ComboBox_ResetContent(snd_list);

		for (auto sound : g_Scene.SoundEffects)
			ComboBox_AddString(snd_list, sound.Name.c_str());

		ComboBox_SetCurSel(snd_list, g_CurrentSound);

		// TODO: handle std::out_of_range exception here
		CSound& sound = g_Scene.SoundEffects.at(g_CurrentSound);

		SetDlgItemText(hwnd, IDC_NAME, sound.Name.c_str());
		SetDlgItemInt(hwnd, IDC_DATA_LENGTH, sound.Length, false); // Frame Count
	}
	break;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL: EndDialog(hwnd, IDCANCEL); return true;
		case IDOK: EndDialog(hwnd, IDOK); return true;

		case IDC_OPEN: { /*importSound(ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION)));*/ } break;
		case IDC_SAVE: { /*exportSound(ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION)));*/ } break;

		case IDC_NEW: {
			CSound sound;
			sound.Name = "New sound";
			sound.Length = 0u;
			g_Scene.SoundEffects.push_back(sound);
		}
					break;

		case IDC_INSERT: break;
		case IDC_DELETE: break;

		case IDC_PLAY: { PlayWave(g_Scene.SoundEffects.at(g_CurrentSound)); } break;
		}
		break;
	}

	return false;
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


INT_PTR CALLBACK DlgObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		std::stringstream ss("");

		SetDlgItemText(hwnd, IDC_NAME, g_Scene.Model.Name.c_str());

		ss.str(""); ss.clear(); ss << g_Scene.ObjInfo.Radius;
		SetDlgItemText(hwnd, IDC_OBJECT_RADIUS, ss.str().c_str());

		ss.str(""); ss.clear(); ss << g_Scene.ObjInfo.YLo;
		SetDlgItemText(hwnd, IDC_OBJECT_YLOW, ss.str().c_str());

		ss.str(""); ss.clear(); ss << g_Scene.ObjInfo.YHi;
		SetDlgItemText(hwnd, IDC_OBJECT_YHIGH, ss.str().c_str());

		// TODO: The rest.
	}
	break;

	case WM_CLOSE: { EndDialog(hwnd, IDCANCEL); } break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			return true;
		}
		break;
	}

	return false;
}


void newScene()
{
	// -- Empty the texture
	g_Scene.Clear();

	ANIMPLAY = false;
	g_CurrentAnimation = 0;
	CUR_FRAME = 0;
	g_CurrentSound = 0;

	g_Camera.x = -24;
	g_Camera.y = -24;
	g_Camera.z = 24;
	g_Camera.TargetX = 0;
	g_Camera.TargetY = 0;
	g_Camera.TargetZ = 0;
	g_Camera.Dist = 24;
	g_Camera.Yaw = 0;
	g_Camera.Pitch = 0;
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
	std::string filepath = OpenFileDlg(g_hMain, "All Supported (*.car;*.c2o;*.3df;*.obj;)\0*.car;*.c2o;*.3df;*.3dn;*.obj\0"
		"Character files (*.car)\0*.car\0"
		"Carnivores 2 Object (*.c2o)\0*.c2o\0"
		"3D File (*.3df)\0*.3df\0"
		"iOS/AOS 3D model (*.3dn)\0*.3dn\0"
		"Wavefront OBJ File (.obj)\0*.obj\0",
		"CAR");

	if (filepath.empty())
	{
		return;
	}

	newScene();

	/*if (strstr(_strlwr(fileName), ".car") != NULL)
	{
		LoadCARData(fileName);
	}
	if (strstr(_strlwr(fileName), ".3dn") != NULL)
	{
		Load3DNData(fileName);
	}
	if (strstr(_strlwr(fileName), ".c2o") != NULL)
	{
		LoadC2OData(fileName);
	}
	if (strstr(_strlwr(fileName), ".c1o") != NULL)
	{
		//LoadC1OData(fileName);
	}
	if (strstr(_strlwr(fileName), ".3df") != NULL)
	{
		Load3DFData(fileName);
	}
	if (strstr(strlwr(fileName), ".obj") != NULL)
	{
		LoadOBJData(fileName);
	}*/
}


const int iProjectVersion = 1;

bool LoadProject(const std::string& filepath)
{
	std::string line = "";
	std::ifstream fs(filepath);

	if (!fs.is_open())
	{
		return false;
	}



	while (!std::getline(fs, line).eof())
	{
		std::stringstream ss;
		ss << std::uppercase << line.substr(line.find_first_of(' '));
		std::string token = ss.str();
		ss.str(""); ss.clear();

		if (!token.compare("VERSION"))
		{
			// Get the version from line
			ss.str(line.substr(line.find_first_of(' ') + 1 , std::string::npos));
			
		}
		else
		{
		}
	}

	//fs << "Version " << iProjectVersion << std::endl;

	return true;
}


void loadCAR(char* fname)
{
	/* loadCAR()
	* Loads one of the supported geometry mediums to a blank scene.
	* Supports:
		CAR - Character Format
		C2O - Carnivores 2 Object Format
		3DF - Action Forms 3D File Format
		OBJ - Wavefront OBJ Format
	*/
	std::stringstream ss;
	ss << std::uppercase << fname;
	std::string filepath = ss.str().substr(ss.str().find_last_of('.'), std::string::npos);

	newScene();

	if (!filepath.compare(".CAR"))
	{
		LoadCARData(fname);
	}
	else if (!filepath.compare(".3DF"))
	{
		Load3DFData(fname);
	}
	else if (!filepath.compare(".C2O"))
	{
		LoadC2OData(fname);
	}
	else if (!filepath.compare(".OBJ"))
	{
		LoadOBJData(fname);
	}
}


/*
	void SaveProject()
	This function saves the current scene as the desired model format.
	This deprecated the import/export model feature.
*/
bool SaveProject(const std::string& projectpath = "")
{
	if (projectpath.empty()) {
		projectpath = SaveFileDlg("All Supported (*.car;*.c2o;*.3df;*.obj;*.cmf;)\0*.car;*.c2o;*.3df;*.obj*.cmf\0"
			"Character files (*.car)\0*.car\0"
			"Carnivores 2 Object (*.c2o)\0*.c2o\0"
			"3D File (*.3df)\0*.3df\0"
			"Text Format (.cmf)\0*.cmf\0"
			"Wavefront OBJ File (.obj)\0*.obj\0", "car");
		if (projectpath.empty()) {
			return false;
		}
	}

	std::string fileext = projectpath.substr(projectpath.find_last_of('.'));
	// Crude case swap
	for (auto chr : fileext) {
		if (chr >= 'A' && chr <= 'Z') {
			chr = 'a' + ((int)chr - (int)'A');
		}
	}

	//--Find out what kind of file this is
	if (!fileext.compare(".car")) SaveCARData(projectpath);
	if (!fileext.compare(".c2o"))) SaveC2OData(projectpath);
	if (!fileext.compare(".3df"))) Save3DFData(projectpath);
	if (!fileext.compare(".obj"))) SaveOBJData(projectpath);
	if (!fileext.compare(".cmf"))) SaveCMFData(projectpath);
	if (!fileext.compare(".3dn"))) Save3DNData(projectpath);

	return true;
}


void keyboardE()
{
	uint8_t KeyState[256];
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
}

int LastMidMouseX = 0;
int LastMidMouseY = 0;

void mouseE()
{
	if (mouse[0])
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

			if (g_Camera.Type > VIEW_PERSPECTIVE + 1)
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
				cam.xt += lengthdir_x(disth, -cam.yaw - 90, 0);
				cam.yt += lengthdir_y(disth, -cam.yaw + 90, 0);
				cam.zt += lengthdir_z(distz, -cam.pitch - 90);
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
			cam.yaw += (CurX - mx);
			cam.pitch += (CurY - my);

			if (cam.pitch >= 89) cam.pitch = 89;
			if (cam.pitch <= -89) cam.pitch = -89;

			mx = CurX;
			my = CurY;
		}
	}
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


std::string OpenFileDlg(HWND parent, const std::string& filter, const std::string& ext, const std::string& title = "")
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	std::memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = parent;
	ofn.lpstrFilter = filter.c_str();
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


std::string SaveFileDlg(HWND parent, const std::string& filter, const std::string& ext, const std::string& title = "")
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	std::memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME); // SEE NOTE BELOW
	ofn.hwndOwner = parent;
	ofn.lpstrFilter = filter.c_str();
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
}

// Utility macro to more easily check a 4-byte string ID against a DWORD/UINT32
#define BLOCKID_UINT(a) static_cast<uint32_t>(a[0] + a[1] + a[2] + a[3])
bool CSound::Load(const std::string& filepath)
{
	// TODO: Support Vorbis OGG in the future

	//std::string filepath = OpenFileDlg(g_hMain, "WAVE sound file (.wav)\0*.wav\0", "wav");
	if (filepath.empty())
		return false;

	std::ifstream fs(filepath, std::ios::binary);
	if (!fs.is_open())
	{
		MessageBox(g_hMain, "Unable to open the file for reading.", "File Error", MB_ICONWARNING);
		return false;
	}
	
	uint32_t block_ID = 0u;
	uint32_t block_size = 0u;

	fs.read((char*)&block_ID, 4);
	fs.read((char*)&block_size, 4);

	if (block_ID != BLOCKID_UINT("RIFF"))
	{
		MessageBox(g_hMain, "Expected a RIFF block but didn't read one.", "File Error", MB_ICONWARNING);
		return false;
	}

	WAVEFORMATEX wf;
	uint32_t wavedata_length = 0u;
	std::unique_ptr<uint8_t[]> pcm_data(nullptr);
	std::memset(&wf, 0, sizeof(WAVEFORMATEX));

	while (!fs.eof() && !fs.fail())
	{
		fs.read((char*)&block_ID, 4);
		fs.read((char*)&block_size, 4);

		if (block_ID == BLOCKID_UINT("WAVE"))
		{
			fs.read((char*)&block_ID, 4); // "fmt "

			if (block_ID != BLOCKID_UINT("fmt "))
			{
				std::cerr << "Expected \'fmt \' block!" << std::endl;
				return false;
			}

			fs.read((char*)&block_size, 4);
			fs.read((char*)&wf, (block_size <= sizeof(WAVEFORMATEX)) ? block_size : sizeof(WAVEFORMATEX));

			if (block_size > sizeof(WAVEFORMATEX))
			{
				std::cerr << "Warning! the \'fmt \' block size is larger than the structure WAVEFORMATEX!" << std::endl;
				fs.seekg(block_size - sizeof(WAVEFORMATEX), std::ios::cur);
			}

			fs.read((char*)&block_ID, 4); // "fmt "

			if (block_ID != BLOCKID_UINT("data"))
			{
				std::cerr << "Expected \'data\' block!" << std::endl;
				return false;
			}

			fs.read((char*)&wavedata_length, 4);
			pcm_data.reset(new uint8_t[wavedata_length]);
			fs.read((char*)&pcm_data, wavedata_length);
		}
		else // Make a note of unhandled blocks in the error log
		{
			std::cerr << "Unhandled RIFF block: \'";
			for (int i = 0; i < 4; i++) { std::cerr.put(reinterpret_cast<char*>(&block_ID)[i]); }
			std::cerr << "\'" << std::endl;

			fs.seekg(block_size, std::ios::cur);
		}
	}
	fs.close();

	//--Header check
	if (wf.wFormatTag != WAVE_FORMAT_PCM)
	{
		MessageBox(g_hMain, "The sound file is not an uncompressed PCM waveform sound!", "Warning", MB_ICONWARNING);
		return false;
	}
	if (wf.nChannels != 1)
	{
		MessageBox(g_hMain, "The sound file is not a Mono sound!", "Warning", MB_ICONWARNING);
		return false;
	}
	if (wf.nSamplesPerSec != 22050)
	{
		MessageBox(g_hMain, "The sound file does not have a frequency of 22.05 kHz!", "Warning", MB_ICONWARNING);
		return false;
	}
	if (wf.nBlockAlign != 2)
	{
		MessageBox(g_hMain, "The sound file does not have a block size of 2 bytes!", "Warning", MB_ICONWARNING);
		return false;
	}
	if (wf.wBitsPerSample != 16)
	{
		MessageBox(g_hMain, "The sound file is not a 16-bit sound!", "Warning", MB_ICONWARNING);
		return false;
	}

	this->SampleCount = (wavedata_length / wf.nBlockAlign) / wf.nChannels;
	this->Length = wavedata_length;

	// Reset the waveform data pointer
	if (this->Data)
		delete[] this->Data;
	this->Data = new int16_t[this->Length / 2];

	// Convert the waveform (if applicable)
	//...

	std::memcpy(this->Data, pcm_data.get(), wavedata_length);

	return true;
}


bool CTexture::Load(const std::string& filepath)
{
	std::string filepath = OpenFileDlg(g_hMain, "All Supported Files\0*.bmp;*.tga\0TGA image (.tga)\0*.tga\0Windows Bitmap (.bmp)\0*.bmp\0", "tga");
	std::string fileext = "";

	if (filepath.empty())
	{
		return false;
	}
	else
	{
		std::stringstream ss;
		ss << std::nouppercase << filepath.substr(filepath.find_last_of('.'), std::string::npos);
		fileext = ss.str();
	}

	int tex_width = 0;
	int tex_height = 0;
	int tex_bits = 0;

	std::ifstream fs(filepath, std::ios::binary);
	if (!fs.is_open())
	{
		MessageBox(g_hMain, "Failed to open the texture file!", "File Error", MB_ICONERROR);
		return false;
	}

	if (!fileext.compare(".tga"))
	{
		TARGAINFOHEADER tih;
		fs.read((char*)&tih, sizeof(TARGAINFOHEADER));

		if (tih.tgaColorMapType != 0 || tih.tgaImageType != 2)
		{
			MessageBox(g_hMain, "", "", MB_ICONWARNING);
			return false;
		}

		tex_width = tih.tgaWidth;
		tex_height = tih.tgaHeight;
		tex_bits = tih.tgaBits;
	}
	else if (!fileext.compare(".bmp"))
	{
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;

		fs.read((char*)&bfh, sizeof(BITMAPFILEHEADER));
		fs.read((char*)&bih, sizeof(BITMAPINFOHEADER));

		fs.seekg(bfh.bfOffBits, std::ios::beg);

		if (bih.biCompression != BI_RGB)
		{
			MessageBox(g_hMain, "The bitmap file is incompatible because it is not uncompressed!", "File Error", MB_ICONWARNING);
			return false;
		}
		else if (bih.biClrUsed != 0)
		{
			MessageBox(g_hMain, "The bitmap file is incompatible because it has a color table!", "File Error", MB_ICONWARNING);
			return false;
		}

		tex_width = bih.biWidth;
		tex_height = bih.biHeight;
		tex_bits = bih.biBitCount;
	}
	else if (!fileext.compare(".png"))
	{
		MessageBox(g_hMain, "PNG files are not supported currently!", "File Error", MB_ICONWARNING);
		return false;
	}
	else
	{
		MessageBox(g_hMain, "Unknown file format!", "File Error", MB_ICONWARNING);
		return false;
	}

#ifdef _DEBUG
	std::cout << "Texture::Load(" << filepath << ") W=" << tex_width << " H=" << tex_height << " BPP=" << tex_bits << std::endl;
#endif

	//--Error catch
	if (tex_bits != 16 && tex_bits != 24 && tex_bits != 32)
	{
		MessageBox(g_hMain, "The image file is not a 16-bit, 24-bit or 32-bit image!", "File Error", MB_ICONWARNING);
		return false;
	}

	g_Scene.Texture.Width = tex_width;
	g_Scene.Texture.Height = tex_height;

	if (g_Scene.Texture.Data)
		delete[] g_Scene.Texture.Data;
	g_Scene.Texture.Data = new RGBA[tex_width * tex_height];
	std::memset(g_Scene.Texture.Data, 0, tex_width * tex_height * sizeof(RGBA));

	if (g_Scene.Texture.Data != nullptr)
	{
		uint8_t* tex_data = nullptr;
		
		try { tex_data = new uint8_t[tex_width * tex_height * (tex_bits >> 3)]; }
		catch (std::bad_alloc& ba) { std::cerr << "CTexture::Load() : Failed to allocate array!" << std::endl; return false; };

		// Convert the image to 32-bit RGBA
		for (auto i = 0u; i < tex_width * tex_height; i++)
		{
			if (tex_bits == 16) {
				g_Scene.Texture.Data[i] = *(reinterpret_cast<RGBA16*>(tex_data) + i);
			}
			else if (tex_bits == 24) {
				g_Scene.Texture.Data[i] = *(reinterpret_cast<RGB*>(tex_data) + i);
			}
			else if (tex_bits == 32) {
				g_Scene.Texture.Data[i] = *(reinterpret_cast<RGBA*>(tex_data) + i);
			}
		}

		delete[] tex_data;
	}

	/*
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);

	if (bpp == 16) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, w, h, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if (bpp == 24) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Model.mTexture);
	if (bpp == 32) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Model.mTexture);
	*/

	return true;
}


bool CSound::Save(const std::string& filepath)
{
	// Move the following to places where `importSound()` are called
	//filepath = SaveFileDlg(g_hMain, "WAVE sound file (.wav)\0*.wav\0", "wav");

	if (filepath.empty())
	{
		return false;
	}

	std::ofstream fs(filepath, std::ios::binary);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, "Failed to open the WAV file for writing!", "File Error", MB_ICONWARNING);
		return false;
	}

	uint32_t block_size = 0;

	fs.write("RIFF", 4);
	fs.write((char*)&block_size, 4);

	fs.write("WAVE", 4); // Chunk
	fs.write("fmt ", 4);

	block_size = sizeof(WAVEFORMATEX);
	fs.write((char*)&block_size, 4);

	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1u;
	wf.nSamplesPerSec = 22050u;
	wf.nAvgBytesPerSec = wf.nChannels * sizeof(int16_t) * wf.nSamplesPerSec;
	wf.nBlockAlign = wf.nChannels * sizeof(int16_t);
	wf.wBitsPerSample = 16u;
	wf.cbSize = 0u;

	fs.write((char*)&wf, sizeof(WAVEFORMATEX));

	fs.write("data", 4);
	fs.write((char*)&this->Length, 4);
	fs.write((char*)&this->Data, this->Length);

	std::string date = "XXXX-XX-XX"; // TODO: Use <chrono> to get the date
	std::string software = "Carnivores Character 3D Editor";

	// NOTE: LIST chunk isn't necessary but it's a nice touch!
	block_size = 4 + 4 + (date.size()+1) + 4 + (software.size()+1);
	fs.write("LIST", 4);
	fs.write((char*)&block_size, 4);
	fs.write("INFO", 4); // Chunk
	fs.write("ICRD", 4);
	fs.write(date.c_str(), date.size()+1);
	fs.write("ISFT", 4);
	fs.write(software.c_str(), software.size()+1);

	// Go back and correct the RIFF block size.
	block_size = static_cast<uint32_t>(fs.tellp()) - 8; // subtract 8 to ignore the RIFF ID and size
	fs.seekp(4, std::ios::beg);
	fs.write((char*)&block_size, 4);
	fs.seekp(0, std::ios::end);
}


bool CTexture::Save(const std::string& filepath)
{
	std::string filepath = SaveFileDlg(g_hMain, "TrueVision Targa Image (.tga)\0*.tga\0Windows Bitmap Image (.bmp)\0*.bmp\0", "tga");
	std::string fileext = "";

	if (filepath.empty())
	{
		return false;
	}
	else
	{
		std::stringstream ss;
		ss << std::uppercase << filepath.substr(filepath.find_last_of('.'), std::string::npos);
		fileext = ss.str();
	}

	std::ofstream fs(filepath, std::ios::binary);
	if (!fs.is_open())
	{
		return false;
	}

	if (!fileext.compare(".TGA"))
	{
		TARGAINFOHEADER tih;

		std::memset(&tih, 0, sizeof(TARGAINFOHEADER));
		tih.tgaIdentSize = 0;
		tih.tgaColorMapType = false;
		tih.tgaImageType = TGA_IMAGETYPE_RGB;
		tih.tgaWidth = g_Scene.Texture.Width;
		tih.tgaHeight = g_Scene.Texture.Height;
		tih.tgaBits = 32;

		fs.write((char*)&tih, sizeof(TARGAINFOHEADER));
	}
	else if (!fileext.compare(".BMP"))
	{
		BITMAPFILEHEADER hdr;
		BITMAPINFOHEADER bmi;

		std::memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biWidth = g_Scene.Texture.Width;
		bmi.biHeight = g_Scene.Texture.Height;
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage = g_Scene.Texture.Width * g_Scene.Texture.Height * sizeof(RGBA);

		std::memset(&hdr, 0, sizeof(BITMAPFILEHEADER));
		hdr.bfType = 'B'+'M'; //0x4d42
		hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + bmi.biSize + bmi.biSizeImage);
		hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + bmi.biSize;

		fs.write((char*)&hdr, sizeof(BITMAPFILEHEADER));
		fs.write((char*)&bmi, sizeof(BITMAPINFOHEADER));
	}
	else if (!fileext.compare(".PNG"))
	{
		return false;
	}

	fs.write((char*)g_Scene.Texture.Data, g_Scene.Texture.Width * g_Scene.Texture.Height * sizeof(RGBA));
	
	return true;
}


void exportModel()
{
	std::string filepath = SaveFileDlg(g_hMain, "3D Model File (.3DF)\0*.3df\0Wavefront .OBJ (.obj)\0*.obj\0", "3df");
	std::string fileext = "";

	if (filepath.empty())
	{
		return;
	}
	else
	{
		std::stringstream ss;
		ss << std::uppercase << filepath.substr(filepath.find_last_of('.'), std::string::npos);
		fileext = ss.str();
	}

	if (!fileext.compare(".3DF"))
	{
		Save3DFData(filepath);
	}
	else if (!fileext.compare(".OBJ"))
	{
		SaveOBJData(filepath);
	}
}

/*
	void importModel()
	DEPRECATED: Use void LoadProject() instead!
*/
void importModel()
{
	if (!OpenFileDlg("3D File (*.3DF)\0*.3DF\0Wavefront OBJ File (.obj)\0*.obj\0", "3DF"))
		return;

	if (strstr(strlwr(fileName), ".3df") != NULL)
	{
		Load3DFData(fileName);
	}

	if (strstr(strlwr(fileName), ".obj") != NULL)
	{
		LoadOBJData(fileName);
	}

	// Set Status bar
	char sstr[64];
	sprintf(sstr, "Triangles: %ld", Model.mFaceCount);
	SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
	MessageBox(g_hMain, sstr, "triangles", MB_OK);
}


void PlayWave(CSound& snd)
{
	// We have to save the .WAV file temporarily

	if (snd.Length == 0) return;
	if (snd.Data == nullptr) return;
	if ((int)snd.Data == 0xFFFFFFFF) return; //What?

	char out[MAX_PATH];
	sprintf(out, "%s%s", g_WorkingPath, "temp_sample.wav");

	FILE* fp = fopen(out, "wb");

	if (fp == 0)
	{
		printf("Failed to create/open the new file!\n");
		return;
	}
	long lval = 0;
	short sval = 0;

	fwrite("RIFF", 4, 1, fp);
	lval = snd.Length + (44 - 8);
	fwrite(&lval, 1, 4, fp);
	fwrite("WAVEfmt ", 8, 1, fp);
	lval = 16;
	fwrite(&lval, 1, 4, fp);
	sval = 1;
	fwrite(&sval, 1, 2, fp);
	sval = 1;
	fwrite(&sval, 1, 2, fp);
	lval = 22050;
	fwrite(&lval, 1, 4, fp);
	lval = 22050 * (1 * 16 / 8);
	fwrite(&lval, 1, 4, fp);
	sval = (1 * 16 / 8);
	fwrite(&sval, 1, 2, fp);
	sval = 16;
	fwrite(&sval, 1, 2, fp);
	fwrite("data", 4, 1, fp);
	lval = snd.Length;
	fwrite(&lval, 1, 4, fp);

	fwrite(snd.Data, snd.Length, 1, fp);
	fclose(fp);

	PlaySound(out, NULL, SND_ASYNC);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	hMenu = GetMenu(g_hMain);

	switch (uMsg)
	{
	case WM_CREATE:
	{
	}
	break;

	case WM_CLOSE:
	{
		// TODO: check if the scene is saved, if not, ask if we want to quit without saving
		//if ( MessageBox(hwnd, "The project has unsaved changes, are you sure you want to exit?", "Unsaved changes", MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		PostQuitMessage(0);
	}
	break;

	case WM_SIZE:
	{
		WinW = LOWORD(lParam);
		WinH = HIWORD(lParam);
		//glViewport( 0, 0, LOWORD(lParam), HIWORD(lParam) );
		RECT rcTool, rcStatus, rcClient, rc;
		int iToolHeight, iStatusHeight;

		// Get Rects
		GetClientRect(g_hMain, &rcClient);
		GetWindowRect(g_FileView, &g_TVRect);

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
		SetRect(&rc, rcClient.left, rcClient.top + iToolHeight, rcClient.right - 200, rcClient.bottom - iToolHeight - iStatusHeight);
		MoveWindow(g_DrawArea, rc.left, rc.top, rc.right, rc.bottom - 32, TRUE);

		MoveWindow(g_AniTrack, rc.left, rc.top + rc.bottom - 32, rc.right, 32, TRUE);

		// File Components View
		SetRect(&rc, rc.left + rc.right, rc.top, 200, rc.bottom);
		MoveWindow(g_FileView, rc.left, rc.top, rc.right, rc.bottom, TRUE);

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
			/*LPNM_TREEVIEW nmtv = (LPNM_TREEVIEW)lParam;
			HTREEITEM ParItem = TreeView_GetParent(g_FileView, nmtv->itemNew.hItem);

			if (ParItem == rootNodes[TV_TRIANGLES])
				if (GetKeyState(VK_CONTROL) & 0x8000)
				{
					// -- Selection
					g_TriSelection.push_back(((UINT)nmtv->itemNew.lParam - (UINT)&g_Triangles[0]) / sizeof(TRIANGLE));
				}
				else
				{
					g_TriSelection.clear();
					g_TriSelection.push_back(((UINT)nmtv->itemNew.lParam - (UINT)&g_Triangles[0]) / sizeof(TRIANGLE));
				}

			if (ParItem == rootNodes[TV_ANIMATIONS]) // Animations
			{
				g_CurrentAnimation = ((UINT)nmtv->itemNew.lParam - (UINT)&g_Animations[0]) / sizeof(TVtl);
				CUR_FRAME = 0;
				SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, (g_Animations[g_CurrentAnimation].FrameCount - 1)));
				SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);

			}

			if (ParItem == rootNodes[TV_SOUNDS]) // Animations
			{
				Snd = ((UINT)nmtv->itemNew.lParam - (UINT)&g_Sounds[0]) / sizeof(SOUND);
			}*/
		}
		break;

		case NM_RCLICK:
		{
			/*LPNMHDR nmh = (LPNMHDR)lParam;
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

				if (SelItem == rootNodes[TV_MESHES]) // Mesh
				{
					//GetCursorPos(&p);
					//HMENU hPopupMenu = CreatePopupMenu();
					//InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Rename");
					//InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Rename");
					//InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 201, "Open");
					//SetForegroundWindow(hwnd);
					//TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
				}
				if (SelItem == rootNodes[TV_ANIMATIONS]) // Animations
				{
					GetCursorPos(&p);
					HMENU hPopupMenu = CreatePopupMenu();
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_CLEAR, "Clear");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_ADD, "Add");
					SetForegroundWindow(hwnd);
					TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
				}
				if (SelItem == rootNodes[TV_SOUNDS]) // Sounds
				{
					GetCursorPos(&p);
					HMENU hPopupMenu = CreatePopupMenu();
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_CLEAR + 10, "Clear");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_SFX_ADD, "Add");
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

					g_CurrentAnimation = ((UINT)ti.lParam - (UINT)&g_Animations[0]) / sizeof(TVtl);
					printf("SelAnim = %u\n", Ani);

					GetCursorPos(&p);
					HMENU hPopupMenu = CreatePopupMenu();
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_RENAME + 20, "Rename");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_APLAY, "Play");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_REMOVE + 20, "Remove");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_ADD + 20, "Insert");
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

					Snd = ((UINT)ti.lParam - (UINT)&g_Sounds[0]) / sizeof(SOUND);
					printf("SelSound = %u\n", Snd);

					GetCursorPos(&p);
					HMENU hPopupMenu = CreatePopupMenu();
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Rename");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_SPLAY, "Play");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Remove");
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 201, "Insert");
					SetForegroundWindow(hwnd);
					TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
				}
			}*/
		}
		break;

		case NM_CLICK:
		{
			LPNMHDR nh = (LPNMHDR)lParam;
			std::cout << "BN_CLICKED: " << nh->idFrom << std::endl;
		}
		break;

		case TVN_ENDLABELEDIT:
		{
			/*LPNMTVDISPINFO ed = (LPNMTVDISPINFO)lParam;
			TVITEM ti;

			std::memset(&ti, 0, sizeof(TVITEM));
			ti.hItem = ed->item.hItem;
			ti.mask = TVIF_HANDLE | TVIF_PARAM;

			TreeView_GetItem(g_FileView, &ti);

			// -- Animations
			if ((LPARAM)&g_Animations[0] <= ti.lParam <= (LPARAM)&g_Animations[Model.mAnimCount])
			{
				if (ed->item.mask & TVIF_TEXT)
				{
					size_t len = ed->item.cchTextMax;
					TVtl* lpAnim = (TVtl*)ti.lParam;
					strncpy(lpAnim->Name, ed->item.pszText, (len <= 32) ? len : 32);
				}
			}
			// -- Sounds
			if ((LPARAM)&g_Sounds[0] <= ti.lParam <= (LPARAM)&g_Sounds[Model.mSoundCount])
			{
				size_t len = strlen(ed->item.pszText);
				SOUND* lpSound = (SOUND*)ti.lParam;
				strncpy(lpSound->name, ed->item.pszText, (len <= 32) ? strlen(ed->item.pszText) : 32);
			}

			ti = ed->item;
			TreeView_SetItem(g_FileView, &ti);*/
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
	}
	return 0;

	case WM_COMMAND:
	{
		switch (wParam)
		{
		case ID_FILE_NEW: newScene(); break;
		case ID_FILE_OPEN: loadCAR(); break;
		case ID_FILE_SAVE: SaveProject(g_ProjectPath); break;
		case ID_FILE_SAVEAS: SaveProject(""); break;
		case ID_HELP_ABOUT: { DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, DlgAboutProc); } break;
		case ID_EDITOR_UPDATE:
		{
			// TODO: Run a check for updates then retrieve
		}
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
			for (int t = 0; t < Model.mFaceCount; t++)
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
			bool vlist[1024];
			std::memset(vlist, 0, sizeof(bool) * 1024);
			for (auto f = 0u; f < g_Scene.Model.Faces.size(); f++)
			{
				int v1 = g_Scene.Model.Faces[f].v1;
				int v2 = g_Scene.Model.Faces[f].v2;
				int v3 = g_Scene.Model.Faces[f].v3;
				g_Normals[f] = ComputeNormals(g_Scene.Model.Vertices[v1], g_Scene.Model.Vertices[v2], g_Scene.Model.Vertices[v3]);
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

		case ID_TEXTURE_EXPORT: exportTexture(); break;
		case ID_TEXTURE_IMPORT: importTexture(); break;
			//case ID_ANIMATION_EXPORT: exportAnimation(int); break;
			//case ID_ANIMATION_IMPORT: importAnimation(); break;

			/*case IDF_TOOLS:
			{
				ShowWindow(g_hTools, SW_SHOW);
			}
			break;*/

		case ID_TOOLS_ANIMATION:
		{
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
			/*ShowWindow(g_hSndDlg, SW_SHOW);
			sprintf(str, "%d Kbs", (int)g_Sounds[Snd].len / 1000);

			SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
			SetDlgItemText(g_hSndDlg, ID_SIZE, str);*/
		}
		break;

		case ID_MODEL_OBJECTINFO:
		{
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_OBJECT_PROPERTIES), hwnd, DlgObjectProc) == IDOK)
			{
				// Apply the properties
			}
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
		key[wParam] = TRUE;

		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
	break;

	case WM_KEYUP:
		key[wParam] = FALSE;
		break;

	case WM_DROPFILES:
	{
		HDROP drop = (HDROP)wParam;
		char file[256];

		for (UINT i = 0; i < DragQueryFile(drop, 0xFFFFFFFF, file, 256); i++)
		{
			if (!DragQueryFile(drop, i, file, 256)) continue;
			MessageBox(hwnd, file, "Dropped", MB_OK);
		}

		DragQueryFile(drop, 0, file, 256);

		DragFinish(drop);

		//loadCAR(file);
	}
	break;

	case WM_LBUTTONDOWN:
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
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}


#ifdef _DEBUG
int main(int argc, char* argv[])
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	LPSTR lpCmdLine = "";
	int nCmdShow = SW_SHOW;
#else
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
#endif
	hInst = hInstance;
	WNDCLASSEX  wc;
	HWND        hwnd;
	HDC         hDC;
	HGLRC       hRC;
	MSG         msg;
	bool        bQuit = false;
	RECT        rc;

	g = GlobalContainer::GlobalVar();

	try {

		// -- Get the executable path
		g_ExePath.reserve(MAX_PATH);
		GetModuleFileName(nullptr, g_ExePath.data(), MAX_PATH);

		// -- Make the settings path
		g_IniPath = g_ExePath.substr(0, g_ExePath.find_last_of('.') + 1);
		g_IniPath += "ini";

		// -- Make the working path
		g_WorkingPath = g_ExePath.substr(0, g_ExePath.find_last_of('\\') + 1);

		// -- Load the settings
		IniFile* ini = new IniFile();
		if (ini) {
			ini->Load(g_IniPath);

			g->sGameDirectory = ini->ReadKeyString("General", "game", "C:\\Program Files (x86)\\Carnivores 2\\");
			g->sEnvironmentMap = ini->ReadKeyString("General", "envmap", "ENVMAP.TGA");
			g->sSpecularMap = ini->ReadKeyString("General", "specular", "SPECULAR.TGA");

			WinX = CW_USEDEFAULT;//ini->ReadKeyInt( "Window", "left", 100 );
			WinY = 0;//ini->ReadKeyInt( "Window", "top", 80 );
			WinW = 800;//ini->ReadKeyInt( "Window", "width", 800 );
			WinH = 600;//ini->ReadKeyInt( "Window", "height", 600 );

			g->sRecentFile[0] = ini->ReadKeyString("RecentFiles", "file1", "");
			g->sRecentFile[1] = ini->ReadKeyString("RecentFiles", "file2", "");
			g->sRecentFile[2] = ini->ReadKeyString("RecentFiles", "file3", "");
			g->sRecentFile[3] = ini->ReadKeyString("RecentFiles", "file4", "");
			g->sRecentFile[4] = ini->ReadKeyString("RecentFiles", "file5", "");

			g->iColorBits = ini->ReadKeyInt("OpenGL", "color", 24);
			g->iDepthBits = ini->ReadKeyInt("OpenGL", "depth", 16);

			delete ini;
			ini = nullptr;
		}

		//TODO: Move this
		cam.x = -24.0f;
		cam.y = -24.0f;
		cam.z = 0.0f;
		cam.xt = 0.0f;
		cam.yt = 0.0f;
		cam.zt = 0.0f;
		cam.dist = 24.0f;

		// Register window class
		std::memset(&wc, 0, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(COLOR_WINDOW + 1);// (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
		wc.lpszClassName = "C3DIT_CLASS";


		if (!RegisterClassEx(&wc))
		{
			/*
			if (g_UseExceptions)
				throw std::runtime_error("RegisterClassEx() failed.");
				*/
			return 1;
		}

		InitializeCommonControls();

		/* create main window */
		g_Title += VERSION_STR;
		g_hMain = CreateWindowEx(
			WS_EX_ACCEPTFILES,
			wc.lpszClassName,
			g_Title.c_str(),
			WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			WinX, WinY,
			WinW, WinH,
			NULL,
			NULL,
			hInstance,
			NULL);

		GetClientRect(g_hMain, &rc);

		g_DrawArea = CreateWindow(
			WC_STATIC,
			NULL,
			WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
			rc.left, rc.top + 16, rc.right - 128, rc.bottom - 48,
			g_hMain,
			NULL,
			hInstance,
			NULL
		);

		g_AniTrack = CreateWindow(
			TRACKBAR_CLASS,
			NULL,
			WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
			rc.left, rc.bottom - 48, rc.right - 128, 16,
			g_hMain,
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
			(HMENU)IDM_STATUS,
			hInstance,
			NULL);

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

		/*tbb[7].iBitmap = 5;
		tbb[7].fsState = TBSTATE_ENABLED;
		tbb[7].fsStyle = TBSTYLE_BUTTON;
		tbb[7].idCommand = ID_UPLOAD;*/

		SendMessage(gHTool, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON), (LPARAM)&tbb);

		/* prepare the status bar */
		srand(timeGetTime());
		int statwidths[] = { 180, -1 };
		SendMessage(gHStatus, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);
		SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)"No Model!");
		SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[rand() % 6].c_str());

		//SetWindowPos( g_hMain, HWND_TOP, WinX,WinY, WinW, WinH, SWP_SHOWWINDOW );

		//GetWindowRect( g_hMain, &rc );
		int PrevW = WinW,
			PrevH = WinH;
		ShowWindow(g_hMain, SW_SHOW);

		/* enable OpenGL for the window */
		EnableOpenGL(g_DrawArea, &hDC, &hRC);

		//MoveWindow( g_hMain, 0,0, WinW,WinH, true );

		// There is a bug with ShowWindow, it sends a WM_SIZE without my permission
		SendMessage(g_hMain, WM_SIZE, 0, (LPARAM)MAKELPARAM(WinW, WinH));
		WinW = PrevW;
		WinH = PrevH;

		DWORD messageTime = timeGetTime();

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
				if (GetForegroundWindow() == g_hMain)
				{
					POINT ms;
					RECT rc;
					GetCursorPos(&ms);
					CurX = ms.x;
					CurY = ms.y;

					mouseE();
					keyboardE();

					/* OpenGL animation code goes here */

					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					// Camera
					GetClientRect(g_DrawArea, &rc);
					glViewport(0, 0, rc.right - rc.left, rc.bottom - rc.top);

					if (CameraView == VIEW_PERSPECTIVE)
					{
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluPerspective(45.0f, (float)rc.right / (float)rc.bottom, 1.0f, 1000.0f);

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						gluLookAt(cam.x, cam.z, cam.y, cam.xt, cam.zt, cam.yt, 0.0f, 1.0f, 0.0f);
					}
					else if (CameraView == VIEW_FIRSTPERSON)
					{
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluPerspective(60.0f, (float)rc.right / (float)rc.bottom, 0.1f, 1000.0f);

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						gluLookAt(0, 0, 0, 0, 0, 1, 0.0f, 1.0f, 0.0f);
					}
					else
					{
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						glOrtho(0, rc.right, rc.bottom, 0, 1000.0f, -1000.0f);

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();

						glTranslatef((rc.right / 2) + ortho.xt, (rc.bottom / 2) + ortho.yt, 0);

						glRotatef(180.0f, 0, 0, 1);

						glScalef((cam.dist * 2) - cam.dist, (cam.dist * 2) - cam.dist, (cam.dist * 2) - cam.dist);

						if (CameraView == VIEW_RIGHT)
							glRotatef(-90.0f, 0, 1, 0);
						if (CameraView == VIEW_LEFT)
							glRotatef(90.0f, 0, 1, 0);
						if (CameraView == VIEW_BACK)
							glRotatef(180.0f, 0, 1, 0);
						if (CameraView == VIEW_FRONT)
							glRotatef(0.0f, 0, 1, 0);
						if (CameraView == VIEW_TOP)
							glRotatef(-90.0f, 1, 0, 0);
						if (CameraView == VIEW_BOTTOM)
							glRotatef(90.0f, 1, 0, 0);


						//glScalef( 16,16,16 );
					}

					double mvmatrix[16], projmatrix[16];
					int viewport[4];
					glGetIntegerv(GL_VIEWPORT, viewport);
					glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
					glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

					// Draw the 3d model (mesh) ... duh
					RenderMesh();

					// Draw text overlay
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glOrtho(rc.left, rc.right, rc.bottom, rc.top, -1280, 1280);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();

					// -- Render a vertex in 2D space, mapped to the 3D location
					if (g->bShowAxis)
					{
						double _outX, _outY, _outZ;

						gluProject(5.5, 0, 0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
						DrawTextGL((float)_outX, (float)rc.bottom - _outY, 0, "X", 0xFF0000FF);
						gluProject(0, 5.5, 0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
						DrawTextGL((float)_outX, (float)rc.bottom - _outY, 0, "Y", 0xFF00FF00);
						gluProject(0, 0, 5.5, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
						DrawTextGL((float)_outX, (float)rc.bottom - _outY, 0, "Z", 0xFFFF0000);
					}

					switch (CameraView)
					{
					case VIEW_PERSPECTIVE: DrawTextGL(13, 13, 0, "Perspective", 0xFF9000FF); break;
					case VIEW_FIRSTPERSON: DrawTextGL(13, 13, 0, "First Person", 0xFF9000FF); break;
					case VIEW_LEFT: DrawTextGL(13, 13, 0, "Left", 0xFF9000FF); break;
					case VIEW_RIGHT: DrawTextGL(13, 13, 0, "Right", 0xFF9000FF); break;
					case VIEW_FRONT: DrawTextGL(13, 13, 0, "Front", 0xFF9000FF); break;
					case VIEW_BACK: DrawTextGL(13, 13, 0, "Back", 0xFF9000FF); break;
					case VIEW_TOP: DrawTextGL(13, 13, 0, "Top", 0xFF9000FF); break;
					case VIEW_BOTTOM: DrawTextGL(13, 13, 0, "Bottom", 0xFF9000FF); break;
					}

					// -- Draw focus rectangle
					if (GetFocus() == g_hMain)
					{
						glBegin(GL_LINE_LOOP);
						glColor4ub(255, 0, 0, 255);
						glVertex2f((float)rc.left + 2, (float)rc.top + 3);
						glVertex2f((float)rc.right - 3, (float)rc.top + 3);
						glVertex2f((float)rc.right - 3, (float)rc.bottom - 3);
						glVertex2f((float)rc.left + 3, (float)rc.bottom - 3);
						glEnd();
					}

					RealTime = timeGetTime();
					//RealTime/=4;
					TimeDt = RealTime - PrevTime;
					if (TimeDt < 0) TimeDt = 10;
					if (TimeDt > 10000) TimeDt = 10;
					if (TimeDt > 1000) TimeDt = 1000;
					PrevTime = RealTime;

					if (GetTickCount() - LastTick >= 1000)
					{
						LastTick = GetTickCount();
						FPS = Frames;
						Frames = 0;
					}
					Frames++;

					if (RealTime - messageTime >= 5000)
					{
						srand(RealTime);
						SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)g_HintStrings.at(rand() % g_HintStrings.size()).c_str());
						messageTime = RealTime;
					}

					// -- Draw the FPS
					std::stringstream ss;
					ss << "FPS: " << FPS;
					video->UI.DrawText(10, 10, ss.str(), 0xFF000);

					// Copy the buffer (Double Buffering)
					SwapBuffers(hDC);
					Sleep(1);
				}
				else
				{
					Sleep(100);
				}
			}
		}
	}
	catch (std::runtime_error& re) {
		std::cerr << "A Runtime Error has occured!\n\t" << re.what() << std::endl;
	}
	catch (std::exception& e) {
		// Some report
	}

	RECT rc;
	GetWindowRect(g_hMain, &rc);

	// -- Save the program settings to INI file
	IniFile ini;
	ini.SetKey("General", "GameDir", "C:\\Program Files (x86)\\Carnivores 2\\");
	ini.SetKey("General", "ProjectDir", "Projects\\");
	ini.SetKey("General", "OldFormatWarnings", true);

	ini.SetKey("Window", "Left", rc.left);
	ini.SetKey("Window", "Top", rc.top);
	ini.SetKey("Window", "Width", rc.right - rc.left);
	ini.SetKey("Window", "Height", rc.bottom - rc.top);

	ini.SetKey("RecentFiles", "File1", g->sRecentFile[0]);
	ini.SetKey("RecentFiles", "File2", g->sRecentFile[1]);
	ini.SetKey("RecentFiles", "File3", g->sRecentFile[2]);
	ini.SetKey("RecentFiles", "File4", g->sRecentFile[3]);
	ini.SetKey("RecentFiles", "File5", g->sRecentFile[4]);

	ini.SetKey("Renderer", "API", 24);
	ini.SetKey("Renderer", "ColorBufferBits", 24);
	ini.SetKey("Renderer", "DepthBufferBits", 16);
	ini.SetKey("Renderer", "EnvironmentMapping", g->sEnvironmentMap);
	ini.SetKey("Renderer", "SpecularMapping", g->sSpecularMap);

	ini.Save(g_IniPath);

	// Shutdown OpenGL
	if (video) delete video;

	// -- Destroy the window explicitly
	if (g_FileView) DestroyWindow(g_FileView);
	if (g_DrawArea) DestroyWindow(g_DrawArea);
	if (hwnd) DestroyWindow(hwnd);

	// --Unregister class
	UnregisterClass(wc.lpszClassName, wc.hInstance);

	return msg.wParam;
}
