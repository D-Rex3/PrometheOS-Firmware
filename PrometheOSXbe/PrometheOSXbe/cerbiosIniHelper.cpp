#include "cerbiosIniHelper.h"

#include "xboxInternals.h"
#include "stringUtility.h"
#include "utils.h"
#include "fileSystem.h"

void cerbiosIniHelper::upperCase(char* value)
{
    for (uint32_t i = 0; i < (int)strlen(value); i++)
	{
        value[i] = (char)(value[i] >= 97 && value[i] <= 122 ? value[i] - 32 : value[i]);
    }
}

void cerbiosIniHelper::trimSpace(char* value)
{
    char* pos = value;
    int length = strlen(pos);

    while (length > 0 && isspace(pos[length - 1])) {
        pos[length - 1] = 0;
        length--;
    }

    while (length > 0 && *pos && isspace(*pos)) {
        ++pos;
        --length;
    }

    memmove(value, pos, length + 1);
}

uint8_t cerbiosIniHelper::parseByte(char* value, uint8_t defaultValue)
{
    char* endPtr;
    uint8_t result;

    upperCase(value);

    if (strncmp(value, "0X", 2) == 0)
    {
        result = (uint8_t)strtol(value + 2, &endPtr, 16);
    }
    else
    {
        result = (uint8_t)strtol(value, &endPtr, 10);
    }

    if (*endPtr == 0)
    {
        return result;
    }

    return defaultValue;
}

uint8_t cerbiosIniHelper::parseBoolean(char* value, uint8_t defaultValue)
{
    upperCase(value);
    if (strcmp(value, "TRUE") == 0) {
        return 1;
    }
    else if (strcmp(value, "FALSE") == 0) {
        return 0;
    }
    return defaultValue;
}

uint32_t cerbiosIniHelper::parseHex(char* value, uint32_t defaultValue)
{
    char* endPtr;
    upperCase(value);
    const char* start = (strncmp(value, "0X", 2) == 0) ? value + 2 : value;
    uint32_t result = (uint32_t)strtoul(start, &endPtr, 16);
    if (*endPtr == 0)
    {
        return result;
    }
    return defaultValue;
}

uint16_t cerbiosIniHelper::parseHexU16(char* value, uint16_t defaultValue)
{
    char* endPtr;
    upperCase(value);
    const char* start = (strncmp(value, "0X", 2) == 0) ? value + 2 : value;
    uint32_t result = (uint32_t)strtoul(start, &endPtr, 16);
    if (*endPtr == 0)
    {
        return (uint16_t)result;
    }
    return defaultValue;
}

