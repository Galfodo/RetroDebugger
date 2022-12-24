#include "CViewC64AllGraphicsSpritesControl.h"
#include "CViewC64AllGraphics.h"
#include "VID_Main.h"
#include "VID_ImageBinding.h"
#include "CDebugInterfaceC64.h"
#include "CViewC64VicDisplay.h"
#include "CViewC64VicControl.h"
#include "CViewDataDump.h"
#include "CViewMemoryMap.h"
#include "C64Tools.h"
#include "CGuiMain.h"
#include "CViewDataDump.h"
#include "CGuiLockableList.h"
#include "CSlrString.h"
#include "CViewC64StateVIC.h"
#include "CViewC64.h"

// TODO: colour leds are just rectangles with hitbox and lot of copy pasted code, replace them to proper buttons!

CViewC64AllGraphicsSpritesControl::CViewC64AllGraphicsSpritesControl(const char *name, float posX, float posY, float posZ, float sizeX, float sizeY, CDebugInterfaceC64 *debugInterface)
: CGuiView(name, posX, posY, posZ, sizeX, sizeY)
{
	this->debugInterface = debugInterface;
	
	imGuiNoWindowPadding = true;
	imGuiNoScrollbar = true;

	this->consumeTapBackground = false;
	this->allowFocus = false;
	
	forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_NONE;

	//
	font = viewC64->fontCBMShifted;
	fontScale = 1.5f;
	fontHeight = font->GetCharHeight('@', fontScale) + 2;
	fontSize = fontHeight;

	float startX = 5;
	float startY = 5;
	
	float px = startX;
	float py = startY;
	float buttonSizeX = 5.96 * fontSize;
	float buttonSizeY = 2.0f * fontSize;
	float gap = 5.0f;
	float mgap = 2.5f;
	float buttonTextOffsetY = 0.67*fontSize;

	btnModeBitmapColorsBlackWhite = new CGuiButtonSwitch(NULL, NULL, NULL,
										   px, py, posZ, buttonSizeX, buttonSizeY,
										   new CSlrString("B/W"),
										   FONT_ALIGN_CENTER, buttonSizeX/2, buttonTextOffsetY,
										   font, fontScale,
										   1.0, 1.0, 1.0, 1.0,
										   1.0, 1.0, 1.0, 1.0,
										   0.3, 0.3, 0.3, 1.0,
										   this);
	btnModeBitmapColorsBlackWhite->SetOn(false);
	SetSwitchButtonDefaultColors(btnModeBitmapColorsBlackWhite);
	this->AddGuiElement(btnModeBitmapColorsBlackWhite);

	px += buttonSizeX + gap;

	btnModeHires = new CGuiButtonSwitch(NULL, NULL, NULL,
														px, py, posZ, buttonSizeX, buttonSizeY,
														new CSlrString("HIRES"),
														FONT_ALIGN_CENTER, buttonSizeX/2, buttonTextOffsetY,
														font, fontScale,
														1.0, 1.0, 1.0, 1.0,
														1.0, 1.0, 1.0, 1.0,
														0.3, 0.3, 0.3, 1.0,
														this);
	btnModeHires->SetOn(false);
	SetSwitchButtonDefaultColors(btnModeHires);
	this->AddGuiElement(btnModeHires);

	px += buttonSizeX + gap;
	
	btnModeMulti = new CGuiButtonSwitch(NULL, NULL, NULL,
										px, py, posZ, buttonSizeX, buttonSizeY,
										new CSlrString("MULTI"),
										FONT_ALIGN_CENTER, buttonSizeX/2, buttonTextOffsetY,
										font, fontScale,
										1.0, 1.0, 1.0, 1.0,
										1.0, 1.0, 1.0, 1.0,
										0.3, 0.3, 0.3, 1.0,
										this);
	btnModeMulti->SetOn(false);
	SetSwitchButtonDefaultColors(btnModeMulti);
	this->AddGuiElement(btnModeMulti);
	
	//
	px = startX;
	py += buttonSizeY + gap;
	
	btnShowRAMorIO = new CGuiButtonSwitch(NULL, NULL, NULL,
												  px, py, posZ, buttonSizeX, buttonSizeY,
												  new CSlrString("ROM"),
												  FONT_ALIGN_CENTER, buttonSizeX/2, buttonTextOffsetY,
												  font, fontScale,
												  1.0, 1.0, 1.0, 1.0,
												  1.0, 1.0, 1.0, 1.0,
												  0.3, 0.3, 0.3, 1.0,
												  this);
	btnShowRAMorIO->SetOn(true);
	SetSwitchButtonDefaultColors(btnShowRAMorIO);
	this->AddGuiElement(btnShowRAMorIO);

	px += buttonSizeX + gap;

	btnShowGrid = new CGuiButtonSwitch(NULL, NULL, NULL,
										  px, py, posZ, buttonSizeX, buttonSizeY,
										  new CSlrString("GRID"),
										  FONT_ALIGN_CENTER, buttonSizeX/2, buttonTextOffsetY,
										  font, fontScale,
										  1.0, 1.0, 1.0, 1.0,
										  1.0, 1.0, 1.0, 1.0,
										  0.3, 0.3, 0.3, 1.0,
										  this);
	btnShowGrid->SetOn(true);
	SetSwitchButtonDefaultColors(btnShowGrid);
	this->AddGuiElement(btnShowGrid);

	// TODO: show grid
	btnShowGrid->SetVisible(false);

	SetupMode();
}

