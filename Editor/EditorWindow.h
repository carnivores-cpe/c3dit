#pragma once

#include "IInterface.h"
#include "Storage.h"
#include <memory>

bool CreateIEditorWindow(std::weak_ptr<IEditorWindow> pInstance);


class IEditorWindow : public IInterface {
public:
	struct SWinData;

	~IEditorWindow();

	// Non-copyable
	IEditorWindow(const IEditorWindow&) = delete;
	IEditorWindow& operator=(const IEditorWindow&) = delete;

	IEditorWindow(IEditorWindow&&);
	IEditorWindow& operator=(IEditorWindow&&);

	const Storage<SWinData>& GetStorage();

private:
	Storage<SWinData> mStorage;

	IEditorWindow();

	friend bool CreateEditorWindow(std::weak_ptr<IEditorWindow> pInstance);
};

