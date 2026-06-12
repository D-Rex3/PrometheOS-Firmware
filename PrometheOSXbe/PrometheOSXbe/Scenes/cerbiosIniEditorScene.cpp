#include "cerbiosIniEditorScene.h"
#include "sceneManager.h"
#include "keyboardScene.h"
#include "frontLedSelectorScene.h"
#include "hexSelectorScene.h"
#include "filePickerScene.h"

#include "..\context.h"
#include "..\drawing.h"
#include "..\component.h"
#include "..\inputManager.h"
#include "..\stringUtility.h"
#include "..\xboxInternals.h"
#include "..\theme.h"

#include <stddef.h>

// ============================================================================
// Control descriptor table
// ============================================================================

enum controlKind
{
	KIND_BOOL,
	KIND_DRIVE_SETUP,
	KIND_UDMA,
	KIND_FAN,
	KIND_FFILTER,
	KIND_LED,
	KIND_PATH,
	KIND_HEX24,
	KIND_HEX_COMBO,
	KIND_HEX_BYTE,
	KIND_BUS_BIN,
	KIND_LCDPROTO,
	KIND_HDD_BIN,
	KIND_PARTITION,
	KIND_IGRPORT,
	KIND_ADV_UNLOCK
};

// Stable control ids; index into kInfoTexts.
enum
{
	CID_DriveSetup,
	CID_AVCheck,
	CID_Debug,
	CID_Force480p,
	CID_ForceVGA,
	CID_ForceFFilter,
	CID_UdmaModeMaster,
	CID_UdmaModeSlave,
	CID_FanSpeed,
	CID_OverrideFan,
	CID_FrontLed,
	CID_BlockDashUpdate,
	CID_ResetOnEject,
	CID_RtcEnable,
	CID_CdPath1,
	CID_CdPath2,
	CID_CdPath3,
	CID_DashPath1,
	CID_DashPath2,
	CID_DashPath3,
	CID_DashPath,
	CID_BootAnimPath,
	CID_XonlineDashRedir,
	CID_AdvCPUSupport,
	CID_DisableLimitMem,
	CID_ApplyTitlePatches,
	CID_ReadOnlyC,
	CID_InAppLCDEnable,
	CID_LCDBus,
	CID_LCDI2CAddr,
	CID_LCDProto,
	CID_TUDATARedir,
	CID_TUDATARedirHDD,
	CID_TUDATARedirPart,
	CID_EnableScreenshots,
	CID_ScreenshotHDD,
	CID_ScreenshotPart,
	CID_IGRMasterPort,
	CID_IGRDash,
	CID_IGRGame,
	CID_IGRFull,
	CID_IGRCycle,
	CID_IGRShutdown,
	CID_IGRScreen,
	CID_AdvUnlock,
	CID_Overclocking,
	CID_CPUMPLLCoeff,
	CID_NVPLLCoeff,
	CID_COUNT
};

#define VL 0x01u  // Legacy (2.4.2)
#define VC 0x02u  // Current (3.0.0+)
#define VB 0x03u  // Both

struct controlDescriptor
{
	int id;
	unsigned char versions;
	bool advancedOnly;
	const char* label;
	int kind;
	size_t offset;  // offsetof in cerbiosConfig; ignored for KIND_ADV_UNLOCK
};

