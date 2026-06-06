#include "systemInfoScene.h"
#include "sceneManager.h"
#include "removeScene.h"
#include "flashFlowScene.h"
#include "editFlowScene.h"
#include "audioSettingsScene.h"
#include "videoSettingsScene.h"

#include "..\context.h"
#include "..\drawing.h"
#include "..\component.h"
#include "..\ssfn.h"
#include "..\inputManager.h"
#include "..\settingsManager.h"
#include "..\hdmiDevice.h"
#include "..\stringUtility.h"
#include "..\xboxConfig.h"
#include "..\theme.h"
#include "..\driveManager.h"

namespace
{
	char* formatStorageSize(uint64_t bytes)
	{
		const uint64_t GB = 1024ULL * 1024ULL * 1024ULL;
		const uint64_t MB = 1024ULL * 1024ULL;
		if (bytes >= GB) {
			return stringUtility::formatString("%.1fGB", (float)bytes / (float)GB);
		} else if (bytes >= MB) {
			return stringUtility::formatString("%uMB", (uint32_t)(bytes / MB));
		}
		return stringUtility::formatString("%uKB", (uint32_t)(bytes / 1024));
	}
}

systemInfoScene::systemInfoScene(systemInfoCategoryEnum systemInfoCategory)
{
	mSelectedControl = 0;
	mInfoItems = new pointerVector<char*>(false);
	mSystemInfoCategory = systemInfoCategory;
	mPartitions = NULL;
	
	if (mSystemInfoCategory == systemInfoCategoryConsole)
	{
		char* cpuSpeed = stringUtility::formatString("CPU: %4.2fMHz", xboxConfig::getCPUFreq());
		mInfoItems->add(cpuSpeed);

		char* xboxVersionString = xboxConfig::getXboxVersionString();
		char* xboxVersion = stringUtility::formatString("Xbox Rev: %s", xboxVersionString);
		mInfoItems->add(xboxVersion);
		free(xboxVersionString);

		char* totalMemory = stringUtility::formatString("RAM: %uMB", utils::getTotalPhysicalMemory() >> 20);
		mInfoItems->add(totalMemory);

		char* serialString = xboxConfig::getSerialString();
		char* serial = stringUtility::formatString("Serial: %s", serialString);
		mInfoItems->add(serial);
		free(serialString);

		char* macString = xboxConfig::getMacString();
 		char* mac = stringUtility::formatString("Mac: %s", macString);
		mInfoItems->add(mac);
		free(macString);

		char* rtcExpansion = stringUtility::formatString("RTC Expansion: %s", xboxConfig::getHasRtcExpansion() ? "Detected" : "Not Detected");
		mInfoItems->add(rtcExpansion);
	}
	else if (mSystemInfoCategory == systemInfoCategoryStorage)
	{
		const char* model = HalDiskModelNumber->Buffer ? HalDiskModelNumber->Buffer : "";
		char* diskModelNumberString = stringUtility::trim(model, ' ');
		char* diskModelNumber = stringUtility::formatString("HDD Model: %s", diskModelNumberString);
		mInfoItems->add(diskModelNumber);
		free(diskModelNumberString);

		const char* serial = HalDiskSerialNumber->Buffer ? HalDiskSerialNumber->Buffer : "";
		char* diskSerialNumberString = stringUtility::trim(serial, ' ');
		char* diskSerialNumber = stringUtility::formatString("HDD Serial: %s", diskSerialNumberString);
		mInfoItems->add(diskSerialNumber);
		free(diskSerialNumberString);

		mPartitions = new pointerVector<partitionInfo*>(true);

		static const char* mountPoints[] = {
			"HDD0-C", "HDD0-E", "HDD0-F", "HDD0-G", "HDD0-H", "HDD0-I",
			"HDD0-J", "HDD0-K", "HDD0-L", "HDD0-M", "HDD0-N",
			"HDD0-X", "HDD0-Y", "HDD0-Z"
		};
		static const char* partitionLabels[] = {
			"Drive C", "Drive E", "Drive F", "Drive G", "Drive H", "Drive I",
			"Drive J", "Drive K", "Drive L", "Drive M", "Drive N",
			"Drive X", "Drive Y", "Drive Z"
		};

		for (int pi = 0; pi < 14; pi++)
		{
			uint64_t total = 0;
			if (driveManager::getTotalNumberOfBytes(mountPoints[pi], total) && total > 0)
			{
				uint64_t freeBytes = 0;
				driveManager::getTotalFreeNumberOfBytes(mountPoints[pi], freeBytes);
				partitionInfo* info = new partitionInfo();
				strncpy(info->label, partitionLabels[pi], sizeof(info->label) - 1);
				info->label[sizeof(info->label) - 1] = '\0';
				info->total = total;
				info->used = total - freeBytes;
				mPartitions->add(info);
			}
		}
	}
	else if (mSystemInfoCategory == systemInfoCategoryAudio)
	{
		char* audioModeString = xboxConfig::getAudioModeString();
		char* audioMode = stringUtility::formatString("Audio Mode: %s", audioModeString);
		mInfoItems->add(audioMode);
		free(audioModeString);

		char* ac3 = stringUtility::formatString("Dolby digital (AC3): %s", xboxConfig::getAudioAC3() ? "Enabled" : "Disabled");
		mInfoItems->add(ac3);

		char* dts = stringUtility::formatString("DTS: %s", xboxConfig::getAudioDTS() ? "Enabled" : "Disabled");
		mInfoItems->add(dts);
	}
	else if (mSystemInfoCategory == systemInfoCategoryVideo)
	{
		char* videoStandardString = xboxConfig::getVideoStandardString();
		char* videoStandard = stringUtility::formatString("Video Standard: %s", videoStandardString);
		mInfoItems->add(videoStandard);
		free(videoStandardString);

		char* gameRegionString = xboxConfig::getGameRegionString();
		char* gameRegion = stringUtility::formatString("Game Region: %s", gameRegionString);
		mInfoItems->add(gameRegion);
		free(gameRegionString);

		char* dvdRegionString = xboxConfig::getDvdRegionString();
		char* dvdRegion = stringUtility::formatString("DVD Region: %s", dvdRegionString);
		mInfoItems->add(dvdRegion);
		free(dvdRegionString);

		char* avPackString = xboxConfig::getAvPackString();
		char* avPack = stringUtility::formatString("AV Pack: %s", avPackString);
		free(avPackString);

		char* encoderString = xboxConfig::getEncoderString();
		char* encoder = stringUtility::formatString("Encoder: %s", encoderString);
		mInfoItems->add(encoder);
		free(encoderString);

		char* hdModString = xboxConfig::getHdModString();
		char* hdMod = stringUtility::formatString("HD Mod: %s", hdModString);
		mInfoItems->add(hdMod);
		free(hdModString);
	}
	else if (mSystemInfoCategory == systemInfoCategoryAbout)
	{
		char *versionSemver = settingsManager::getVersionSting(settingsManager::getVersion());
		char *version = stringUtility::formatString("PrometheOS: V%s", versionSemver);
		mInfoItems->add(version);
		free(versionSemver);

		char *by = strdup("Team Cerbios + Team Resurgent");
		mInfoItems->add(by);

		char *coded = strdup("Coded By: EqUiNoX");
		mInfoItems->add(coded);

		char* skinAutthor = theme::getSkinAuthor();
		char *author = stringUtility::formatString("Skin Author: %s", skinAutthor);
		mInfoItems->add(author);
		free(skinAutthor);

		if (drawing::imageExists("installer-logo") == false)
		{
			utils::dataContainer* installerLogoData = settingsManager::getInstallerLogoData();
			if (installerLogoData->data[0] == 'I' && installerLogoData->data[1] == 'M')
			{
				int width = (uint8_t)installerLogoData->data[2];
				int height = (uint8_t)installerLogoData->data[3];
				drawing::addImage("installer-logo", (uint8_t*)(installerLogoData->data + 4), D3DFMT_A8B8G8R8, width, height);
			}
			delete(installerLogoData);
		}
	}
}