void cerbiosIniHelper::parseConfigLine(cerbiosConfig* config, char* param1, char* param2, char* buffer, unsigned long bufferSize)
{
	char* params[2];
    params[0] = param1;
    params[1] = param2;

	uint32_t paramLengths[2];
    paramLengths[0] = 0;
    paramLengths[1] = 0;

    uint32_t paramIndex = 0;
    for (uint32_t i = 0; i < bufferSize; i++) {
        char currentChar = buffer[i];
        if (paramIndex > 1) {
            return;
        }
        else if (currentChar == '=') {
            params[paramIndex][paramLengths[paramIndex]] = 0;
            paramIndex++;
            continue;
        }
        else if (currentChar == ';') {
            break;
        }
        if (paramLengths[paramIndex] > 255) {
            continue;
        }
        params[paramIndex][paramLengths[paramIndex]] = currentChar;
        paramLengths[paramIndex] = paramLengths[paramIndex] + 1;
    }
    params[paramIndex][paramLengths[paramIndex]] = 0;

    if (paramIndex != 1) {
        return;
    }

    trimSpace(params[0]);
    upperCase(params[0]);

    trimSpace(params[1]);

    // Shared keys (both versions)
    if (strcmp(params[0], "DRIVESETUP") == 0) {
        config->DriveSetup = parseByte(params[1], config->DriveSetup);
        config->DriveSetup  = min(max(config->DriveSetup , 0), 3);
    }
    else if (strcmp(params[0], "AVCHECK") == 0) {
        config->AVCheck = parseBoolean(params[1], config->AVCheck);
    }
    else if (strcmp(params[0], "DEBUG") == 0) {
        config->Debug = parseBoolean(params[1], config->Debug);
    }
    else if (strcmp(params[0], "BOOTANIMPATH") == 0) {
        strncpy(config->BootAnimPath, params[1], 99);
    }
    else if (strcmp(params[0], "FRONTLED") == 0 && strlen(params[1]) == 4) {
        strncpy(config->FrontLed, params[1], 4);
        upperCase(config->FrontLed);
    }
    else if (strcmp(params[0], "FANSPEED") == 0) {
        config->FanSpeed = parseByte(params[1], config->FanSpeed);
    }
    else if (strcmp(params[0], "UDMAMOE") == 0) {
        config->UdmaModeMaster = parseByte(params[1], config->UdmaModeMaster);
    }
    else if (strcmp(params[0], "UDMAMODEMASTER") == 0) {
        config->UdmaModeMaster = parseByte(params[1], config->UdmaModeMaster);
    }
    else if (strcmp(params[0], "UDMAMODESLAVE") == 0) {
        config->UdmaModeSlave = parseByte(params[1], config->UdmaModeSlave);
    }
    else if (strcmp(params[0], "FORCE480P") == 0) {
        config->Force480p = parseBoolean(params[1], config->Force480p);
    }
    else if (strcmp(params[0], "FORCEVGA") == 0) {
        config->ForceVGA = parseBoolean(params[1], config->ForceVGA);
    }
    else if (strcmp(params[0], "RTCENABLE") == 0) {
        // 2.4.2 spells this "RtcEnable", 3.0.0 uses "RTCEnable" -- both upper-case to RTCENABLE here.
        config->RtcEnable = parseBoolean(params[1], config->RtcEnable);
    }
    else if (strcmp(params[0], "BLOCKDASHUPDATE") == 0) {
        config->BlockDashUpdate = parseBoolean(params[1], config->BlockDashUpdate);
    }
    else if (strcmp(params[0], "RESETONEJECT") == 0) {
        config->ResetOnEject = parseBoolean(params[1], config->ResetOnEject);
    }
    // Legacy (2.4.2) keys
    else if (strcmp(params[0], "CDPATH1") == 0) {
        strncpy(config->CdPath1, params[1], 99);
    }
    else if (strcmp(params[0], "CDPATH2") == 0) {
        strncpy(config->CdPath2, params[1], 99);
    }
    else if (strcmp(params[0], "CDPATH3") == 0) {
        strncpy(config->CdPath3, params[1], 99);
    }
    else if (strcmp(params[0], "DASHPATH1") == 0) {
        strncpy(config->DashPath1, params[1], 99);
    }
    else if (strcmp(params[0], "DASHPATH2") == 0) {
        strncpy(config->DashPath2, params[1], 99);
    }
    else if (strcmp(params[0], "DASHPATH3") == 0) {
        strncpy(config->DashPath3, params[1], 99);
    }
    // Current (3.0.0+) keys
    else if (strcmp(params[0], "DASHPATH") == 0) {
        strncpy(config->DashPath, params[1], 99);
    }
    else if (strcmp(params[0], "FORCEFFILTER") == 0) {
        config->ForceFFilter = parseByte(params[1], config->ForceFFilter);
        config->ForceFFilter = min(max(config->ForceFFilter, 0), 6);
    }
    else if (strcmp(params[0], "OVERRIDEFAN") == 0) {
        config->OverrideFan = parseBoolean(params[1], config->OverrideFan);
    }
    else if (strcmp(params[0], "OVERCLOCKING") == 0) {
        config->Overclocking = parseBoolean(params[1], config->Overclocking);
    }
    else if (strcmp(params[0], "CPUMPLLCOEFF") == 0) {
        config->CPUMPLLCoeff = parseHex(params[1], config->CPUMPLLCoeff) & 0x00FFFFFF;
    }
    else if (strcmp(params[0], "NVPLLCOEFF") == 0) {
        config->NVPLLCoeff = parseHex(params[1], config->NVPLLCoeff) & 0x00FFFFFF;
    }
    else if (strcmp(params[0], "INAPPLCDENABLE") == 0) {
        config->InAppLCDEnable = parseBoolean(params[1], config->InAppLCDEnable);
    }
    else if (strcmp(params[0], "LCDBUS") == 0) {
        config->LCDBus = parseByte(params[1], config->LCDBus);
    }
    else if (strcmp(params[0], "LCDI2CADDR") == 0) {
        config->LCDI2CAddr = parseByte(params[1], config->LCDI2CAddr);
    }
    else if (strcmp(params[0], "LCDPROTO") == 0) {
        config->LCDProto = parseByte(params[1], config->LCDProto);
    }
    else if (strcmp(params[0], "XONLINEDASHREDIR") == 0) {
        config->XonlineDashRedir = parseBoolean(params[1], config->XonlineDashRedir);
    }
    else if (strcmp(params[0], "ADVCPUSUPPORT") == 0) {
        config->AdvCPUSupport = parseBoolean(params[1], config->AdvCPUSupport);
    }
    else if (strcmp(params[0], "DISABLELIMITMEM") == 0) {
        config->DisableLimitMem = parseBoolean(params[1], config->DisableLimitMem);
    }
    else if (strcmp(params[0], "APPLYTITLEPATCHES") == 0) {
        config->ApplyTitlePatches = parseBoolean(params[1], config->ApplyTitlePatches);
    }
    else if (strcmp(params[0], "READONLYC") == 0) {
        config->ReadOnlyC = parseBoolean(params[1], config->ReadOnlyC);
    }
    else if (strcmp(params[0], "TUDATAREDIR") == 0) {
        config->TUDATARedir = parseBoolean(params[1], config->TUDATARedir);
    }
    else if (strcmp(params[0], "TUDATAREDIRHDD") == 0) {
        config->TUDATARedirHDD = parseByte(params[1], config->TUDATARedirHDD);
    }
    else if (strcmp(params[0], "TUDATAREDIRPART") == 0) {
        config->TUDATARedirPart = parseByte(params[1], config->TUDATARedirPart);
    }
    else if (strcmp(params[0], "ENABLESCREENSHOTS") == 0) {
        config->EnableScreenshots = parseBoolean(params[1], config->EnableScreenshots);
    }
    else if (strcmp(params[0], "SCREENSHOTHDD") == 0) {
        config->ScreenshotHDD = parseByte(params[1], config->ScreenshotHDD);
    }
    else if (strcmp(params[0], "SCREENSHOTPART") == 0) {
        config->ScreenshotPart = parseByte(params[1], config->ScreenshotPart);
    }
    else if (strcmp(params[0], "IGRMASTERPORT") == 0) {
        config->IGRMasterPort = parseByte(params[1], config->IGRMasterPort);
        config->IGRMasterPort = min(max(config->IGRMasterPort, 0), 4);
    }
    else if (strcmp(params[0], "IGRDASH") == 0) {
        config->IGRDash = parseHexU16(params[1], config->IGRDash);
    }
    else if (strcmp(params[0], "IGRGAME") == 0) {
        config->IGRGame = parseHexU16(params[1], config->IGRGame);
    }
    else if (strcmp(params[0], "IGRFULL") == 0) {
        config->IGRFull = parseHexU16(params[1], config->IGRFull);
    }
    else if (strcmp(params[0], "IGRCYCLE") == 0) {
        config->IGRCycle = parseHexU16(params[1], config->IGRCycle);
    }
    else if (strcmp(params[0], "IGRSHUTDOWN") == 0) {
        config->IGRShutdown = parseHexU16(params[1], config->IGRShutdown);
    }
    else if (strcmp(params[0], "IGRSCREEN") == 0) {
        config->IGRScreen = parseHexU16(params[1], config->IGRScreen);
    }
}