static const controlDescriptor kControls[] = {
	{ CID_DriveSetup,        VB, false, "Drive Setup:",         KIND_DRIVE_SETUP, offsetof(cerbiosConfig, DriveSetup) },
	{ CID_AVCheck,           VB, false, "AV Check:",            KIND_BOOL,        offsetof(cerbiosConfig, AVCheck) },
	{ CID_Debug,             VB, false, "Debug:",               KIND_BOOL,        offsetof(cerbiosConfig, Debug) },
	{ CID_Force480p,         VB, false, "Force 480p:",          KIND_BOOL,        offsetof(cerbiosConfig, Force480p) },
	{ CID_ForceVGA,          VB, false, "Force VGA:",           KIND_BOOL,        offsetof(cerbiosConfig, ForceVGA) },
	{ CID_ForceFFilter,      VC, false, "Force F-Filter:",      KIND_FFILTER,     offsetof(cerbiosConfig, ForceFFilter) },
	{ CID_UdmaModeMaster,    VB, false, "Udma Master:",         KIND_UDMA,        offsetof(cerbiosConfig, UdmaModeMaster) },
	{ CID_UdmaModeSlave,     VB, false, "Udma Slave:",          KIND_UDMA,        offsetof(cerbiosConfig, UdmaModeSlave) },
	{ CID_FanSpeed,          VB, false, "Fan Speed:",           KIND_FAN,         offsetof(cerbiosConfig, FanSpeed) },
	{ CID_OverrideFan,       VC, false, "Override Fan:",        KIND_BOOL,        offsetof(cerbiosConfig, OverrideFan) },
	{ CID_FrontLed,          VB, false, "Front Led:",           KIND_LED,         offsetof(cerbiosConfig, FrontLed) },
	{ CID_BlockDashUpdate,   VB, false, "Block Dash Update:",   KIND_BOOL,        offsetof(cerbiosConfig, BlockDashUpdate) },
	{ CID_ResetOnEject,      VB, false, "Reset On Eject:",      KIND_BOOL,        offsetof(cerbiosConfig, ResetOnEject) },
	{ CID_RtcEnable,         VB, false, "Rtc Enable:",          KIND_BOOL,        offsetof(cerbiosConfig, RtcEnable) },
	{ CID_CdPath1,           VL, false, "CD Path 1:",           KIND_PATH,        offsetof(cerbiosConfig, CdPath1) },
	{ CID_CdPath2,           VL, false, "CD Path 2:",           KIND_PATH,        offsetof(cerbiosConfig, CdPath2) },
	{ CID_CdPath3,           VL, false, "CD Path 3:",           KIND_PATH,        offsetof(cerbiosConfig, CdPath3) },
	{ CID_DashPath1,         VL, false, "Dash Path 1:",         KIND_PATH,        offsetof(cerbiosConfig, DashPath1) },
	{ CID_DashPath2,         VL, false, "Dash Path 2:",         KIND_PATH,        offsetof(cerbiosConfig, DashPath2) },
	{ CID_DashPath3,         VL, false, "Dash Path 3:",         KIND_PATH,        offsetof(cerbiosConfig, DashPath3) },
	{ CID_DashPath,          VC, false, "Dash Path:",           KIND_PATH,        offsetof(cerbiosConfig, DashPath) },
	{ CID_BootAnimPath,      VB, false, "Boot Anim Path:",      KIND_PATH,        offsetof(cerbiosConfig, BootAnimPath) },
	{ CID_XonlineDashRedir,  VC, false, "Xonline Dash Redir:",  KIND_BOOL,        offsetof(cerbiosConfig, XonlineDashRedir) },
	{ CID_AdvCPUSupport,     VC, false, "Adv CPU Support:",     KIND_BOOL,        offsetof(cerbiosConfig, AdvCPUSupport) },
	{ CID_DisableLimitMem,   VC, false, "Disable Limit Mem:",   KIND_BOOL,        offsetof(cerbiosConfig, DisableLimitMem) },
	{ CID_ApplyTitlePatches, VC, false, "Apply Title Patches:", KIND_BOOL,        offsetof(cerbiosConfig, ApplyTitlePatches) },
	{ CID_ReadOnlyC,         VC, false, "Read-Only C:",         KIND_BOOL,        offsetof(cerbiosConfig, ReadOnlyC) },
	{ CID_InAppLCDEnable,    VC, false, "In-App LCD:",          KIND_BOOL,        offsetof(cerbiosConfig, InAppLCDEnable) },
	{ CID_LCDBus,            VC, false, "LCD Bus:",             KIND_BUS_BIN,     offsetof(cerbiosConfig, LCDBus) },
	{ CID_LCDI2CAddr,        VC, false, "LCD I2C Addr:",        KIND_HEX_BYTE,    offsetof(cerbiosConfig, LCDI2CAddr) },
	{ CID_LCDProto,          VC, false, "LCD Protocol:",        KIND_LCDPROTO,    offsetof(cerbiosConfig, LCDProto) },
	{ CID_TUDATARedir,       VC, false, "TUDATA Redirect:",     KIND_BOOL,        offsetof(cerbiosConfig, TUDATARedir) },
	{ CID_TUDATARedirHDD,    VC, false, "TUDATA Redir HDD:",    KIND_HDD_BIN,     offsetof(cerbiosConfig, TUDATARedirHDD) },
	{ CID_TUDATARedirPart,   VC, false, "TUDATA Redir Part:",   KIND_PARTITION,   offsetof(cerbiosConfig, TUDATARedirPart) },
	{ CID_EnableScreenshots, VC, false, "Enable Screenshots:",  KIND_BOOL,        offsetof(cerbiosConfig, EnableScreenshots) },
	{ CID_ScreenshotHDD,     VC, false, "Screenshot HDD:",      KIND_HDD_BIN,     offsetof(cerbiosConfig, ScreenshotHDD) },
	{ CID_ScreenshotPart,    VC, false, "Screenshot Part:",     KIND_PARTITION,   offsetof(cerbiosConfig, ScreenshotPart) },
	{ CID_IGRMasterPort,     VC, false, "IGR Master Port:",     KIND_IGRPORT,     offsetof(cerbiosConfig, IGRMasterPort) },
	{ CID_IGRDash,           VC, false, "IGR Dash:",            KIND_HEX_COMBO,   offsetof(cerbiosConfig, IGRDash) },
	{ CID_IGRGame,           VC, false, "IGR Game:",            KIND_HEX_COMBO,   offsetof(cerbiosConfig, IGRGame) },
	{ CID_IGRFull,           VC, false, "IGR Full:",            KIND_HEX_COMBO,   offsetof(cerbiosConfig, IGRFull) },
	{ CID_IGRCycle,          VC, false, "IGR Cycle:",           KIND_HEX_COMBO,   offsetof(cerbiosConfig, IGRCycle) },
	{ CID_IGRShutdown,       VC, false, "IGR Shutdown:",        KIND_HEX_COMBO,   offsetof(cerbiosConfig, IGRShutdown) },
	{ CID_IGRScreen,         VC, false, "IGR Screenshot:",      KIND_HEX_COMBO,   offsetof(cerbiosConfig, IGRScreen) },
	{ CID_AdvUnlock,         VC, false, "Unlock Advanced (OC):",KIND_ADV_UNLOCK,  0 },
	{ CID_Overclocking,      VC, true,  "Overclocking:",        KIND_BOOL,        offsetof(cerbiosConfig, Overclocking) },
	{ CID_CPUMPLLCoeff,      VC, true,  "CPU Overclock:",       KIND_HEX24,       offsetof(cerbiosConfig, CPUMPLLCoeff) },
	{ CID_NVPLLCoeff,        VC, true,  "GPU Overclock:",       KIND_HEX24,       offsetof(cerbiosConfig, NVPLLCoeff) },
};

static const int kControlsCount = (int)(sizeof(kControls) / sizeof(kControls[0]));

