#pragma once

#include "scene.h"
#include "sceneManager.h"

#include "..\cerbiosIniHelper.h"

class cerbiosIniEditorScene : public scene
{
public:
	cerbiosIniEditorScene(const char* iniPath, cerbiosVersion version);
	~cerbiosIniEditorScene();
	void update();
	void render();
	char* shortenString(const char* value);
private:
	char* mIniPath;
	cerbiosVersion mVersion;
	int mSelectedControl;
	int mMaxOptionCount;
	bool mNeedsSave;
	bool mHasFilePicker;
	bool mShowingFilePicker;
	bool mShowingInfo;
	bool mShowingConfirm;
	cerbiosConfig mConfig;
	cerbiosIniDoc mDoc;
	int* mActiveControls;
	int mActiveControlCount;
	char* mShortCdPath1;
	char* mShortCdPath2;
	char* mShortCdPath3;
	char* mShortDashPath1;
	char* mShortDashPath2;
	char* mShortDashPath3;
	char* mShortDashPath;
	char* mShortBootAnimPath;
	void rebuildActiveControls();
	void refreshShortPath(int controlId);
	char* getOptionInfo(int controlId);
	static void onPathClosingCallback(sceneResult result, void* context, scene* scene);
	static void onFrontLedClosingCallback(sceneResult result, void* context, scene* scene);
};
