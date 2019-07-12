/*
  ==============================================================================

    FractionalGrid.h
    Created: 24 Mar 2019 1:58:51pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../PluginIDs.h"
#include "../CommonFunctions.h"
#include "Mode.h"

struct SvkPreset
{
	ValueTree parentNode;
    ValueTree thePropertiesNode;
	ValueTree theModeNode;
	ValueTree theKeyboardNode;
    ValueTree theMidiSettingsNode;

	SvkPreset();
	SvkPreset(ValueTree presetNodeIn);
	SvkPreset(SvkPreset& presetToCopy);
	~SvkPreset();

	bool restoreFromNode(ValueTree presetNodeIn);
	void commitPreset();

	ValueTree getModeInSlot(int slotNum);

	ValueTree getMode1();
	ValueTree getMode2();

	int getMode1SlotNumber();
	int getMode2SlotNumber();

	int getNumModes();

	void setMode1SlotNumber(int slotNumIn);
	void setMode2SlotNumber(int slotNumIn);
	
	int setModeSlot(ValueTree modeNodeIn, int slotNumber = 0);
	int addMode(ValueTree modeNodeIn);
	bool addModes(Array <ValueTree> modeSlotsIn);
       
	String toString();	

	static bool isValidPresetNode(ValueTree presetNodeIn);
	static bool isValidPresetNode(ValueTree presetNodeIn, Array<ValueTree>& modesContained);

private:

	Array<ValueTree> modeSlots;

	int mode1SlotNumber = 0;
	int mode2SlotNumber = 0;
};