void cerbiosIniHelper::parseConfig(cerbiosConfig* config, utils::dataContainer* configData)
{
    char* lineBuffer = (char*)malloc(1024);
    char* param1 = (char*)lineBuffer + 512;
    char* param2 = (char*)param1 + 256;
    bool skip = FALSE;

    uint32_t lineLength = 0;
    for (uint32_t i = 0; i < configData->size; i++)
    {
        char currentChar = configData->data[i];
        if (currentChar == '\t')
        {
            continue;
        }
        else if (currentChar == ';')
        {
            skip = true;
        }
        else if (currentChar == '\r' || currentChar == '\n')
        {
            if (lineLength > 0)
            {
                parseConfigLine(config, param1, param2, lineBuffer, lineLength);
                lineLength = 0;
            }
            skip = false;
            continue;
        }
        else if (lineLength < 512 && skip == FALSE)
        {
            lineBuffer[lineLength] = currentChar;
            lineLength++;
        }
    }

    if (lineLength > 0)
    {
        parseConfigLine(config, param1, param2, lineBuffer, lineLength);
    }

    free(lineBuffer);
}

cerbiosConfig cerbiosIniHelper::loadConfig(const char* path, cerbiosVersion version)
{
	// Init default config

	cerbiosConfig config;
	memset(&config, 0, sizeof(config));
	setConfigDefault(&config, version);

	uint32_t fileHandle;
	if (fileSystem::fileOpen(path, fileSystem::FileModeRead, fileHandle))
	{
		uint32_t fileSize;
		if (fileSystem::fileSize(fileHandle, fileSize))
		{
			char* buffer = (char*)malloc(fileSize);
			uint32_t bytesRead;
			if (fileSystem::fileRead(fileHandle, buffer, fileSize, bytesRead))
			{
				utils::dataContainer configData(buffer, fileSize, fileSize);
				parseConfig(&config, &configData);
			}
			free(buffer);
		}
		fileSystem::fileClose(fileHandle);
	}

	// Post-load normalization

	config.FanSpeed = min(((config.FanSpeed / 10) * 10), 100);

	for (int i = 0; i < 4; i++)
	{
		char value = config.FrontLed[i];
		if (value != 'G' && value != 'R' && value != 'A' && value != 'O')
		{
			config.FrontLed[i] = 'G';
		}
	}

	return config;
}

