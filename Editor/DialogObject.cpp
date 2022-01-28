#include "stdafx.h"
#include "C3Dit.h"

#include <iostream>
#include <sstream>
#include <string>



INT_PTR CALLBACK DlgObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		HWND hItem = nullptr;
		std::stringstream ss("");

		hItem = GetDlgItem(hwnd, IDC_OBJECT_DEFAULTLIGHT);
		SetWindowLong(hItem, GWL_STYLE, GetWindowLong(hItem, GWL_STYLE) | BS_OWNERDRAW);
		InvalidateRect(hItem, nullptr, false);
		UpdateWindow(hItem);

		SetDlgItemText(hwnd, IDC_NAME, g_Project.Model.Name.c_str());

		ss.str(""); ss.clear(); ss << g_Project.ObjInfo.Radius;
		SetDlgItemText(hwnd, IDC_OBJECT_RADIUS, ss.str().c_str());

		ss.str(""); ss.clear(); ss << g_Project.ObjInfo.YLo;
		SetDlgItemText(hwnd, IDC_OBJECT_YLOW, ss.str().c_str());

		ss.str(""); ss.clear(); ss << g_Project.ObjInfo.YHi;
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

		case IDC_OBJECT_DEFAULTLIGHT: {
			COLORREF custom_colors[16];
			CHOOSECOLOR cc;
			std::memset(&cc, 0, sizeof(cc));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hwnd;
			cc.lpCustColors = custom_colors;
			cc.rgbResult = 0xFFFFFF;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc)) {
				g_Project.ObjInfo.DefLight = cc.rgbResult;
				//RedrawWindow((HWND)lParam, nullptr, nullptr, );
				InvalidateRect((HWND)lParam, nullptr, false);
				UpdateWindow((HWND)lParam);
			}
		} break;
		}
		break;

	case WM_DRAWITEM: {
		COLORREF color = g_Project.ObjInfo.DefLight;
		int R = (color & 0xFF);
		int G = (color >> 8 & 0xFF);
		int B = (color >> 16 & 0xFF);
		std::stringstream ss;
		ss << std::hex << (color & 0xFF) << " ";
		ss << std::hex << ((color >> 8) & 0xFF) << " ";
		ss << std::hex << ((color >> 16) & 0xFF);
		DRAWITEMSTRUCT* pdis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
		std::string text = ss.str();
		SIZE size;

		GetTextExtentPoint32(pdis->hDC, text.c_str(), text.size(), &size);

		SetTextColor(pdis->hDC, RGB(255 - R, 255 - G, 255 - B));
		SetBkColor(pdis->hDC, color);

		//HPEN oldPen = SelectObject(pdis->hDC, (HPEN)CreatePen());
		HBRUSH oldBrush = (HBRUSH)SelectObject(pdis->hDC, CreateSolidBrush(color));
		Rectangle(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, pdis->rcItem.right, pdis->rcItem.bottom);
		SelectObject(pdis->hDC, oldBrush);

		TextOut(pdis->hDC,
			((pdis->rcItem.right - pdis->rcItem.left) - size.cx) / 2,
			((pdis->rcItem.bottom - pdis->rcItem.top) - size.cy) / 2,
			text.c_str(), text.size());

		DrawEdge(pdis->hDC, &pdis->rcItem, (pdis->itemState & ODS_SELECTED ? EDGE_SUNKEN : EDGE_RAISED), BF_RECT);
	} return true;

		/*case WM_ERASEBKGND: {
			HWND button = GetDlgItem(hwnd, IDC_OBJECT_DEFAULTLIGHT);
		} break;*/
	}

	return false;
}
