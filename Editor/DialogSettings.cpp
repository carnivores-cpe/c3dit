#include "stdafx.h"
#include "C3Dit.h"

#include <iostream>
#include <sstream>
#include <string>


INT_PTR CALLBACK DlgSettingsProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
	}
	break;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL: { EndDialog(hwnd, IDCANCEL); } return true;
		case IDOK: {
			EndDialog(hwnd, IDOK);
		} return true;
		}
		break;
	}

	return false;
}