CViewC64AllGraphicsSpritesControl::~CViewC64AllGraphicsSpritesControl()
{
}

void CViewC64AllGraphicsSpritesControl::SetSwitchButtonDefaultColors(CGuiButtonSwitch *btn)
{
	btn->buttonSwitchOffColorR = 0.0f;
	btn->buttonSwitchOffColorG = 0.0f;
	btn->buttonSwitchOffColorB = 0.0f;
	btn->buttonSwitchOffColorA = 1.0f;
	
	btn->buttonSwitchOffColor2R = 0.3f;
	btn->buttonSwitchOffColor2G = 0.3f;
	btn->buttonSwitchOffColor2B = 0.3f;
	btn->buttonSwitchOffColor2A = 1.0f;
	
	btn->buttonSwitchOnColorR = 0.0f;
	btn->buttonSwitchOnColorG = 0.7f;
	btn->buttonSwitchOnColorB = 0.0f;
	btn->buttonSwitchOnColorA = 1.0f;
	
	btn->buttonSwitchOnColor2R = 0.3f;
	btn->buttonSwitchOnColor2G = 0.3f;
	btn->buttonSwitchOnColor2B = 0.3f;
	btn->buttonSwitchOnColor2A = 1.0f;
	
}

void CViewC64AllGraphicsSpritesControl::SetLockableButtonDefaultColors(CGuiButtonSwitch *btn)
{
	btn->buttonSwitchOffColorR = 0.0f;
	btn->buttonSwitchOffColorG = 0.0f;
	btn->buttonSwitchOffColorB = 0.0f;
	btn->buttonSwitchOffColorA = 1.0f;
	
	btn->buttonSwitchOffColor2R = 0.3f;
	btn->buttonSwitchOffColor2G = 0.3f;
	btn->buttonSwitchOffColor2B = 0.3f;
	btn->buttonSwitchOffColor2A = 1.0f;
	
	btn->buttonSwitchOnColorR = 0.7f;
	btn->buttonSwitchOnColorG = 0.0f;
	btn->buttonSwitchOnColorB = 0.0f;
	btn->buttonSwitchOnColorA = 1.0f;
	
	btn->buttonSwitchOnColor2R = 0.3f;
	btn->buttonSwitchOnColor2G = 0.3f;
	btn->buttonSwitchOnColor2B = 0.3f;
	btn->buttonSwitchOnColor2A = 1.0f;
	
}

