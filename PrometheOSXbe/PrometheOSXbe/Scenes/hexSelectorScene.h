#pragma once

#include "scene.h"

#include "..\xboxInternals.h"

class hexSelectorScene : public scene
{
public:
	hexSelectorScene(const char* title, uint32_t value, uint8_t digitCount, bool showPrefix);
	void update();
	void render();
	uint32_t getValue();
private:
	char mTitle[64];
	char mDigits[6];
	uint8_t mDigitCount;
	bool mShowPrefix;
	int mSelectedDigit;
};
