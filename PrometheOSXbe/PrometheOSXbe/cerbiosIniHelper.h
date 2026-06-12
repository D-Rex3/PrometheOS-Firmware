#pragma once

#include "utils.h"
#include "xboxInternals.h"

typedef enum cerbiosVersion
{
    CerbiosVersionLegacy,   // Cerbios 2.4.2 and below (C:\cerbios.ini)
    CerbiosVersionCurrent   // Cerbios 3.0.0+ (E:\Cerbios\cerbios.ini)
} cerbiosVersion;

typedef struct cerbiosConfig
{
    // Shared (both versions)
    unsigned char AVCheck;
    unsigned char Debug;
    unsigned char DriveSetup;
    char BootAnimPath[100];
    char FrontLed[5];
    unsigned char FanSpeed;
    unsigned char UdmaModeMaster;
    unsigned char UdmaModeSlave;
    unsigned char Force480p;
    unsigned char ForceVGA;
    unsigned char RtcEnable;
    unsigned char BlockDashUpdate;
    unsigned char ResetOnEject;

    // Legacy (2.4.2) only
    char CdPath1[100];
    char CdPath2[100];
    char CdPath3[100];
    char DashPath1[100];
    char DashPath2[100];
    char DashPath3[100];

    // Current (3.0.0+) only
    char DashPath[100];
    unsigned char ForceFFilter;
    unsigned char OverrideFan;
    unsigned char Overclocking;
    uint32_t CPUMPLLCoeff;
    uint32_t NVPLLCoeff;
    unsigned char InAppLCDEnable;
    unsigned char LCDBus;
    unsigned char LCDI2CAddr;
    unsigned char LCDProto;
    unsigned char XonlineDashRedir;
    unsigned char AdvCPUSupport;
    unsigned char DisableLimitMem;
    unsigned char ApplyTitlePatches;
    unsigned char ReadOnlyC;
    unsigned char TUDATARedir;
    unsigned char TUDATARedirHDD;
    unsigned char TUDATARedirPart;
    unsigned char EnableScreenshots;
    unsigned char ScreenshotHDD;
    unsigned char ScreenshotPart;
    unsigned char IGRMasterPort;
    uint16_t IGRDash;
    uint16_t IGRGame;
    uint16_t IGRFull;
    uint16_t IGRCycle;
    uint16_t IGRShutdown;
    uint16_t IGRScreen;
} cerbiosConfig;

class cerbiosIniHelper
{
public:
	static cerbiosConfig loadConfig(const char* path, cerbiosVersion version);
	static void buildConfig(cerbiosConfig* config, cerbiosVersion version, char* buffer);
	static void saveConfig(const char* path, char* buffer);
	static void setConfigDefault(cerbiosConfig* config, cerbiosVersion version);
private:
	static void upperCase(char* value);
	static void trimSpace(char* value);
	static uint8_t parseByte(char* value, uint8_t defaultValue);
	static uint8_t parseBoolean(char* value, uint8_t defaultValue);
	static uint32_t parseHex(char* value, uint32_t defaultValue);
	static uint16_t parseHexU16(char* value, uint16_t defaultValue);
	static void parseConfigLine(cerbiosConfig* config, char* param1, char* param2, char* buffer, unsigned long bufferSize);
	static void parseConfig(cerbiosConfig* config, utils::dataContainer* configData);
};