// Find descriptor by control id (linear scan; small N).
static const controlDescriptor* findById(int id)
{
	for (int i = 0; i < kControlsCount; i++)
	{
		if (kControls[i].id == id) return &kControls[i];
	}
	return NULL;
}

// ============================================================================
// Render value strings
// ============================================================================

static const char* kBools[] = { "False", "True" };
static const char* kDriveModes[] = { "HDD & DVD", "HDD & No DVD (Legacy)", "HDD & No DVD (Modern)", "Dual HDD" };
static const char* kUdmaModes[] = { "Auto (Startech Adapter)", "Auto (Generic Adapter)", "UDMA 2 (Default / Stock)", "UDMA 3 (Ultra DMA 80-Conductor)", "UDMA 4 (Ultra DMA 80-Conductor)", "UDMA 5 (Ultra DMA 80-Conductor)", "UDMA 6 (Experimental)" };
static const char* kFanSpeeds[] = { "Auto", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%" };
static const char* kFFilter[] = { "Off", "Level 1", "Level 2", "Level 3", "Level 4", "Level 5", "System/Game Default" };
static const char* kBusBin[] = { "SMBus", "X3LCD" };
static const char* kLcdProto[] = { "HD44780 / X3LCD", "US2066 (NHD-0420CW)" };
static const char* kHddBin[] = { "Master", "Slave" };
static const char* kIgrPort[] = { "ALL", "Port 1", "Port 2", "Port 3", "Port 4" };

static const char* partitionLabel(unsigned char value)
{
	if (value == 1) return "E:";
	if (value == 6) return "F:";
	if (value == 7) return "G:";
	return "(unknown)";
}

// ============================================================================
// Field accessors (using offset)
// ============================================================================

static unsigned char* ucPtr(cerbiosConfig* cfg, size_t off) { return (unsigned char*)((char*)cfg + off); }
static uint16_t* u16Ptr(cerbiosConfig* cfg, size_t off) { return (uint16_t*)((char*)cfg + off); }
static uint32_t* u32Ptr(cerbiosConfig* cfg, size_t off) { return (uint32_t*)((char*)cfg + off); }
static char* charPtr(cerbiosConfig* cfg, size_t off) { return (char*)((char*)cfg + off); }

// ============================================================================
// Callbacks
// ============================================================================

void cerbiosIniEditorScene::onPathClosingCallback(sceneResult result, void* context, scene* scene)
{
	cerbiosIniEditorScene* self = (cerbiosIniEditorScene*)context;
	if (self->mShowingFilePicker == true)
	{
		filePickerScene* closingScene = (filePickerScene*)scene;
		const char* path = closingScene->getPath();
		if (path != NULL)
		{
			int controlId = kControls[self->mActiveControls[self->mSelectedControl]].id;
			const controlDescriptor* desc = findById(controlId);
			if (desc != NULL)
			{
				char* dest = charPtr(&self->mConfig, desc->offset);
				strncpy(dest, path, 99);
				dest[99] = 0;
				self->refreshShortPath(controlId);
				self->mNeedsSave = true;
			}
		}
		self->mShowingFilePicker = false;
		return;
	}

	keyboardScene* closingScene = (keyboardScene*)scene;
	const char* text = closingScene->getText();
	int controlId = kControls[self->mActiveControls[self->mSelectedControl]].id;
	const controlDescriptor* desc = findById(controlId);
	if (desc != NULL)
	{
		char* dest = charPtr(&self->mConfig, desc->offset);
		strncpy(dest, text, 99);
		dest[99] = 0;
		self->refreshShortPath(controlId);
		self->mNeedsSave = true;
	}
}

void cerbiosIniEditorScene::onFrontLedClosingCallback(sceneResult result, void* context, scene* scene)
{
	cerbiosIniEditorScene* self = (cerbiosIniEditorScene*)context;
	frontLedSelectorScene* closingScene = (frontLedSelectorScene*)scene;
	char* ledSequence = closingScene->getLedSequence();
	self->mConfig.FrontLed[0] = ledSequence[0];
	self->mConfig.FrontLed[1] = ledSequence[1];
	self->mConfig.FrontLed[2] = ledSequence[2];
	self->mConfig.FrontLed[3] = ledSequence[3];
	free(ledSequence);
	self->mNeedsSave = true;
}

void cerbiosIniEditorScene::onHexClosingCallback(sceneResult result, void* context, scene* scene)
{
	cerbiosIniEditorScene* self = (cerbiosIniEditorScene*)context;
	hexSelectorScene* closingScene = (hexSelectorScene*)scene;
	uint32_t value = closingScene->getValue();
	int controlId = kControls[self->mActiveControls[self->mSelectedControl]].id;
	const controlDescriptor* desc = findById(controlId);
	if (desc == NULL) return;
	if (desc->kind == KIND_HEX24)
	{
		*u32Ptr(&self->mConfig, desc->offset) = value & 0x00FFFFFF;
	}
	else if (desc->kind == KIND_HEX_COMBO)
	{
		*u16Ptr(&self->mConfig, desc->offset) = (uint16_t)value;
	}
	else if (desc->kind == KIND_HEX_BYTE)
	{
		*ucPtr(&self->mConfig, desc->offset) = (unsigned char)value;
	}
	self->mNeedsSave = true;
}

// ============================================================================
// Constructor / destructor
// ============================================================================

cerbiosIniEditorScene::cerbiosIniEditorScene(const char* iniPath, cerbiosVersion version)
{
	mIniPath = strdup(iniPath);
	mVersion = version;
	mConfig = cerbiosIniHelper::loadConfig(mIniPath, mVersion);
	mShortCdPath1 = shortenString(mConfig.CdPath1);
	mShortCdPath2 = shortenString(mConfig.CdPath2);
	mShortCdPath3 = shortenString(mConfig.CdPath3);
	mShortDashPath1 = shortenString(mConfig.DashPath1);
	mShortDashPath2 = shortenString(mConfig.DashPath2);
	mShortDashPath3 = shortenString(mConfig.DashPath3);
	mShortDashPath = shortenString(mConfig.DashPath);
	mShortBootAnimPath = shortenString(mConfig.BootAnimPath);
	mSelectedControl = 0;
	mHasFilePicker = false;
	mNeedsSave = false;
	mShowingFilePicker = false;
	mShowingInfo = false;
	mAdvancedUnlocked = false;
	mActiveControls = NULL;
	mActiveControlCount = 0;
	rebuildActiveControls();
}

cerbiosIniEditorScene::~cerbiosIniEditorScene()
{
	free(mIniPath);
	free(mActiveControls);
	free(mShortCdPath1);
	free(mShortCdPath2);
	free(mShortCdPath3);
	free(mShortDashPath1);
	free(mShortDashPath2);
	free(mShortDashPath3);
	free(mShortDashPath);
	free(mShortBootAnimPath);
}

// ============================================================================
// Active-controls list management
// ============================================================================

void cerbiosIniEditorScene::rebuildActiveControls()
{
	free(mActiveControls);
	mActiveControls = (int*)malloc(sizeof(int) * kControlsCount);
	mActiveControlCount = 0;

	unsigned char versionMask = (mVersion == CerbiosVersionCurrent) ? VC : VL;

	for (int i = 0; i < kControlsCount; i++)
	{
		const controlDescriptor* desc = &kControls[i];
		if ((desc->versions & versionMask) == 0) continue;
		if (desc->advancedOnly && !mAdvancedUnlocked) continue;
		mActiveControls[mActiveControlCount++] = i;
	}

	mMaxOptionCount = mActiveControlCount - 1;
	if (mSelectedControl > mMaxOptionCount) mSelectedControl = mMaxOptionCount;
	if (mSelectedControl < 0) mSelectedControl = 0;
}

void cerbiosIniEditorScene::refreshShortPath(int controlId)
{
	switch (controlId)
	{
		case CID_CdPath1: free(mShortCdPath1); mShortCdPath1 = shortenString(mConfig.CdPath1); break;
		case CID_CdPath2: free(mShortCdPath2); mShortCdPath2 = shortenString(mConfig.CdPath2); break;
		case CID_CdPath3: free(mShortCdPath3); mShortCdPath3 = shortenString(mConfig.CdPath3); break;
		case CID_DashPath1: free(mShortDashPath1); mShortDashPath1 = shortenString(mConfig.DashPath1); break;
		case CID_DashPath2: free(mShortDashPath2); mShortDashPath2 = shortenString(mConfig.DashPath2); break;
		case CID_DashPath3: free(mShortDashPath3); mShortDashPath3 = shortenString(mConfig.DashPath3); break;
		case CID_DashPath: free(mShortDashPath); mShortDashPath = shortenString(mConfig.DashPath); break;
		case CID_BootAnimPath: free(mShortBootAnimPath); mShortBootAnimPath = shortenString(mConfig.BootAnimPath); break;
	}
}

// ============================================================================
// Update — value mutations, navigation, sub-scene push
// ============================================================================

// Helpers for cycling — direction = +1 (right) or -1 (left).
static void cycleByte(unsigned char* v, int dir, int lo, int hi)
{
	int cur = (int)*v;
	int range = hi - lo + 1;
	cur += dir;
	while (cur < lo) cur += range;
	while (cur > hi) cur -= range;
	*v = (unsigned char)cur;
}

static const unsigned char kPartitionValues[] = { 1, 6, 7 };
static void cyclePartition(unsigned char* v, int dir)
{
	int idx = 0;
	for (int i = 0; i < 3; i++) { if (kPartitionValues[i] == *v) { idx = i; break; } }
	idx = (idx + dir + 3) % 3;
	*v = kPartitionValues[idx];
}

static void cycleFan(unsigned char* v, int dir)
{
	int cur = (int)*v;
	if (dir > 0) { cur = (cur >= 100) ? 0 : cur + 10; }
	else         { cur = (cur <= 0)   ? 100 : cur - 10; }
	*v = (unsigned char)cur;
}

void cerbiosIniEditorScene::update()
{
	// Info overlay short-circuits all other input
	if (mShowingInfo)
	{
		if (inputManager::buttonPressed(ButtonB)) mShowingInfo = false;
		return;
	}

	if (inputManager::buttonPressed(ButtonBlack))
	{
		mShowingInfo = true;
		return;
	}

	if (inputManager::buttonPressed(ButtonB))
	{
		sceneManager::popScene();
		return;
	}

	if (inputManager::buttonPressed(ButtonY))
	{
		cerbiosIniHelper::setConfigDefault(&mConfig, mVersion);
		mAdvancedUnlocked = false;
		rebuildActiveControls();
		mNeedsSave = true;
		// Refresh all short paths
		refreshShortPath(CID_CdPath1);
		refreshShortPath(CID_CdPath2);
		refreshShortPath(CID_CdPath3);
		refreshShortPath(CID_DashPath1);
		refreshShortPath(CID_DashPath2);
		refreshShortPath(CID_DashPath3);
		refreshShortPath(CID_DashPath);
		refreshShortPath(CID_BootAnimPath);
	}

	if (mNeedsSave && inputManager::buttonPressed(ButtonX))
	{
		char* buffer = (char*)malloc(65536);
		cerbiosIniHelper::buildConfig(&mConfig, mVersion, buffer);
		cerbiosIniHelper::saveConfig(mIniPath, buffer);
		free(buffer);
		mNeedsSave = false;
	}

	if (inputManager::buttonPressed(ButtonDpadDown))
	{
		mSelectedControl = mSelectedControl < mMaxOptionCount ? mSelectedControl + 1 : 0;
	}
	if (inputManager::buttonPressed(ButtonDpadUp))
	{
		mSelectedControl = mSelectedControl > 0 ? mSelectedControl - 1 : mMaxOptionCount;
	}

	const controlDescriptor* desc = &kControls[mActiveControls[mSelectedControl]];

	bool leftPress = inputManager::buttonPressed(ButtonA) || inputManager::buttonPressed(ButtonTriggerLeft);
	bool rightPress = inputManager::buttonPressed(ButtonTriggerRight);

	if (!leftPress && !rightPress) return;

	int dir = rightPress ? +1 : -1;

	switch (desc->kind)
	{
		case KIND_BOOL:
		{
			unsigned char* v = ucPtr(&mConfig, desc->offset);
			*v = (*v == 0) ? 1 : 0;
			mNeedsSave = true;
		} break;

		case KIND_DRIVE_SETUP:
		{
			cycleByte(ucPtr(&mConfig, desc->offset), dir, 0, 3);
			mNeedsSave = true;
		} break;

		case KIND_UDMA:
		{
			cycleByte(ucPtr(&mConfig, desc->offset), dir, 0, 6);
			mNeedsSave = true;
		} break;

		case KIND_FAN:
		{
			cycleFan(ucPtr(&mConfig, desc->offset), dir);
			mNeedsSave = true;
		} break;

		case KIND_FFILTER:
		{
			cycleByte(ucPtr(&mConfig, desc->offset), dir, 0, 6);
			mNeedsSave = true;
		} break;

		case KIND_BUS_BIN:
		case KIND_LCDPROTO:
		case KIND_HDD_BIN:
		{
			cycleByte(ucPtr(&mConfig, desc->offset), dir, 0, 1);
			mNeedsSave = true;
		} break;

		case KIND_PARTITION:
		{
			cyclePartition(ucPtr(&mConfig, desc->offset), dir);
			mNeedsSave = true;
		} break;

		case KIND_IGRPORT:
		{
			cycleByte(ucPtr(&mConfig, desc->offset), dir, 0, 4);
			mNeedsSave = true;
		} break;

		case KIND_LED:
		{
			sceneManager::pushScene(new sceneContainer(sceneItemGenericScene,
				new frontLedSelectorScene(mConfig.FrontLed), "", this, onFrontLedClosingCallback));
		} break;

		case KIND_PATH:
		{
			const char* current = charPtr(&mConfig, desc->offset);
			sceneManager::pushScene(new sceneContainer(sceneItemGenericScene,
				new keyboardScene(99, "Please enter path...", desc->label, current),
				"", this, onPathClosingCallback));
		} break;

		case KIND_HEX24:
		{
			uint32_t value = *u32Ptr(&mConfig, desc->offset);
			sceneManager::pushScene(new sceneContainer(sceneItemGenericScene,
				new hexSelectorScene(desc->label, value, 6, true),
				"", this, onHexClosingCallback));
		} break;

		case KIND_HEX_COMBO:
		{
			uint32_t value = (uint32_t)(*u16Ptr(&mConfig, desc->offset));
			sceneManager::pushScene(new sceneContainer(sceneItemGenericScene,
				new hexSelectorScene(desc->label, value, 4, false),
				"", this, onHexClosingCallback));
		} break;

		case KIND_HEX_BYTE:
		{
			uint32_t value = (uint32_t)(*ucPtr(&mConfig, desc->offset));
			sceneManager::pushScene(new sceneContainer(sceneItemGenericScene,
				new hexSelectorScene(desc->label, value, 2, true),
				"", this, onHexClosingCallback));
		} break;

		case KIND_ADV_UNLOCK:
		{
			mAdvancedUnlocked = !mAdvancedUnlocked;
			rebuildActiveControls();
		} break;
	}

	// File-picker entry on White button (KIND_PATH only)
	if (desc->kind == KIND_PATH && inputManager::buttonPressed(ButtonWhite))
	{
		mShowingFilePicker = true;
		sceneContainer* container = new sceneContainer(sceneItemGenericScene,
			new filePickerScene(filePickerTypeXbe, true, true),
			"", this, onPathClosingCallback);
		sceneManager::pushScene(container);
	}
}

// ============================================================================
// Render
// ============================================================================

static const char* valueTextFor(const controlDescriptor* desc, cerbiosConfig* cfg,
	char* mShortCdPath1, char* mShortCdPath2, char* mShortCdPath3,
	char* mShortDashPath1, char* mShortDashPath2, char* mShortDashPath3,
	char* mShortDashPath, char* mShortBootAnimPath, bool advUnlocked,
	char* scratch /*at least 16 bytes*/)
{
	switch (desc->kind)
	{
		case KIND_BOOL:        return kBools[*ucPtr(cfg, desc->offset) ? 1 : 0];
		case KIND_DRIVE_SETUP: return kDriveModes[*ucPtr(cfg, desc->offset)];
		case KIND_UDMA:        return kUdmaModes[*ucPtr(cfg, desc->offset)];
		case KIND_FAN:         return kFanSpeeds[*ucPtr(cfg, desc->offset) / 10];
		case KIND_FFILTER:     return kFFilter[*ucPtr(cfg, desc->offset)];
		case KIND_BUS_BIN:     return kBusBin[*ucPtr(cfg, desc->offset) ? 1 : 0];
		case KIND_LCDPROTO:    return kLcdProto[*ucPtr(cfg, desc->offset) ? 1 : 0];
		case KIND_HDD_BIN:     return kHddBin[*ucPtr(cfg, desc->offset) ? 1 : 0];
		case KIND_PARTITION:   return partitionLabel(*ucPtr(cfg, desc->offset));
		case KIND_IGRPORT:     return kIgrPort[*ucPtr(cfg, desc->offset) > 4 ? 0 : *ucPtr(cfg, desc->offset)];
		case KIND_LED:         return cfg->FrontLed;
		case KIND_HEX24:
		{
			sprintf(scratch, "0x%06X", *u32Ptr(cfg, desc->offset) & 0x00FFFFFF);
			return scratch;
		}
		case KIND_HEX_COMBO:
		{
			sprintf(scratch, "%X", *u16Ptr(cfg, desc->offset));
			return scratch;
		}
		case KIND_HEX_BYTE:
		{
			sprintf(scratch, "0x%02X", *ucPtr(cfg, desc->offset));
			return scratch;
		}
		case KIND_PATH:
		{
			switch (desc->id)
			{
				case CID_CdPath1: return mShortCdPath1;
				case CID_CdPath2: return mShortCdPath2;
				case CID_CdPath3: return mShortCdPath3;
				case CID_DashPath1: return mShortDashPath1;
				case CID_DashPath2: return mShortDashPath2;
				case CID_DashPath3: return mShortDashPath3;
				case CID_DashPath: return mShortDashPath;
				case CID_BootAnimPath: return mShortBootAnimPath;
				default: return "";
			}
		}
		case KIND_ADV_UNLOCK: return kBools[advUnlocked ? 1 : 0];
	}
	return "";
}

void cerbiosIniEditorScene::render()
{
	component::panel(theme::getPanelFillColor(), theme::getPanelStrokeColor(), 16, 16, 688, 448);

	const char* title = (mVersion == CerbiosVersionCurrent)
		? "Cerbios INI Editor (3.0.0+)..."
		: "Cerbios INI Editor (2.4.2 and below)...";
	drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), title, theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);

	int32_t maxItems = 7;
	int32_t menuItems = mMaxOptionCount + 1;

	int32_t start = 0;
	if (menuItems >= maxItems)
	{
		start = min(max(mSelectedControl - (maxItems / 2), 0), menuItems - maxItems);
	}

	int32_t itemCount = min(start + maxItems, menuItems) - start;
	char scratch[16];

	if (itemCount > 0)
	{
		uint32_t yPos = (context::getBufferHeight() - ((itemCount * 40) - 10)) / 2;
		yPos += theme::getCenterOffset();

		for (int32_t i = 0; i < itemCount; i++)
		{
			int32_t index = start + i;
			const controlDescriptor* desc = &kControls[mActiveControls[index]];
			const char* valueText = valueTextFor(desc, &mConfig,
				mShortCdPath1, mShortCdPath2, mShortCdPath3,
				mShortDashPath1, mShortDashPath2, mShortDashPath3,
				mShortDashPath, mShortBootAnimPath, mAdvancedUnlocked,
				scratch);
			component::splitButton(mSelectedControl == index, false, desc->label, 165, valueText, 40, yPos, 640, 30);
			yPos += 40;
		}
	}

	// File-picker affordance for the currently-selected control
	const controlDescriptor* selDesc = &kControls[mActiveControls[mSelectedControl]];
	mHasFilePicker = (selDesc->kind == KIND_PATH);

	uint32_t yPos = (context::getBufferHeight() - ((itemCount * 40) - 10)) / 2;
	yPos += theme::getCenterOffset();

	if (mShowingInfo)
	{
		char* infoStr = getOptionInfo(selDesc->id);
		component::textBox(infoStr, true, false, horizAlignmentLeft, 60, yPos + 5, 600, 260, true, true);
	}

	if (!mShowingInfo)
	{
		const char* baseBtns = "\xC2\xA1 or \xC2\xB2\xC2\xB3 Change Value, \xC2\xA4 Defaults";
		const char* filePkrBtn = ", \xC2\xB5 Browse";
		const char* saveBtn = ", \xC2\xA3 Save";

		char* buttons = stringUtility::formatString(
			"%s%s%s",
			baseBtns,
			mHasFilePicker ? filePkrBtn : "",
			mNeedsSave ? saveBtn : ""
		);

		drawing::drawBitmapString(context::getBitmapFontSmall(), buttons, theme::getFooterTextColor(), 40, theme::getFooterY());
		free(buttons);
	}

	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), mShowingInfo ? "\xC2\xA2 Back" : "\xC2\xB6 Info  \xC2\xA2 Back", theme::getFooterTextColor(), horizAlignmentRight, 40, theme::getFooterY(), 640);
}

