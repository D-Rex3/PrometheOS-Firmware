#include "hexSelectorScene.h"
#include "sceneManager.h"

#include "..\context.h"
#include "..\drawing.h"
#include "..\component.h"
#include "..\inputManager.h"
#include "..\stringUtility.h"
#include "..\xboxInternals.h"
#include "..\theme.h"

static char hexCharFor(uint8_t nibble)
{
	return (char)(nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10));
}

static uint8_t nibbleFor(char hex)
{
	if (hex >= '0' && hex <= '9') return hex - '0';
	if (hex >= 'A' && hex <= 'F') return hex - 'A' + 10;
	if (hex >= 'a' && hex <= 'f') return hex - 'a' + 10;
	return 0;
}

hexSelectorScene::hexSelectorScene(const char* title, uint32_t value, uint8_t digitCount, bool showPrefix)
{
	strncpy(mTitle, title, 63);
	mTitle[63] = 0;

	if (digitCount > 6) digitCount = 6;
	if (digitCount < 1) digitCount = 1;
	mDigitCount = digitCount;
	mShowPrefix = showPrefix;
	mSelectedDigit = 0;

	for (int i = 0; i < 6; i++) mDigits[i] = '0';

	// Most-significant digit at mDigits[0], least at mDigits[mDigitCount-1]
	for (int i = 0; i < mDigitCount; i++)
	{
		uint8_t shift = (uint8_t)((mDigitCount - 1 - i) * 4);
		mDigits[i] = hexCharFor((uint8_t)((value >> shift) & 0xF));
	}
}

void hexSelectorScene::update()
{
	if (inputManager::buttonPressed(ButtonB))
	{
		sceneManager::popScene(sceneResultDone);
		return;
	}

	if (inputManager::buttonPressed(ButtonDpadLeft))
	{
		mSelectedDigit = mSelectedDigit > 0 ? mSelectedDigit - 1 : (mDigitCount - 1);
	}

	if (inputManager::buttonPressed(ButtonDpadRight))
	{
		mSelectedDigit = mSelectedDigit < (mDigitCount - 1) ? mSelectedDigit + 1 : 0;
	}

	if (inputManager::buttonPressed(ButtonDpadUp))
	{
		uint8_t n = nibbleFor(mDigits[mSelectedDigit]);
		n = (uint8_t)((n + 1) & 0xF);
		mDigits[mSelectedDigit] = hexCharFor(n);
	}

	if (inputManager::buttonPressed(ButtonDpadDown))
	{
		uint8_t n = nibbleFor(mDigits[mSelectedDigit]);
		n = (uint8_t)((n + 15) & 0xF);
		mDigits[mSelectedDigit] = hexCharFor(n);
	}
}

void hexSelectorScene::render()
{
	component::panel(theme::getPanelFillColor(), theme::getPanelStrokeColor(), 16, 16, 688, 448);
	drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), mTitle, theme::getHeaderTextColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);

	const int cellW = 48;
	const int cellH = 64;
	const int cellGap = 8;
	const int prefixW = mShowPrefix ? 56 : 0;

	int rowW = prefixW + (mDigitCount * cellW) + ((mDigitCount - 1) * cellGap);
	int startX = (context::getBufferWidth() - rowW) / 2;
	int startY = (context::getBufferHeight() - cellH) / 2;
	startY += theme::getCenterOffset();

	if (mShowPrefix)
	{
		component::text("0x", false, horizAlignmentLeft, startX, startY, prefixW, cellH);
	}

	for (int i = 0; i < mDigitCount; i++)
	{
		char label[2] = { mDigits[i], 0 };
		int x = startX + prefixW + (i * (cellW + cellGap));
		component::button(mSelectedDigit == i, false, label, x, startY, cellW, cellH);
	}

	drawing::drawBitmapString(context::getBitmapFontSmall(), "\xC2\xB0\xC2\xB1 Adjust  \xC2\xB2\xC2\xB3 Move", theme::getFooterTextColor(), 40, theme::getFooterY());
	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "\xC2\xA2 Back", theme::getFooterTextColor(), horizAlignmentRight, 40, theme::getFooterY(), 640);
}

uint32_t hexSelectorScene::getValue()
{
	uint32_t value = 0;
	for (int i = 0; i < mDigitCount; i++)
	{
		value = (value << 4) | nibbleFor(mDigits[i]);
	}
	return value;
}
