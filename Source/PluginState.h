/*
  ==============================================================================

    PluginState.h
    Created: 7 Apr 2019 12:27:01pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "PluginIDs.h"
#include "Structures/Preset.h"
#include "Structures/Mode.h"

struct SuperVirtualKeyboardPluginState
{
	ValueTree pluginStateNode;
	ValueTree modeLibraryNode;
	ValueTree presetCurrentNode; // Always updated when some preset is selected
	ValueTree keyboardWindowNode;
	ValueTree pianoNode;

	SuperVirtualKeyboardPluginState() :
			pluginStateNode(IDs::pluginStateNode),
			modeLibraryNode(IDs::modeLibraryNode),
			presetCurrentNode(IDs::presetNode),
			undoManager(new UndoManager()),
			appCmdMgr(new ApplicationCommandManager())

	{
		createPresets();
		pluginStateNode.addChild(modeLibraryNode, 0, nullptr);
		pluginStateNode.addChild(presetCurrentNode, 1, nullptr);
	}

	~SuperVirtualKeyboardPluginState() {}

	//==============================================================================

	OwnedArray<Mode>* get_presets();
	Array<Array<Mode*>>* get_presets_sorted();
	UndoManager* getUndoManager();

	SvkPreset* getCurrentPreset();
	int get_current_preset_index();
	Mode* getCurrentMode();

	int is_mode_in_presets(String stepsStringIn);

	void setCurrentMode(int presetIndexIn);
	void setCurrentMode(Mode* modeIn);

	void set_current_key_settings(ValueTree pianoNodeIn);
	bool loadPreset();

	//==============================================================================
	
	template <class T>
	static void add_array_to_node(ValueTree nodeIn, const Array<T>& arrayIn, Identifier arrayID, Identifier itemId);

	template <class T>
	static void get_array_from_node(const ValueTree nodeIn, Array<T>& arrayIn, Identifier arrayID);

	static void add_array_to_node(ValueTree nodeIn, const Array<Colour>& arrayIn, Identifier arrayID, Identifier itemId);

	static void get_array_from_node(const ValueTree nodeIn, Array<Colour>& arrayIn, Identifier arrayID);

	//==============================================================================

	std::unique_ptr<ApplicationCommandManager> appCmdMgr;

private:

	void createPresets();

	//==============================================================================

	std::unique_ptr<UndoManager> undoManager;
	OwnedArray<Mode> presets;
	Array<Array<Mode*>> presetsSorted;

	std::unique_ptr<SvkPreset> presetCurrent;
	Mode* modeCurrent;
};