// ============================================================================
// shortenString — unchanged from original
// ============================================================================

char* cerbiosIniEditorScene::shortenString(const char* value)
{
	int textWidth;
	int textHeight;
	drawing::measureBitmapString(context::getBitmapFontSmall(), value, &textWidth, &textHeight);

	char* temp = strdup(value);

	int maxWidth = (640 - 165) - 20;
	if (textWidth >= maxWidth)
	{
		for (int i = 0; i < (int)strlen(value); i++)
		{
			free(temp);
			temp = stringUtility::formatString("...%s", value + i);
			drawing::measureBitmapString(context::getBitmapFontSmall(), temp, &textWidth, &textHeight);
			if (textWidth < maxWidth)
			{
				break;
			}
		}
	}

	return temp;
}

// ============================================================================
// Info-text lookup
// ============================================================================

char* cerbiosIniEditorScene::getOptionInfo(int controlId)
{
	switch (controlId)
	{
		case CID_DriveSetup: return (char*)
			"Drive Setup (DriveSetup):\n"
			"\n"
			"0 = HDD & DVD (Stock)\n"
			"1 = HDD & No DVD (Legacy Mode)\n"
			"2 = HDD & No DVD (Modern Mode)\n"
			"3 = Dual HDD (slave HDD in DVD bay)\n"
			"\n"
			"Default = 1";
		case CID_AVCheck: return (char*)
			"AV Check (AVCheck):\n"
			"\n"
			"Halts boot with flashing LEDs if no AV cable detected.\n"
			"\n"
			"Default = True";
		case CID_Debug: return (char*)
			"Debug (Debug):\n"
			"\n"
			"Boots C:\\xshell.xbe and loads E:\\xbdm.dll for VS debugging.\n"
			"\n"
			"Default = False";
		case CID_Force480p: return (char*)
			"Force 480p (Force480p):\n"
			"\n"
			"Forces 480i games/apps to render in 480p. Requires component\n"
			"cables and 480p enabled in MS Dashboard. PAL unaffected.\n"
			"\n"
			"Default = False";
		case CID_ForceVGA: return (char*)
			"Force VGA (ForceVGA):\n"
			"\n"
			"Forces VGA output using RGsB signal. Auto-enables Force480p\n"
			"and NTSC mode. Not compatible with 1.6 consoles.\n"
			"\n"
			"Default = False";
		case CID_ForceFFilter: return (char*)
			"Force Flicker Filter (ForceFFilter):\n"
			"\n"
			"0 = Off, 1-5 = Filter strength, 6 = System/Game default\n"
			"\n"
			"Default = 6";
		case CID_UdmaModeMaster:
		case CID_UdmaModeSlave: return (char*)
			"UDMA Mode (UdmaModeMaster, UdmaModeSlave):\n"
			"\n"
			"0 = Auto (Startech), 1 = Auto (Generic)\n"
			"2-6 = UDMA modes (higher = faster, 80-wire IDE required for 3+)\n"
			"\n"
			"Default = 2";
		case CID_FanSpeed: return (char*)
			"Fan Speed (FanSpeed):\n"
			"\n"
			"0 = Auto, 10-100 = Manual percent (steps of 10)\n"
			"\n"
			"Default = 0";
		case CID_OverrideFan: return (char*)
			"Override Fan (OverrideFan):\n"
			"\n"
			"Allows Cerbios to control fan speed on boot/reboot/launch.\n"
			"Disable to let apps/dash override.\n"
			"\n"
			"Default = True";
		case CID_FrontLed: return (char*)
			"Front LED (FrontLed):\n"
			"\n"
			"4-character pattern. G = Green, R = Red, A = Amber, O = Off\n"
			"\n"
			"Default = GGGG";
		case CID_BlockDashUpdate: return (char*)
			"Block Dash Update (BlockDashUpdate):\n"
			"\n"
			"Prevents games from updating the MS Dashboard. Useful for\n"
			"softmods running in BFM mode.\n"
			"\n"
			"Default = False";
		case CID_ResetOnEject: return (char*)
			"Reset On Eject (ResetOnEject):\n"
			"\n"
			"Restores stock Xbox behavior of resetting on DVD eject.\n"
			"\n"
			"Default = False";
		case CID_RtcEnable: return (char*)
			"RTC Enable (RtcEnable / RTCEnable):\n"
			"\n"
			"Auto time-sync with external RTC module on SMBus.\n"
			"Requires Cerbios 2.4.0+ or PrometheOS 1.4.0+. DS3231 supported.\n"
			"\n"
			"Default = False";
		case CID_CdPath1:
		case CID_CdPath2:
		case CID_CdPath3: return (char*)
			"CD Paths (CdPath1, CdPath2, CdPath3):\n"
			"\n"
			"List of XBEs tried on disc detect in D:. Falls back to D:\\default.xbe.\n"
			"\n"
			"Default = (all blank)";
		case CID_DashPath1:
		case CID_DashPath2:
		case CID_DashPath3: return (char*)
			"Dash Paths (DashPath1, DashPath2, DashPath3):\n"
			"\n"
			"Dashboard XBEs tried on boot. Allows HDD0-/HDD1- prefixes.\n"
			"Falls back to C:\\xboxdash.xbe.";
		case CID_DashPath: return (char*)
			"Dash Path (DashPath):\n"
			"\n"
			"Primary dashboard XBE. Supports HDD0-/HDD1- prefixes.\n"
			"Falls back to E:\\Cerbios\\Recovery\\default.xbe then C:\\xboxdash.xbe.\n"
			"\n"
			"Default = HDD0-C:\\evoxdash.xbe";
		case CID_BootAnimPath: return (char*)
			"Boot Animation Path (BootAnimPath):\n"
			"\n"
			"Custom boot animation XBE.\n"
			"\n"
			"3.0.0+ default = HDD0-E:\\Cerbios\\BootAnims\\Xbox\\bootanim.xbe\n"
			"2.4.2 default  = C:\\BootAnims\\Xbox\\bootanim.xbe";
		case CID_XonlineDashRedir: return (char*)
			"Xonline Dash Redir (XonlineDashRedir):\n"
			"\n"
			"Redirects XBL dash calls to Xonlinedash_Original.xbe.\n"
			"\n"
			"Default = False";
		case CID_AdvCPUSupport: return (char*)
			"Adv CPU Support (AdvCPUSupport):\n"
			"\n"
			"Runtime XBE patching for CPU-upgraded or overclocked consoles.\n"
			"No effect on stock CPUs.\n"
			"\n"
			"Default = True";
		case CID_DisableLimitMem: return (char*)
			"Disable Limit Mem (DisableLimitMem):\n"
			"\n"
			"Ignores /LIMITMEM in XBEs to allow full 128MB. Only safe on\n"
			"128MB consoles. May cause compat issues.\n"
			"\n"
			"Default = False";
		case CID_ApplyTitlePatches: return (char*)
			"Apply Title Patches (ApplyTitlePatches):\n"
			"\n"
			"Runtime XBE patches for known game issues.\n"
			"\n"
			"Default = True";
		case CID_ReadOnlyC: return (char*)
			"Read-Only C (ReadOnlyC):\n"
			"\n"
			"Prevents writes to C: while Cerbios runs. Useful for softmods.\n"
			"\n"
			"Default = False";
		case CID_InAppLCDEnable: return (char*)
			"In-App LCD (InAppLCDEnable):\n"
			"\n"
			"Enables in-game LCD for FPS, temps, RAM, etc.\n"
			"\n"
			"Default = False";
		case CID_LCDBus: return (char*)
			"LCD Bus (LCDBus):\n"
			"\n"
			"0 = SMBus, 1 = X3LCD via X3 modchip\n"
			"\n"
			"Default = 0";
		case CID_LCDI2CAddr: return (char*)
			"LCD I2C Address (LCDI2CAddr):\n"
			"\n"
			"Valid: 0x20-0x27 or 0x38-0x3F depending on module.\n"
			"\n"
			"Default = 0x3C";
		case CID_LCDProto: return (char*)
			"LCD Protocol (LCDProto):\n"
			"\n"
			"0 = HD44780 / X3LCD, 1 = US2066 (NHD-0420CW)\n"
			"\n"
			"Default = 0";
		case CID_TUDATARedir: return (char*)
			"TUDATA Redirect (TUDATARedir):\n"
			"\n"
			"Redirects UDATA/TDATA to alternate drive/partition.\n"
			"\n"
			"Default = False";
		case CID_TUDATARedirHDD: return (char*)
			"TUDATA Redir HDD (TUDATARedirHDD):\n"
			"\n"
			"0 = Master, 1 = Slave\n"
			"\n"
			"Default = Master";
		case CID_TUDATARedirPart: return (char*)
			"TUDATA Redir Part (TUDATARedirPart):\n"
			"\n"
			"1 = E:, 6 = F:, 7 = G:\n"
			"\n"
			"Default = E:";
		case CID_EnableScreenshots: return (char*)
			"Enable Screenshots (EnableScreenshots):\n"
			"\n"
			"Capture XBE framebuffer to E:\\Cerbios\\Screenshots\\(TitleID)\n"
			"when L-Thumb + R-Thumb pressed.\n"
			"\n"
			"Default = False";
		case CID_ScreenshotHDD: return (char*)
			"Screenshot HDD (ScreenshotHDD):\n"
			"\n"
			"0 = Master, 1 = Slave\n"
			"\n"
			"Default = Master";
		case CID_ScreenshotPart: return (char*)
			"Screenshot Part (ScreenshotPart):\n"
			"\n"
			"1 = E:, 6 = F:, 7 = G:\n"
			"\n"
			"Default = E:";
		case CID_IGRMasterPort: return (char*)
			"IGR Master Port (IGRMasterPort):\n"
			"\n"
			"Which controller can trigger IGR combos.\n"
			"0 = ALL, 1-4 = Controller ports.\n"
			"\n"
			"Default = ALL";
		case CID_IGRDash:
		case CID_IGRGame:
		case CID_IGRFull:
		case CID_IGRCycle:
		case CID_IGRShutdown:
		case CID_IGRScreen: return (char*)
			"IGR Combo (hex digits = button sequence):\n"
			"\n"
			"A=0 B=1 X=2 Y=3 BLACK=4 WHITE=5 LT=6 RT=7\n"
			"DUP=8 DDN=9 DL=A DR=B START=C BACK=D LT-STK=E RT-STK=F\n"
			"\n"
			"Edit each hex digit with the on-screen picker.";
		case CID_AdvUnlock: return (char*)
			"Unlock Advanced (Overclock):\n"
			"\n"
			"Toggle to reveal Overclocking / CPUMPLLCoeff / NVPLLCoeff.\n"
			"These directly expose CPU/GPU PLL hardware registers.\n"
			"Incorrect values can cause instability or damage.\n"
			"Team Cerbios is not responsible for misuse.";
		case CID_Overclocking: return (char*)
			"Overclocking (Overclocking):\n"
			"\n"
			"Master switch for the CPU/GPU overclock coefficients below.\n"
			"\n"
			"Default = False";
		case CID_CPUMPLLCoeff: return (char*)
			"CPU Overclock (CPUMPLLCoeff):\n"
			"\n"
			"3-byte hex value controlling CPU PLL. Stock = 0x230801.\n"
			"Example: 0x232304 for ~802MHz on a stock 733MHz CPU.\n"
			"Eject during boot enters safe mode with stock values.\n"
			"\n"
			"Default = 0x000000";
		case CID_NVPLLCoeff: return (char*)
			"GPU Overclock (NVPLLCoeff):\n"
			"\n"
			"3-byte hex value controlling GPU PLL. Stock = 0x011C01.\n"
			"Example: 0x012001 for ~266MHz GPU.\n"
			"\n"
			"Default = 0x000000";
	}
	return (char*)"NO INFO AVAILABLE FOR THIS OPTION";
}