static void appendLine(char* buffer, const char* text)
{
    strcat(buffer, text);
    strcat(buffer, "\r\n");
}

static void appendKVBool(char* buffer, const char* key, unsigned char value)
{
    strcat(buffer, key);
    strcat(buffer, " = ");
    strcat(buffer, value == 1 ? "True" : "False");
    strcat(buffer, "\r\n");
}

static void appendKVByte(char* buffer, const char* key, unsigned char value)
{
    char* tmp = stringUtility::formatString("%i", value);
    strcat(buffer, key);
    strcat(buffer, " = ");
    strcat(buffer, tmp);
    strcat(buffer, "\r\n");
    free(tmp);
}

static void appendKVHex24(char* buffer, const char* key, uint32_t value)
{
    char* tmp = stringUtility::formatString("0x%06X", value & 0x00FFFFFF);
    strcat(buffer, key);
    strcat(buffer, " = ");
    strcat(buffer, tmp);
    strcat(buffer, "\r\n");
    free(tmp);
}

static void appendKVHexU16(char* buffer, const char* key, uint16_t value)
{
    char* tmp = stringUtility::formatString("%X", value);
    strcat(buffer, key);
    strcat(buffer, " = ");
    strcat(buffer, tmp);
    strcat(buffer, "\r\n");
    free(tmp);
}

static void appendKVHexU8(char* buffer, const char* key, unsigned char value)
{
    char* tmp = stringUtility::formatString("0x%02X", value);
    strcat(buffer, key);
    strcat(buffer, " = ");
    strcat(buffer, tmp);
    strcat(buffer, "\r\n");
    free(tmp);
}

static void appendKVString(char* buffer, const char* key, const char* value)
{
    strcat(buffer, key);
    strcat(buffer, " = ");
    strcat(buffer, value);
    strcat(buffer, "\r\n");
}

