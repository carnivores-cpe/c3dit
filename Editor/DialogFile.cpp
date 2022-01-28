#include "stdafx.h"
#include "DialogFile.h"


CDialogFile::CDialogFile() :
	cFileNameSize(MAX_PATH),
	mFilterIndex(0u),
	mFilters(),
	mFileNameSize(cFileNameSize),
	mFileName(mFileNameSize, '\0')
{
	//this->SetFilter({ std::pair<std::tstring, std::tstring>(TEXT("A"), TEXT("B")) });
	
	return;
	// Example of setting the Filter
	this->SetFilter({ { TEXT("Text file"), TEXT("*.txt") }, { TEXT("Bitmap file"), TEXT("*.bmp") }, { TEXT("All image files"), TEXT("*.bmp;*.png;*.tga;*.gif") } });
}

CDialogFile::~CDialogFile() {

}

void CDialogFile::SetFilters(const std::vector<std::pair<std::tstring, std::tstring>>& filters) {
	size_t stFiltersSize = 0u;

	for (auto filter : filters) {
		stFiltersSize += filter.first.size() + 1;
		stFiltersSize += filter.second.size() + 1;
	}

	this->mFilters = std::make_unique<TCHAR[]>(stFiltersSize + 1);

	unsigned i = 0u;
	for (auto filter : filters) {
		std::copy(filter.first.begin(), filter.first.end(), this->mFilters.get() + i);
		i += filter.first.size();
		this->mFilters[i] = 0;
		++i;
		std::copy(filter.second.begin(), filter.second.end(), this->mFilters.get() + i);
		i += filter.second.size();
		this->mFilters[i] = 0;
		++i;
	}

	this->mFilters[i] = 0;
	++i;
}

bool CDialogFile::Open() {
	OPENFILENAME ofn;
	std::fill(&ofn, &ofn + sizeof(OPENFILENAME), 0);

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.hwndOwner = CEditorWindow::GetHandle();
	ofn.hInstance = CEditorWindow::GetInstance();//GetModuleHandle(nullptr);
	ofn.lpstrFilter = mFilters.get();
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = mFileName.data();
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = ext.c_str();

	return GetOpenFileNameW(&ofn);
}