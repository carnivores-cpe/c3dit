#include "stdafx.h"
#include "C3Dit.h"

#include <iostream>
#include <sstream>
#include <string>

INT_PTR CALLBACK DlgSoundProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		// -- Populate the sound effect combobox
		HWND snd_list = GetDlgItem(hwnd, IDC_SELECTION);
		ComboBox_ResetContent(snd_list);

		for (auto sound : g_Project.SoundEffects)
			ComboBox_AddString(snd_list, sound.Name.c_str());

		ComboBox_SetCurSel(snd_list, g_Project.sound_current);

		if (g_Project.sound_current >= 0) {
			// TODO: handle std::out_of_range exception here
			CSound& sound = g_Project.SoundEffects.at(g_Project.sound_current);

			SetDlgItemText(hwnd, IDC_NAME, sound.Name.c_str());
			SetDlgItemInt(hwnd, IDC_DATA_LENGTH, sound.Length, false); // Frame Count
		}
		else {
			EnableWindow(GetDlgItem(hwnd, IDC_NAME), false);
			EnableWindow(GetDlgItem(hwnd, IDC_DATA_LENGTH), false);
			EnableWindow(GetDlgItem(hwnd, IDC_OPEN), false);
			EnableWindow(GetDlgItem(hwnd, IDC_SAVE), false);
			EnableWindow(GetDlgItem(hwnd, IDC_PLAY), false);
			EnableWindow(GetDlgItem(hwnd, IDC_DELETE), false);
		}
	}
	break;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL: EndDialog(hwnd, IDCANCEL); return true;
			//case IDOK: EndDialog(hwnd, IDOK); return true;

		case IDC_OPEN: { /*importSound(ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION)));*/ } break;
		case IDC_SAVE: { /*exportSound(ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SELECTION)));*/ } break;

		case IDC_NEW: {
			CSound sound;
			sound.Name = "New sound";
			sound.Length = 0u;
			g_Project.SoundEffects.push_back(sound);
		} break;

		case IDC_INSERT: break;
		case IDC_DELETE: break;

		case IDC_PLAY: { PlayWave(g_Project.SoundEffects.at(g_Project.sound_current)); } break;
		}
		break;
	}

	return false;
}