static void buildConfigLegacy(cerbiosConfig* config, char* buffer)
{
	strcat(buffer, "; Cerbios Config\r\n");
	strcat(buffer, "\r\n");

	strcat(buffer, "; Check For AV Pack\r\n");
	appendKVBool(buffer, "AVCheck", config->AVCheck);
	strcat(buffer, "\r\n");

	strcat(buffer, "; LED Ring Color, G = Green, R = Red, A = Amber, O = Off\r\n");
	appendKVString(buffer, "FrontLed", config->FrontLed);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Fan Speed 0 = Default, 10-100 = Manual Control, Supports increments of 2's\r\n");
	appendKVByte(buffer, "FanSpeed", config->FanSpeed);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Drive Setup\r\n");
	strcat(buffer, "; 0 = HDD & DVD,  1 = HDD & No DVD (Legacy Mode), 2 = HDD & No DVD (Modern Mode), 3 = Dual HDD\r\n");
	appendKVByte(buffer, "DriveSetup", config->DriveSetup);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Set Master Drive UDMA mode 0-6 on cold-boot\r\n");
	appendKVByte(buffer, "UdmaMode", config->UdmaModeMaster);
	appendKVByte(buffer, "UdmaModeMaster", config->UdmaModeMaster);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Set Slave Drive UDMA mode 0-6 on cold-boot (if enabled by DriveSetup = 3)\r\n");
	appendKVByte(buffer, "UdmaModeSlave", config->UdmaModeSlave);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Enables Automatic Time Sync With Optional RTC Hardware Connected to SMBus\r\n");
	appendKVBool(buffer, "RtcEnable", config->RtcEnable);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Forces AV Modes That Would Normally Be Rendered At 480i to 480p. Requires 480p Set In MS Dash And Component Cables\r\n");
	appendKVBool(buffer, "Force480p", config->Force480p);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Forces VGA Modes For Component Cables Or Custom VGA Cables Using Mode(2+3) for VGA Displays Only, This Enables Force480p By Default & Sets Console To NTSC.\r\n");
	appendKVBool(buffer, "ForceVGA", config->ForceVGA);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Load XDK Launcher/XBDM if it exists (Debug Bios Only)\r\n");
	appendKVBool(buffer, "Debug", config->Debug);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Blocks Games From Updating The Original Xbox Dashboard, Useful for softmods.\r\n");
	appendKVBool(buffer, "BlockDashUpdate", config->BlockDashUpdate);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Restore original stock xbox reset functionality when the DVD tray is ejected.\r\n");
	appendKVBool(buffer, "ResetOnEject", config->ResetOnEject);
	strcat(buffer, "\r\n");

	strcat(buffer, "; CD Paths (always falls back to D:\\default.xbe)\r\n");
	appendKVString(buffer, "CdPath1", config->CdPath1);
	appendKVString(buffer, "CdPath2", config->CdPath2);
	appendKVString(buffer, "CdPath3", config->CdPath3);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Dash Paths (always falls back to C:\\xboxdash.xbe)\r\n");
	appendKVString(buffer, "DashPath1", config->DashPath1);
	appendKVString(buffer, "DashPath2", config->DashPath2);
	appendKVString(buffer, "DashPath3", config->DashPath3);
	strcat(buffer, "\r\n");

	strcat(buffer, "; Boot Animation Path (always falls back to C:\\BootAnims\\Xbox\\bootanim.xbe)\r\n");
	appendKVString(buffer, "BootAnimPath", config->BootAnimPath);
}

