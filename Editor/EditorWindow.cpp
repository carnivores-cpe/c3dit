#include "stdafx.h"
#include "EditorWindow.h"

#include <Windows.h>
#include <stdexcept>

#include "resource.h"


HWND hEditorWindow;
WNDCLASSEX wcEditorWindowClass;
HINSTANCE hEditorInstance;


LRESULT CALLBACK EditorWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void CreateEditorWindow() {
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	WNDCLASSEX &wc = wcEditorWindowClass;

	// MSDN: https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-initnetworkaddresscontrol
	InitNetworkAddressControl();

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = TEXT("CARNCAREDITOR_WCLASS");
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.lpfnWndProc = EditorWindowProc;

	if (!RegisterClassEx(&wcEditorWindowClass)) {
		throw std::runtime_error("RegisterClassEx");
	}

	hEditorWindow = CreateWindowEx(WS_EX_ACCEPTFILES, wc.lpszClassName, TEXT("Carnivores Character Editor"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 960, HWND_DESKTOP, (HMENU)nullptr, hInstance, this);
	if (!hEditorWindow) throw std::runtime_error("CreateWindowEx");

	// TODO: Create instance of CUserInterface to handle creation of window elements
}

void DestroyEditorWindow() {
	WNDCLASSEX& wc = wcEditorWindowClass;

	DestroyWindow(hEditorWindow);

	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

HWND GetEditorWindowNativeHandle();

LRESULT CALLBACK EditorWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	CEditorWindow* pEditorWindow = reinterpret_cast<CEditorWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	auto rStorage = pEditorWindow->GetStorage();

	switch (uMsg) {
	case WM_DROPFILES: {
		/* Drag 'n' Drop files onto the application window */
		HDROP hDrop = reinterpret_cast<HDROP>(wParam);

		POINT sDropFilePoint{};
		auto iDropFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
		TCHAR szDropFileName[MAX_PATH];

		DragQueryPoint(hDrop, &sDropFilePoint);

		for (int i = 0; i < iDropFileCount; i++) {
			std::fill(&szDropFileName[0], &szDropFileName[MAX_PATH], 0);
			auto iDropFileNameSize = DragQueryFile(hDrop, i, szDropFileName, MAX_PATH);

			if (iDropFileNameSize != 0) {
				// Operate on this file.
			}
		}

		DragFinish(hDrop);
	} return 0;

	case WM_NCCREATE:
		return 0;

	case WM_CREATE:
		return 0;

	case WM_DESTROY:
		return 0;

	case WM_CLOSE: {
		// TODO: Check if there are unsaved changes
		switch (MessageBox(hWnd, TEXT("Are you sure you want to quit without saving the project?"), TEXT("Unsaved changes..."), MB_ICONQUESTION | MB_YESNOCANCEL)) {
		default: break;
		case IDYES: /*Save, then quit*/
			DestroyWindow(hWnd);
			return 0;
		case IDNO: /*Quit, but don't save*/
			DestroyWindow(hWnd);
			return 0;
		case IDCANCEL: /*Don't quit*/
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	} return 0;

	case WM_COMMAND:
		return 0;

	case WM_NOTIFY:
		return 0;
		
	case WM_SIZE:
		return 0;

	default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