systemInfoScene::~systemInfoScene()
{
	delete(mInfoItems);
	if (mPartitions != NULL)
	{
		delete(mPartitions);
	}
}

void systemInfoScene::update()
{
	// Exit Action

	if (inputManager::buttonPressed(ButtonB))
	{
		sceneManager::popScene();
		return;
	}
	
	// Down Actions

	if (inputManager::buttonPressed(ButtonDpadDown))
	{
		int32_t navCount = (mPartitions != NULL) ? (int32_t)mPartitions->count() : (mInfoItems != NULL ? (int32_t)mInfoItems->count() : 0);
		if (navCount > 0)
		{
			mSelectedControl = mSelectedControl < (navCount - 1) ? mSelectedControl + 1 : 0;
		}
	}

	// Up Actions

	if (inputManager::buttonPressed(ButtonDpadUp))
	{
		int32_t navCount = (mPartitions != NULL) ? (int32_t)mPartitions->count() : (mInfoItems != NULL ? (int32_t)mInfoItems->count() : 0);
		if (navCount > 0)
		{
			mSelectedControl = mSelectedControl > 0 ? mSelectedControl - 1 : (navCount - 1);
		}
	}
}

void systemInfoScene::render()
{
	component::panel(theme::getPanelFillColor(), theme::getPanelStrokeColor(), 16, 16, 688, 448);

	if (mSystemInfoCategory == systemInfoCategoryConsole)
	{
		drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "System Info: Console", theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);
	}
	else if (mSystemInfoCategory == systemInfoCategoryStorage)
	{
		drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "System Info: Storage", theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);
	}
	else if (mSystemInfoCategory == systemInfoCategoryAudio)
	{
		drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "System Info: Audio", theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);
	}
	else if (mSystemInfoCategory == systemInfoCategoryVideo)
	{
		drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "System Info: Video", theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);
	}
	else if (mSystemInfoCategory == systemInfoCategoryAbout)
	{
		drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "System Info: About", theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);
	}

	if (mSystemInfoCategory == systemInfoCategoryStorage && mPartitions != NULL)
	{
		uint32_t yPos = 80;

		for (uint32_t i = 0; i < mInfoItems->count(); i++)
		{
			component::textBox(mInfoItems->get(i), false, false, horizAlignmentCenter, 40, yPos, 640, 28);
			yPos += 34;
		}

		yPos += 8;

		int32_t maxPartitions = 6;
		int32_t partCount = (int32_t)mPartitions->count();
		int32_t startPart = 0;
		if (partCount > maxPartitions)
		{
			startPart = min(max(mSelectedControl - (maxPartitions / 2), 0), partCount - maxPartitions);
		}
		int32_t visibleCount = min(startPart + maxPartitions, partCount) - startPart;

		for (int32_t i = 0; i < visibleCount; i++)
		{
			partitionInfo* info = mPartitions->get(startPart + i);
			bool selected = (mSelectedControl == startPart + i);

			char* usedStr = formatStorageSize(info->used);
			char* totalStr = formatStorageSize(info->total);
			uint32_t pct = (uint32_t)((info->used * 100ULL) / info->total);
			char* rowLabel = stringUtility::formatString("%s  %s / %s (%u%%)", info->label, usedStr, totalStr, pct);
			free(usedStr);
			free(totalStr);

			component::textBox(rowLabel, selected, false, horizAlignmentLeft, 40, yPos, 640, 22);
			free(rowLabel);

			int barX = 56;
			int barWidth = 608;
			int barY = (int)yPos + 24;
			int barH = 8;

			drawing::drawHorizontalLine(0xff374956, barX, barY, barWidth, barH);

			int fillWidth = (int)((uint64_t)info->used * (uint64_t)barWidth / info->total);
			if (fillWidth > barWidth) fillWidth = barWidth;
			if (fillWidth > 0)
			{
				uint32_t fillColor = 0xff19b3e6;
				if (pct >= 95) fillColor = 0xffd40c00;
				else if (pct >= 80) fillColor = 0xffffcd00;
				drawing::drawHorizontalLine(fillColor, barX, barY, fillWidth, barH);
			}

			yPos += 40;
		}

		drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "\xC2\xA2 Back", theme::getFooterTextColor(), horizAlignmentRight, 40, theme::getFooterY(), 640);
		return;
	}

	uint32_t yPos = 96;

	int32_t maxItems = 7;

	int32_t start = 0;
	if ((int32_t)mInfoItems->count() >= maxItems)
	{
		start = min(max(mSelectedControl - (maxItems / 2), 0), (int32_t)mInfoItems->count() - maxItems);
	}

	int32_t itemCount = min(start + maxItems, (int32_t)mInfoItems->count()) - start; 
	if (itemCount > 0)
	{
		uint32_t yPos = (context::getBufferHeight() - ((itemCount * 40) - 10)) / 2;
		yPos += theme::getCenterOffset();

		if (mSystemInfoCategory == systemInfoCategoryAbout && drawing::imageExists("installer-logo") == true)
		{
			yPos += 32;
			drawing::drawImage("installer-logo", theme::getInstallerTint(), 271, yPos - 64, 178, 46);
		}

		for (int32_t i = 0; i < itemCount; i++)
		{
			uint32_t index = start + i;
			if (index >= mInfoItems->count())
			{
				continue;
			}
			char* infoItem = mInfoItems->get(index);
			component::textBox(infoItem, mSelectedControl == index, false, horizAlignmentCenter, 40, yPos, 640, 30);
			yPos += 40;
		}
	}
	else
	{
		int yPos = ((context::getBufferHeight() - 44) / 2);
		yPos += theme::getCenterOffset();

		component::textBox("No items", false, false, horizAlignmentCenter, 193, 225, 322, 44);
	}
	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "\xC2\xA2 Back", theme::getFooterTextColor(), horizAlignmentRight, 40, theme::getFooterY(), 640);
}
