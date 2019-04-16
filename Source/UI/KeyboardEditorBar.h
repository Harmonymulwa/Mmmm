/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.4.3

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2017 - ROLI Ltd.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"
#include "../PluginState.h"
#include "../Structures/ModeLayout.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class KeyboardEditorBar  : public Component,
                           public Button::Listener,
                           public ComboBox::Listener
{
public:
    //==============================================================================
    KeyboardEditorBar (SuperVirtualKeyboardPluginState* pluginStateIn, ApplicationCommandManager* managerIn);
    ~KeyboardEditorBar();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    int get_mode_preset_index(String anyNameIn);
    int get_mode_library_index();
    void set_text_boxes(String presetName, String steps);

	void create_and_send_mode();

	void populate_preset_menu();

    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void mouseEnter (const MouseEvent& e) override;
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) override;
    bool keyPressed (const KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

	struct KeyboardMenu : public Component,
						public MenuBarModel
	{
		StringArray options;
		std::unique_ptr<MenuBarComponent> menuParent;
		ApplicationCommandManager* appCmdMgr;

		KeyboardMenu(ApplicationCommandManager* managerIn);
		~KeyboardMenu();

		StringArray getMenuBarNames() override;
		void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
		PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName) override;
		void resized() override;
	};

	std::unique_ptr<KeyboardMenu> pianoMenu;
	SuperVirtualKeyboardPluginState* pluginState;
	ApplicationCommandManager* appCmdMgr;

	// Preset Menus

    OwnedArray<ModeLayout>* presets;
    Array<Array<ModeLayout*>>* presetsSorted;

    HashMap<String, int> menuToPresetIndex;
    std::unique_ptr<PopupMenu> menuSortByScale;
    std::unique_ptr<PopupMenu> menuSortByMode;
    std::unique_ptr<PopupMenu> menuSortByFamily;

	// Key Locks

	bool enterDown = false;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<TextEditor> modeTextEditor;
    std::unique_ptr<TextButton> sendScaleBtn;
    std::unique_ptr<ComboBox> modeLibraryBox;
    std::unique_ptr<TextButton> keyboardModeBtn;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardEditorBar)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
