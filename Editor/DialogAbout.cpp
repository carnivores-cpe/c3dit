#include "stdafx.h"
#include "C3Dit.h"

#include <iostream>
#include <sstream>
#include <string>


INT_PTR CALLBACK DlgAboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		std::stringstream ss;
		HWND hcontrol = GetDlgItem(hwnd, IDC_ABOUT_TEXT);
		ss << "Carnivores Character 3D Editor\r\nAKA: C3Dit\r\nVersion: " << VERSION_STR;
		ss << "\r\nBy Rexhunter99/Cool Breeze\r\nContact: rexhunter99@gmail.com";
		SetWindowText(hcontrol, ss.str().c_str());
		hcontrol = GetDlgItem(hwnd, IDC_ABOUT_IMAGE);
		SetWindowLong(hcontrol, GWL_STYLE, GetWindowLong(hcontrol, GWL_STYLE) | SS_BITMAP); // Visual Studio editor doesnt have a Bitmap option
		SendMessage(hcontrol, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)MAKEINTRESOURCE(IDB_REXHU));
	}
	break;

	case WM_CLOSE: EndDialog(hwnd, IDOK); return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK: EndDialog(hwnd, IDOK); return true;
		}
		break;
	}

	return false;
}