void CViewC64AllGraphicsSpritesControl::SetupMode()
{
	guiMain->LockMutex();

	btnModeBitmapColorsBlackWhite->SetVisible(true);
	btnModeHires->SetVisible(true);
	btnModeMulti->SetVisible(true);

	switch(forcedRenderScreenMode)
	{
		case VIEW_C64_ALL_GRAPHICS_FORCED_GRAY:
			viewC64->viewC64MemoryDataDump->renderDataWithColors = false;
			break;
		case VIEW_C64_ALL_GRAPHICS_FORCED_HIRES:
			viewC64->viewC64MemoryDataDump->renderDataWithColors = false;
			break;
		case VIEW_C64_ALL_GRAPHICS_FORCED_MULTI:
			viewC64->viewC64MemoryDataDump->renderDataWithColors = true;
			break;
	}

	guiMain->UnlockMutex();
}

void CViewC64AllGraphicsSpritesControl::DoLogic()
{
	CGuiView::DoLogic();
}

void CViewC64AllGraphicsSpritesControl::RenderImGui()
{
	PreRenderImGui();
	Render();
	PostRenderImGui();
}

void CViewC64AllGraphicsSpritesControl::Render()
{
	float ledX = posX + fontSize * 20.1725f;
	float ledY = posY + 6.0f;

	float ledSizeX = fontSize*4.0f;
	float gap = fontSize * 0.1f;
	float step = fontSize * 0.75f;
	float ledSizeY = fontSize + gap + gap;

	//
	if (forcedRenderScreenMode == VIEW_C64_ALL_GRAPHICS_FORCED_NONE)
	{
		btnModeBitmapColorsBlackWhite->SetOn(false);
		btnModeHires->SetOn(! (viewC64->viewC64MemoryDataDump->renderDataWithColors) );
		btnModeMulti->SetOn(  (viewC64->viewC64MemoryDataDump->renderDataWithColors) );
	}

	// render color leds for sprites
	if (btnModeMulti->IsOn())
	{
		float px = ledX;
		float py = ledY;

		// D021
		int i = 0x01;
		u8 color = viewC64->colorsToShow[i];
		bool isForced = (viewC64->viewC64StateVIC->forceColors[i] != -1);
		RenderColorRectangleWithHexCode(px, py, ledSizeX, ledSizeY, gap, isForced, color, fontSize, viewC64->debugInterfaceC64);
		py += fontSize + step;

		// D025-D027
		for (int i = 0x05; i < 0x08; i++)
		{
			u8 color = viewC64->colorsToShow[i];
			bool isForced = (viewC64->viewC64StateVIC->forceColors[i] != -1);
			RenderColorRectangleWithHexCode(px, py, ledSizeX, ledSizeY, gap, isForced, color, fontSize, viewC64->debugInterfaceC64);
			py += fontSize + step; //gap + ledSizeY + gap;
		}
	}

	btnShowGrid->SetOn(viewC64->viewC64VicControl->btnShowGrid->IsOn());

	CGuiView::Render();
}

void CViewC64AllGraphicsSpritesControl::Render(float posX, float posY)
{
	CGuiView::Render(posX, posY);
}

void CViewC64AllGraphicsSpritesControl::UpdateShowIOButton()
{
	btnShowRAMorIO->SetOn( ! viewC64->isDataDirectlyFromRAM);
}