static void buildConfigCurrent(cerbiosConfig* config, char* buffer)
{
	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Launch Settings");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Sets The Primary Dashboard XBE To Load On Boot. Supports HDD0-/HDD1- Prefixes. Falls Back To Recovery E:\\Cerbios\\Recovery\\default.xbe Then C:\\xboxdash.xbe If Missing.");
	appendKVString(buffer, "DashPath", config->DashPath);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Plays A Custom Boot Animation XBE During Startup. Falls Back To E:\\Cerbios\\BootAnims\\Xbox\\bootanim.xbe If Not Set.");
	appendKVString(buffer, "BootAnimPath", config->BootAnimPath);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Video & Display Settings");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Performs AV Cable Check On Boot. Halts Boot With Flashing Green And Amber LEDs If No Cable Detected.");
	appendKVBool(buffer, "AVCheck", config->AVCheck);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Manually Sets Flicker Filter Level. 0 = Off, 1-5 = Filter Strength, 6 = Use System/Game Default.");
	appendKVByte(buffer, "ForceFFilter", config->ForceFFilter);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Forces 480i Games And Apps To Render In 480p. Requires Component Cables And 480p Enabled In MS Dashboard. PAL Modes Unaffected.");
	appendKVBool(buffer, "Force480p", config->Force480p);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Forces VGA Output Using RGsB Signal. Requires VGA Or Component Cable And Sync-On-Green Compatible Display. Automatically Enables Force480p And NTSC Mode. Not Compatible With 1.6 Consoles.");
	appendKVBool(buffer, "ForceVGA", config->ForceVGA);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Storage & Drive Config");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Defines Console Drive Setup: 0 = HDD+DVD (Stock), 1 = HDD Only (Legacy Mode), 2 = HDD Only (Modern Mode), 3 = Dual HDD With Slave In DVD Bay.");
	appendKVByte(buffer, "DriveSetup", config->DriveSetup);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Sets UDMA Mode For Master HDD On Cold Boot. 0 = Auto (Startech), 1 = Auto (Generic), 2-6 = UDMA Modes (Higher = Faster, Requires 80-Wire IDE Cable).");
	appendKVByte(buffer, "UdmaModeMaster", config->UdmaModeMaster);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Sets UDMA Mode For Slave HDD In Dual HDD Setup. 0 = Auto (Startech), 1 = Auto (Generic), 2-6 = UDMA Modes (Higher = Faster, Requires 80-Wire IDE Cable And Jumper).");
	appendKVByte(buffer, "UdmaModeSlave", config->UdmaModeSlave);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Cooling & Performance");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Custom Fan Speed (%) Set During Boot. 0 = Auto, 10-100 = Manual Control In Steps Of 2.");
	appendKVByte(buffer, "FanSpeed", config->FanSpeed);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Allows Cerbios To Control Fan Speed On Boot, Reboot, And App Launch. Disable To Let Apps / Dash Override Fan Speed Instead.");
	appendKVBool(buffer, "OverrideFan", config->OverrideFan);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Overclocking");
	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Warning: This Is For Advanced Users Only As Setting The Incorrect Values Can Lead To Instability Or Damage.");
	appendLine(buffer, "; Team Cerbios Is Not Responsible For Misuse Of This Feature. See Cerbios documentation for FSB/PLL/VCO details.");
	strcat(buffer, "\r\n");

	appendKVBool(buffer, "Overclocking", config->Overclocking);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; CPUMPLLCoeff (hex): Sets The CPU Overclock (Stock: 0x230801) Or 0x0");
	appendKVHex24(buffer, "CPUMPLLCoeff", config->CPUMPLLCoeff);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; NVPLLCoeff (hex): Sets The GPU Overclock (Stock: 0x011C01) Or 0x0");
	appendKVHex24(buffer, "NVPLLCoeff", config->NVPLLCoeff);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Front Panel & Visuals");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Configures Front Panel LED Ring Pattern. Uses 4-Character Code: G = Green, R = Red, A = Amber, O = Off. Cycles Pattern Until Overridden.");
	appendKVString(buffer, "FrontLed", config->FrontLed);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables In-Game LCD Display For FPS, Temps, RAM Usage, And More.");
	appendKVBool(buffer, "InAppLCDEnable", config->InAppLCDEnable);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Sets The LCD Bus. 0 = Xbox's SMBus, 1 = X3LCD Via X3 Modchip.");
	appendKVByte(buffer, "LCDBus", config->LCDBus);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Sets I2C Address For LCD Display. Valid Range: 0x20-0x27 Or 0x38-0x3F Depending On Module Type.");
	appendKVHexU8(buffer, "LCDI2CAddr", config->LCDI2CAddr);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Sets Protocol For LCD. 0 = HD44780-Compatible (LCD2004) Or X3LCD, 1 = US2066 (NHD-0420CW).");
	appendKVByte(buffer, "LCDProto", config->LCDProto);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Enhancements");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Blocks Games From Updating The MS Dashboard. Useful For Softmod Installs Or When Running Cerbios In BFM Mode.");
	appendKVBool(buffer, "BlockDashUpdate", config->BlockDashUpdate);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Redirects Xbox Live Dashboard Calls To Xonlinedash_Original.xbe. Useful For Softmod Setups That Use A Dummy Xonlinedash.xbe To Prevent Disc Boot Updates.");
	appendKVBool(buffer, "XonlineDashRedir", config->XonlineDashRedir);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables Runtime XBE Patching For CPU-Upgraded Or Overclocked Consoles. Has No Effect On Stock CPUs.");
	appendKVBool(buffer, "AdvCPUSupport", config->AdvCPUSupport);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Ignores /LIMITMEM Flag In XBE Files To Allow Access To Full 128MB RAM. Only Use On Consoles With 128MB RAM. May Cause Compatibility Issues Or Crashes.");
	appendKVBool(buffer, "DisableLimitMem", config->DisableLimitMem);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables Compatibility Fixes For Specific Games By Applying XBE Patches At Runtime. Useful For Titles With Known Issues.");
	appendKVBool(buffer, "ApplyTitlePatches", config->ApplyTitlePatches);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Marks C Partition As Read-Only. Prevents Writes To The System Partition While Cerbios Is Running. Useful For Softmod Installs.");
	appendKVBool(buffer, "ReadOnlyC", config->ReadOnlyC);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Restores Original Xbox Behavior Of Resetting When Tray Is Ejected. Reloads Dashboard Automatically.");
	appendKVBool(buffer, "ResetOnEject", config->ResetOnEject);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables Automatic Time Sync Using RTC Module Connected To SMBus. Requires Cerbios 2.4+ Or PrometheOS 1.4+.");
	appendKVBool(buffer, "RTCEnable", config->RtcEnable);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables UDATA/TDATA Redirection To Alternate Drive/Partition.");
	appendKVBool(buffer, "TUDATARedir", config->TUDATARedir);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Selects Which HDD To Use For Redirection. 0 = Master, 1 = Slave (Dual HDD).");
	appendKVByte(buffer, "TUDATARedirHDD", config->TUDATARedirHDD);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Selects Which Partition To Use For UDATA/TDATA:, 1 = E:, 6 = F:, 7 = G:");
	appendKVByte(buffer, "TUDATARedirPart", config->TUDATARedirPart);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables Screenshot Capture Of The Running XBEs Framebuffer. Saved To E:\\Cerbios\\Screenshots\\(Title ID) When L-Thumb + R-Thumb Combo Is Pressed.");
	appendKVBool(buffer, "EnableScreenshots", config->EnableScreenshots);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Selects Which HDD To Use For Screenshots. 0 = Master, 1 = Slave (Dual HDD).");
	appendKVByte(buffer, "ScreenshotHDD", config->ScreenshotHDD);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Selects Which Partition To Use For Screenshots:, 1 = E:, 6 = F:, 7 = G:");
	appendKVByte(buffer, "ScreenshotPart", config->ScreenshotPart);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; In-Game Key Combos");
	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Button hex codes: A=0 B=1 X=2 Y=3 BLACK=4 WHITE=5 LT=6 RT=7 DUP=8 DDN=9 DL=A DR=B START=C BACK=D LT-STK=E RT-STK=F");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Select Which Controller Can Trigger IGR Combos. 0 = ALL, 1-4 = Controller Ports.");
	appendKVByte(buffer, "IGRMasterPort", config->IGRMasterPort);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Soft Reset Back To Dashboard. Keeps Any Mounted ISO/CCI Or Game Active.");
	appendKVHexU16(buffer, "IGRDash", config->IGRDash);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Reloads The Currently Running Game Or Application.");
	appendKVHexU16(buffer, "IGRGame", config->IGRGame);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Full Kernel Reset. Unmounts Any Loaded ISO/CCI Or Game And Returns To Dashboard.");
	appendKVHexU16(buffer, "IGRFull", config->IGRFull);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Performs A Full Power Cycle. Reboots The Console Completely.");
	appendKVHexU16(buffer, "IGRCycle", config->IGRCycle);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Shuts Down The Console From In-Game.");
	appendKVHexU16(buffer, "IGRShutdown", config->IGRShutdown);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Captures A Screenshot Of The Current Framebuffer. Requires EnableScreenshots To Be Set To True.");
	appendKVHexU16(buffer, "IGRScreen", config->IGRScreen);
	strcat(buffer, "\r\n");

	appendLine(buffer, "; ========================");
	appendLine(buffer, "; Debugging & Development");
	appendLine(buffer, "; ========================");
	strcat(buffer, "\r\n");

	appendLine(buffer, "; Enables Debug Mode. Boots C:\\xshell.xbe And Loads E:\\xbdm.dll To Allow Visual Studio Debugging Over Network.");
	appendKVBool(buffer, "Debug", config->Debug);
}

