#pragma once
#ifndef DIALOGFILE_H_INCLUDED
#define DIALOGFILE_H_INCLUDED

#include <vector>
#include <string>
#include <memory>

#ifndef tstring
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif //_UNICODE
#endif //tstring

#ifndef DFilterList
#define DFilterList std::vector<std::pair<std::tstring, std::tstring>>
#endif //DFilterList

class CDialogFile {
private:
	const unsigned cFileNameSize;

	unsigned mFilterIndex;
	std::unique_ptr<TCHAR[]> mFilters;
	unsigned mFileNameSize;
	std::tstring mFileName;

public:

	CDialogFile();
	CDialogFile(std::tstring &filename, const std::tstring &title, const DFilterList &filters);
	~CDialogFile();

	std::tstring GetFileName();
	std::tstring GetFilePath();
	std::tstring GetFilePathName();

	void SetTitle(const std::tstring& title);
	void SetFilters(const DFilterList &filters);

	bool Open();
	bool Save();
};

#endif // DIALOGFILE_H_INCLUDED