//@returns is consumed
bool CViewC64AllGraphicsSpritesControl::DoTap(float x, float y)
{
	LOGG("CViewC64AllGraphicsSpritesControl::DoTap:  x=%f y=%f", x, y);
	
	guiMain->LockMutex();

	// TODO: note this is copy pasted code from C64ViewStateVIC, needs to be generalized
	//       idea is to sync values with VIC state view. Leds should be replaced with proper buttons and callbacks.
	//		 the below is just a temporary POC made in a few minutes

	float ledX = posX + fontSize * 20.1725f;
	float ledY = posY + 6.0f;

	float ledSizeX = fontSize*4.0f;
	float gap = fontSize * 0.1f;
	float step = fontSize * 0.75f;
	float ledSizeY = fontSize + gap + gap;
	float ledSizeY2 = fontSize + step;

	// render color leds for sprites
	if (btnModeMulti->IsOn())
	{
		float px = ledX;
		float py = ledY;

		if (x >= px && x <= px + ledSizeX && y >= py && y <= py + ledSizeY2)
		{
			// D021
			int i = 0x01;

			if (viewC64->viewC64StateVIC->forceColors[i] == -1)
			{
				viewC64->viewC64StateVIC->forceColors[i] = viewC64->colorsToShow[i];
			}
			else
			{
				viewC64->viewC64StateVIC->forceColors[i] = -1;
			}
			guiMain->UnlockMutex();
			return true;
		}

		py += fontSize + step;

		// D025-D027
		for (int i = 5; i < 8; i++)
		{
			if (x >= px && x <= px + ledSizeX && y >= py && y <= py + ledSizeY2)
			{
				if (viewC64->viewC64StateVIC->forceColors[i] == -1)
				{
					viewC64->viewC64StateVIC->forceColors[i] = viewC64->colorsToShow[i];
				}
				else
				{
					viewC64->viewC64StateVIC->forceColors[i] = -1;
				}
				guiMain->UnlockMutex();
				return true;
			}

			py += fontSize + step;
		}
	}

	return CGuiView::DoTap(x, y);
}

bool CViewC64AllGraphicsSpritesControl::DoRightClick(float x, float y)
{
	guiMain->LockMutex();

	// TODO: note this is copy pasted code from C64ViewStateVIC, needs to be generalized
	//       idea is to sync values with VIC state view. Leds should be replaced with proper buttons and callbacks.
	//		 the below is just a temporary POC made in few minutes

	float ledX = posX + fontSize * 20.1725f;
	float ledY = posY + 6.0f;

	float ledSizeX = fontSize*4.0f;
	float gap = fontSize * 0.1f;
	float step = fontSize * 0.75f;
	float ledSizeY = fontSize + gap + gap;
	float ledSizeY2 = fontSize + step;

		// render color leds for sprites
		if (btnModeMulti->IsOn())
		{
			float px = ledX;
			float py = ledY;

			if (x >= px && x <= px + ledSizeX && y >= py && y <= py + ledSizeY2)
			{
				// D021
				int i = 0x01;

				if (viewC64->viewC64StateVIC->forceColors[i] == -1)
				{
					viewC64->viewC64StateVIC->forceColors[i] = viewC64->colorsToShow[i];
				}
				viewC64->viewC64StateVIC->forceColors[i] = (viewC64->viewC64StateVIC->forceColors[i] + 1) & 0x0F;

				guiMain->UnlockMutex();
				return true;
			}

			py += fontSize + step;

			// D025-D027
			for (int i = 5; i < 8; i++)
			{
				if (x >= px && x <= px + ledSizeX && y >= py && y <= py + ledSizeY2)
				{
					if (viewC64->viewC64StateVIC->forceColors[i] == -1)
					{
						viewC64->viewC64StateVIC->forceColors[i] = viewC64->colorsToShow[i];
					}
					viewC64->viewC64StateVIC->forceColors[i] = (viewC64->viewC64StateVIC->forceColors[i] + 1) & 0x0F;

					guiMain->UnlockMutex();
					return true;
				}

				py += fontSize + step;
			}
		}
	
	return CGuiView::CGuiElement::DoRightClick(x, y);
}

bool CViewC64AllGraphicsSpritesControl::ButtonClicked(CGuiButton *button)
{
	return false;
}