void cerbiosIniHelper::buildConfig(cerbiosConfig* config, cerbiosVersion version, char* buffer)
{
	buffer[0] = 0;

	if (version == CerbiosVersionCurrent)
	{
		buildConfigCurrent(config, buffer);
	}
	else
	{
		buildConfigLegacy(config, buffer);
	}
}

void cerbiosIniHelper::saveConfig(const char* path, char* buffer)
{
	const char* lastSlash = strrchr(path, '\\');
	if (lastSlash != NULL && lastSlash != path)
	{
		uint32_t parentLen = (uint32_t)(lastSlash - path);
		char* parentPath = (char*)malloc(parentLen + 1);
		strncpy(parentPath, path, parentLen);
		parentPath[parentLen] = 0;
		fileSystem::directoryCreate(parentPath);
		free(parentPath);
	}

	uint32_t fileHandle;
	if (fileSystem::fileOpen(path, fileSystem::FileModeWrite, fileHandle))
	{
		uint32_t bytesWritten = 0;
		fileSystem::fileWrite(fileHandle, buffer, strlen(buffer), bytesWritten);
		fileSystem::fileClose(fileHandle);
	}
}

static void setConfigDefaultLegacy(cerbiosConfig* config)
{
	config->DriveSetup = 1;
	config->AVCheck = 1;
	config->Debug = 0;
	strcpy(config->CdPath1, "");
	strcpy(config->CdPath2, "");
	strcpy(config->CdPath3, "");
	strcpy(config->DashPath1, "\\Device\\Harddisk0\\Partition2\\evoxdash.xbe");
	strcpy(config->DashPath2, "\\Device\\Harddisk0\\Partition2\\avalaunch.xbe");
	strcpy(config->DashPath3, "\\Device\\Harddisk0\\Partition2\\nexgen.xbe");
	strcpy(config->BootAnimPath, "\\Device\\Harddisk0\\Partition2\\BootAnims\\Xbox\\bootanim.xbe");
	strcpy(config->FrontLed, "GGGG");
	config->FanSpeed = 0;
	config->UdmaModeMaster = 2;
	config->UdmaModeSlave = 2;
	config->Force480p = 0;
	config->ForceVGA = 0;
	config->RtcEnable = 0;
	config->BlockDashUpdate = 0;
	config->ResetOnEject = 0;
}

