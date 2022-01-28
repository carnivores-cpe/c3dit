#include "stdafx.h"
#include "C3Dit.h"

#include <iostream>
#include <sstream>
#include <string>

INT_PTR CALLBACK DlgAnimationProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		RECT rc = { 0,0,0,0 };
		HWND ani_list = GetDlgItem(hwnd, IDC_SELECTION);
		GetWindowRect(ani_list, &rc);
		SetWindowPos(ani_list, 0, 0, 0, rc.right - rc.left, (rc.bottom - rc.top) * 3, SWP_NOMOVE | SWP_NOZORDER);

		ComboBox_ResetContent(ani_list);

		for (auto anim : g_Project.Animations)
			ComboBox_AddString(ani_list, anim.Name.c_str());

		if (!g_Project.Animations.empty())
			ComboBox_SetCurSel(ani_list, g_Project.animation_current);

		if (g_Project.animation_current > -1) {
			// TODO: handle std::out_of_range exception here
			CVertexAnimation& anim = g_Project.Animations.at(g_Project.animation_current);

			SetDlgItemText(hwnd, IDC_NAME, anim.Name.c_str());
			SetDlgItemInt(hwnd, IDC_KPS, anim.KPS, true);
			//SetDlgItemInt(hwnd, IDC_TIME, 0u, FALSE); // Time length of animation
			SetDlgItemInt(hwnd, IDC_DATA_LENGTH, anim.FrameCount, false); // Frame Count

			// -- Populate the sound effect combobox
			HWND snd_list = GetDlgItem(hwnd, IDC_SOUND);
			GetWindowRect(snd_list, &rc);
			SetWindowPos(ani_list, 0, 0, 0, rc.right - rc.left, (rc.bottom - rc.top) * 3, SWP_NOMOVE | SWP_NOZORDER);
			ComboBox_ResetContent(snd_list);

			for (auto sound : g_Project.SoundEffects)
				ComboBox_AddString(snd_list, sound.Name.c_str());

			ComboBox_SetCurSel(snd_list, anim.SoundFX);
		}
		else {
			EnableWindow(GetDlgItem(hwnd, IDOK), false);
			EnableWindow(GetDlgItem(hwnd, IDC_SAVE), false);
			EnableWindow(GetDlgItem(hwnd, IDC_OPEN), false);
			EnableWindow(GetDlgItem(hwnd, IDC_DELETE), false);
			EnableWindow(GetDlgItem(hwnd, IDC_NAME), false);
			EnableWindow(GetDlgItem(hwnd, IDC_KPS), false);
			//EnableWindow(GetDlgItem(hwnd, IDC_TIME), false);
			EnableWindow(GetDlgItem(hwnd, IDC_DATA_LENGTH), false);
			EnableWindow(GetDlgItem(hwnd, IDC_SOUND), false);
		}
	}
	break;

	case WM_CLOSE: {
		g_Project.animation_current = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION));
		EndDialog(hwnd, IDCANCEL);
	} return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL: SendMessage(hwnd, WM_CLOSE, 0, 0); break;//EndDialog(hwnd, IDCANCEL); return true;
		case IDOK: { // Apply

			int32_t iItem = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION)) - 1;

			if (iItem < 0) break;

			CVertexAnimation& anim = g_Project.Animations.at(iItem);

			char anim_name[256]; GetDlgItemText(hwnd, IDC_NAME, anim_name, 256);
			anim.Name = anim_name;
			anim.KPS = GetDlgItemInt(hwnd, IDC_KPS, nullptr, true);
			//time = GetDlgItemInt(hwnd, IDC_TIME, nullptr, false); // Time length of animation
			//anim.FrameCount = GetDlgItemInt(hwnd, IDC_DATA_LENGTH, anim.FrameCount, false); // Frame Count

			anim.SoundFX = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SOUND));
			if (anim.SoundFX < -1) anim.SoundFX = -1;

		} return true;

		case IDC_NEW: {
			std::stringstream ss;
			ss << "Animation " << g_Project.Animations.size();

			g_Project.animation_current = g_Project.Animations.size();

			auto anim = g_Project.Animations.insert(g_Project.Animations.end(), CVertexAnimation());
			anim->Name = ss.str();
			anim->KPS = -1;
			anim->SoundFX = -1;

			ComboBox_AddString(GetDlgItem(hwnd, IDC_SELECTION), anim->Name.c_str());

			if (!g_Project.Animations.empty())
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SELECTION), g_Project.animation_current);
		} break;

		case IDC_INSERT: {
			int iIndex = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION));
			auto itIndex = g_Project.Animations.begin();

			if (iIndex >= 0) {
				itIndex += iIndex;
			}
			else {
				itIndex = g_Project.Animations.end();
			}

			std::stringstream ss;
			ss << "Insert Animation " << g_Project.Animations.size();
			std::cout << ss.str() << std::endl;

			auto anim = g_Project.Animations.insert(itIndex, CVertexAnimation());
			anim->Name = ss.str();
			anim->KPS = -1;
			anim->SoundFX = -1;

			g_Project.animation_current = iIndex;

			ComboBox_InsertString(GetDlgItem(hwnd, IDC_SELECTION), iIndex, anim->Name.c_str());

			if (!g_Project.Animations.empty())
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SELECTION), iIndex);
		} break;

		case IDC_DELETE: {
			int32_t iIndex = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION)) - 1;

			if (iIndex < 0) break;

			g_Project.Animations.erase(g_Project.Animations.begin() + iIndex);

			ComboBox_DeleteString(GetDlgItem(hwnd, IDC_SELECTION), iIndex);

			if (!g_Project.Animations.empty())
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SELECTION), iIndex - 1); // Select previous index
			else
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SELECTION), -1);
		} break;

		////////////////////////////////////////////////////////////////////////
		// Save
		case IDC_SAVE:
		{
			int iIndex = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION));

			if (iIndex > -1) {
				// TODO: handle std::out_of_range exception here
				CVertexAnimation& anim = g_Project.Animations.at(iIndex);

				// Save the animation to disk
				anim.Save(SaveFileDlg(hwnd, g_AnimationFileFormats, "vtl"));
			}
			else {
				MessageBox(hwnd, "You must select a valid animation slot!", "Animation ID -1", MB_ICONEXCLAMATION);
			}
		}
		break;

		////////////////////////////////////////////////////////////////////////
		// Open
		case IDC_OPEN:
		{
			int iIndex = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION));

			if (iIndex > -1) {
				// TODO: handle std::out_of_range exception here
				CVertexAnimation& anim = g_Project.Animations.at(iIndex);

				anim.Load(SaveFileDlg(hwnd, g_AnimationFileFormats, "vtl"));

				// Send change notification to update the dialog:
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SELECTION), iIndex);
			}
			else {
				MessageBox(hwnd, "You must select a valid animation slot!", "Animation ID -1", MB_ICONEXCLAMATION);
			}

			//int64_t time = (g_Animations[g_CurrentAnimation].FrameCount * 1000) / g_Animations[g_CurrentAnimation].KPS;
		} break;
		}
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			int32_t iIndex = ComboBox_GetCurSel((HWND)lParam);

			if (iIndex > -1)
			{
				CVertexAnimation& anim = g_Project.Animations.at(iIndex);

				SetDlgItemText(hwnd, IDC_NAME, anim.Name.c_str());
				SetDlgItemInt(hwnd, IDC_KPS, anim.KPS, true);
				//SetDlgItemInt(hwnd, IDC_TIME, 0u, FALSE); // Time length of animation
				SetDlgItemInt(hwnd, IDC_DATA_LENGTH, anim.FrameCount, false); // Frame Count

				// -- Populate the sound effect combobox
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SOUND), anim.SoundFX + 1);

				EnableWindow(GetDlgItem(hwnd, IDOK), true);
				EnableWindow(GetDlgItem(hwnd, IDC_SAVE), true);
				EnableWindow(GetDlgItem(hwnd, IDC_OPEN), true);
				EnableWindow(GetDlgItem(hwnd, IDC_DELETE), true);
				EnableWindow(GetDlgItem(hwnd, IDC_NAME), true);
				EnableWindow(GetDlgItem(hwnd, IDC_KPS), true);
				//EnableWindow(GetDlgItem(hwnd, IDC_TIME), true);
				EnableWindow(GetDlgItem(hwnd, IDC_DATA_LENGTH), false);
				EnableWindow(GetDlgItem(hwnd, IDC_SOUND), true);
			}
			else {
				EnableWindow(GetDlgItem(hwnd, IDOK), false);
				EnableWindow(GetDlgItem(hwnd, IDC_SAVE), false);
				EnableWindow(GetDlgItem(hwnd, IDC_OPEN), false);
				EnableWindow(GetDlgItem(hwnd, IDC_DELETE), false);
				EnableWindow(GetDlgItem(hwnd, IDC_NAME), false);
				EnableWindow(GetDlgItem(hwnd, IDC_KPS), false);
				//EnableWindow(GetDlgItem(hwnd, IDC_TIME), false);
				EnableWindow(GetDlgItem(hwnd, IDC_DATA_LENGTH), false);
				EnableWindow(GetDlgItem(hwnd, IDC_SOUND), false);
			}
		} break;
		}
		break;
	}

	return false;
}