bool CViewC64AllGraphicsSpritesControl::ButtonPressed(CGuiButton *button)
{
	LOGD("CViewC64AllGraphicsSpritesControl::ButtonPressed");
	
	if (button == btnModeBitmapColorsBlackWhite)
	{
		viewC64->viewC64MemoryDataDump->renderDataWithColors = false;

		if (forcedRenderScreenMode != VIEW_C64_ALL_GRAPHICS_FORCED_GRAY)
		{
			btnModeMulti->SetOn(false);
			btnModeHires->SetOn(false);
			btnModeBitmapColorsBlackWhite->SetOn(true);
			forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_GRAY;
			SetLockableButtonDefaultColors(btnModeBitmapColorsBlackWhite);
		}
		else
		{
			ClearGraphicsForcedMode();
		}
		return true;
	}
	else if (button == btnModeHires)
	{
		// is it already set?
		if (forcedRenderScreenMode == VIEW_C64_ALL_GRAPHICS_FORCED_HIRES)
		{
			// switch to multi
			viewC64->viewC64MemoryDataDump->renderDataWithColors = true;

			btnModeBitmapColorsBlackWhite->SetOn(false);
			btnModeHires->SetOn(false);
			btnModeMulti->SetOn(true);
			forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_MULTI;
			SetLockableButtonDefaultColors(btnModeMulti);
			return true;
		}

		viewC64->viewC64MemoryDataDump->renderDataWithColors = false;

		if (forcedRenderScreenMode != VIEW_C64_ALL_GRAPHICS_FORCED_HIRES)
		{
			btnModeBitmapColorsBlackWhite->SetOn(false);
			btnModeMulti->SetOn(false);
			btnModeHires->SetOn(true);
			forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_HIRES;
			SetLockableButtonDefaultColors(btnModeHires);
		}
		else
		{
			ClearGraphicsForcedMode();
		}
		return true;
	}
	else if (button == btnModeMulti)
	{
		if (forcedRenderScreenMode == VIEW_C64_ALL_GRAPHICS_FORCED_MULTI)
		{
			// switch to hires
			viewC64->viewC64MemoryDataDump->renderDataWithColors = false;

			btnModeBitmapColorsBlackWhite->SetOn(false);
			btnModeMulti->SetOn(false);
			btnModeHires->SetOn(true);
			forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_HIRES;
			SetLockableButtonDefaultColors(btnModeHires);
			return true;
		}

		viewC64->viewC64MemoryDataDump->renderDataWithColors = true;

		if (forcedRenderScreenMode != VIEW_C64_ALL_GRAPHICS_FORCED_MULTI)
		{
			btnModeBitmapColorsBlackWhite->SetOn(false);
			btnModeHires->SetOn(false);
			btnModeMulti->SetOn(true);
			forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_MULTI;
			SetLockableButtonDefaultColors(btnModeMulti);
		}
		else
		{
			ClearGraphicsForcedMode();
		}
		return true;
	}

	else if (button == btnShowGrid)
	{
		viewC64->viewC64VicControl->btnShowGrid->DoSwitch();
		return true;
	}

	return false;
}

bool CViewC64AllGraphicsSpritesControl::ButtonSwitchChanged(CGuiButtonSwitch *button)
{
	LOGD("CViewC64AllGraphicsSpritesControl::ButtonSwitchChanged");
	
	if (button == btnShowRAMorIO)
	{
		viewC64->SwitchIsDataDirectlyFromRam(!btnShowRAMorIO->IsOn());
	}
	return false;
}

void CViewC64AllGraphicsSpritesControl::UpdateRenderDataWithColors()
{
	// ctrl+k shortcut was pressed
	ClearGraphicsForcedMode();
}

void CViewC64AllGraphicsSpritesControl::ClearGraphicsForcedMode()
{
	this->forcedRenderScreenMode = VIEW_C64_ALL_GRAPHICS_FORCED_NONE;
	SetSwitchButtonDefaultColors(btnModeBitmapColorsBlackWhite);
	SetSwitchButtonDefaultColors(btnModeHires);
	SetSwitchButtonDefaultColors(btnModeMulti);
}

// bug: this event is not called when layout is set, and button state is updated on keyboard shortcut only
void CViewC64AllGraphicsSpritesControl::ActivateView()
{
	LOGG("CViewC64AllGraphicsSpritesControl::ActivateView()");
	UpdateShowIOButton();
}

void CViewC64AllGraphicsSpritesControl::DeactivateView()
{
	LOGG("CViewC64AllGraphicsSpritesControl::DeactivateView()");
}