static void setConfigDefaultCurrent(cerbiosConfig* config)
{
	// Mirrors cerbios3.0.0.ini exactly
	strcpy(config->DashPath, "HDD0-C:\\evoxdash.xbe");
	strcpy(config->BootAnimPath, "HDD0-E:\\Cerbios\\BootAnims\\Xbox\\bootanim.xbe");
	config->AVCheck = 1;
	config->ForceFFilter = 6;
	config->Force480p = 0;
	config->ForceVGA = 0;
	config->DriveSetup = 1;
	config->UdmaModeMaster = 2;
	config->UdmaModeSlave = 2;
	config->FanSpeed = 0;
	config->OverrideFan = 1;
	config->Overclocking = 0;
	config->CPUMPLLCoeff = 0x000000;
	config->NVPLLCoeff = 0x000000;
	strcpy(config->FrontLed, "GGGG");
	config->InAppLCDEnable = 0;
	config->LCDBus = 0;
	config->LCDI2CAddr = 0x3C;
	config->LCDProto = 0;
	config->BlockDashUpdate = 0;
	config->XonlineDashRedir = 0;
	config->AdvCPUSupport = 1;
	config->DisableLimitMem = 0;
	config->ApplyTitlePatches = 1;
	config->ReadOnlyC = 0;
	config->ResetOnEject = 0;
	config->RtcEnable = 0;
	config->TUDATARedir = 0;
	config->TUDATARedirHDD = 0;
	config->TUDATARedirPart = 1;
	config->EnableScreenshots = 0;
	config->ScreenshotHDD = 0;
	config->ScreenshotPart = 1;
	config->IGRMasterPort = 0;
	config->IGRDash = 0x67CD;
	config->IGRGame = 0x467C;
	config->IGRFull = 0x467D;
	config->IGRCycle = 0x4678;
	config->IGRShutdown = 0x678D;
	config->IGRScreen = 0xEF;
	config->Debug = 0;
}

void cerbiosIniHelper::setConfigDefault(cerbiosConfig* config, cerbiosVersion version)
{
	memset(config, 0, sizeof(cerbiosConfig));
	if (version == CerbiosVersionCurrent)
	{
		setConfigDefaultCurrent(config);
	}
	else
	{
		setConfigDefaultLegacy(config);
	}
}
