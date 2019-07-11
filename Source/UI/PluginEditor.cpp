/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "../PluginProcessor.h"

using namespace VirtualKeyboard;

//==============================================================================
SvkPluginEditor::SvkPluginEditor(SvkAudioProcessor& p, ApplicationCommandManager* cmdMgr)
	: AudioProcessorEditor(&p), processor(p), appCmdMgr(cmdMgr), pluginState(processor.getPluginState())
{
	setName("Super Virtual Keyboard");
	setResizable(true, true);
	setBroughtToFrontOnMouseClick(true);
	
	keyboardEditorBar = std::make_unique<KeyboardEditorBar>(pluginState, appCmdMgr);
	keyboardEditorBar.get()->setName("Keyboard Editor Bar");
	keyboardEditorBar.get()->setSize(640, 48);
	addAndMakeVisible(keyboardEditorBar.get());
    
	virtualKeyboard = pluginState->getKeyboard();

	view = std::make_unique<Viewport>("Piano Viewport");
	addAndMakeVisible(view.get());
	view.get()->setViewedComponent(virtualKeyboard, false);
	view.get()->setTopLeftPosition(1, 49);
    
    colorChooserWindow.reset(new ColorChooserWindow("Color Chooser", Colours::slateblue, DocumentWindow::closeButton));
    colorChooserWindow->setSize(450, 450);
	colorChooserWindow->addChangeListener(this);
	colorChooserWindow->addToDesktop();

	colorSelector.reset(new ColourSelector());
	colorSelector->setSize(450, 450);
	colorChooserWindow->setContentOwned(colorSelector.get(), true);
    
    midiSettingsWindow = std::make_unique<MidiSettingsWindow>();
    midiSettingsWindow->setSize(560, 150);
    midiSettingsComponent = std::make_unique<MidiSettingsComponent>(pluginState);
    midiSettingsComponent->setSize(560, 150);
    midiSettingsWindow->setContentOwned(midiSettingsComponent.get(), true);
    midiSettingsWindow->setAlwaysOnTop(true);
    
	pluginState->getMidiProcessor()->resetWithRate(processor.getSampleRate());

    pluginState->addChangeListener(this);
    pluginState->getMidiProcessor()->getKeyboardState()->addListener(virtualKeyboard); // displays MIDI on Keyboard

	initNodeData();
    
    midiSettingsComponent->setMode1(9);
    midiSettingsComponent->setMode2(9);

    midiSettingsComponent->setMode1RootNote(60);
    midiSettingsComponent->setMode2RootNote(60);
    
	appCmdMgr->registerAllCommandsForTarget(this);
	appCmdMgr->registerAllCommandsForTarget(virtualKeyboard);

	setMouseClickGrabsKeyboardFocus(true);
	addMouseListener(this, true);
    
    setSize(1000, 250);
	setResizeLimits(986, 100, 10e4, 10e4);
    
	startTimerHz(20);
}

SvkPluginEditor::~SvkPluginEditor()
{
    pluginState->removeChangeListener(this);
    keyboardEditorBar->removeChangeListener(this);
    pluginState->getMidiProcessor()->getKeyboardState()->removeListener(virtualKeyboard);
}

//==============================================================================

void SvkPluginEditor::initNodeData()
{
	if (pluginState->pluginEditorNode.isValid())
	{
		pluginEditorNode = pluginState->pluginEditorNode;

		setSize(pluginEditorNode[IDs::windowBoundsW], pluginEditorNode[IDs::windowBoundsH]);
        view.get()->setViewPosition((int)pluginEditorNode[IDs::viewportPosition], 0);
	}
	else
	{
		pluginEditorNode = ValueTree(IDs::pluginEditorNode);
		pluginState->pluginEditorNode = pluginEditorNode;
		pluginState->pluginStateNode.addChild(pluginEditorNode, -1, nullptr);

		view.get()->setViewPositionProportionately(0.52, 0);
        pluginState->getMidiProcessor()->setAutoRemapOn();
	}
    
    update_children_to_preset();
}

void SvkPluginEditor::updateNodeData()
{
	pluginEditorNode.setProperty(IDs::windowBoundsW, getWidth(), nullptr);
	pluginEditorNode.setProperty(IDs::windowBoundsH, getHeight(), nullptr);
	pluginEditorNode.setProperty(IDs::viewportPosition, view.get()->getViewPositionX(), nullptr);
}

void SvkPluginEditor::update_children_to_preset()
{
	Mode* modeLoaded = pluginState->getModeLoaded();
	
	keyboardEditorBar->setModeReadoutText(modeLoaded->getStepsString());
	keyboardEditorBar->setModeLibraryText(modeLoaded->getName());
    keyboardEditorBar->setOffsetReadout(modeLoaded->getRootNote());

	midiSettingsComponent->setMode2(modeLoaded);
    
    if (pluginState->getPresetinSlot()->theKeyboardNode[IDs::pianoHasCustomColor])
        virtualKeyboard->restoreDataNode(pluginState->getPresetinSlot()->theKeyboardNode);
    
    virtualKeyboard->setMode(modeLoaded);
	virtualKeyboard->updateKeys();

    keyboardEditorBar->repaint();
	DBG("Children Updated");
}

//==============================================================================

bool SvkPluginEditor::savePresetToFile()
{
    bool written = pluginState->savePresetViewedToFile();
	if (written)
		DBG("file was saved");
	else
		DBG("not saved");

	return written;
}

bool SvkPluginEditor::saveMode()
{

}

bool SvkPluginEditor::loadPreset()
{
	bool loaded = pluginState->loadPresetFromFile();

	if (loaded)
	{
		//virtualKeyboard->restoreDataNode(pluginState->getPresetLoaded()->theKeyboardNode);
		update_children_to_preset();
	}

	return loaded;
}

bool SvkPluginEditor::loadMode()
{

}

bool SvkPluginEditor::exportReaperMap()
{
	ReaperWriter rpp = ReaperWriter(pluginState->getModeLoaded());
	return rpp.write_file();
}

bool SvkPluginEditor::exportAbletonMap()
{

}

void SvkPluginEditor::commitCustomScale()
{
	String scaleSteps = controlComponent->getScaleEntryText();
	ValueTree customMode = Mode::createNode(scaleSteps);
	pluginState->loadMode(customMode);
}

void SvkPluginEditor::setMode1()
{
	setMode1(controlComponent->getMode1BoxSelection());
}

void SvkPluginEditor::setMode1(ValueTree modeNodeIn)
{
	pluginState->loadMode(modeNodeIn, 0);
}

void SvkPluginEditor::setMode1(int presetId)
{
	pluginState->loadMode(presetId, 0);
}

void SvkPluginEditor::setMode2()
{
	setMode2(controlComponent->getMode2BoxSelection());
}

void SvkPluginEditor::setMode2(ValueTree modeNodeIn)
{
	pluginState->loadMode(modeNodeIn, 1);
}

void SvkPluginEditor::setMode2(int presetId)
{
	pluginState->loadMode(presetId, 1);
}

void SvkPluginEditor::setMode1Root()
{
	setMode1Root(controlComponent->getMode1Root());
}

void SvkPluginEditor::setMode1Root(int rootIn)
{
	Mode* mode1 = pluginState->getModeLoaded(0);
	mode1->setRootNote(rootIn);
}

void SvkPluginEditor::setMode2Root()
{
	setMode2Root(controlComponent->getMode2Root());
}

void SvkPluginEditor::setMode2Root(int rootIn)
{
	Mode* mode2 = pluginState->getModeLoaded(1);
	mode2->setRootNote(rootIn);
}

void SvkPluginEditor::setModeView()
{
	setModeView(controlComponent->getModeViewed());
}

void SvkPluginEditor::setModeView(int modeNumberIn)
{

}

void SvkPluginEditor::showModeInfo()
{
	modeInfo = new ModeInfoDialog(pluginState->getModeLoaded());
	modeInfo->addChangeListener(this);

	Component* modeViewed = controlComponent->getModeViewed() == 0 ?
		controlComponent->getMode1Box() : controlComponent->getMode2Box();

	CallOutBox::launchAsynchronously(modeInfo, getScreenBounds(), modeViewed);
}

void SvkPluginEditor::setMappingStyle()
{
	setMappingStyle(controlComponent->getMappingStyle());
}

void SvkPluginEditor::setMappingStyle(int mapStyleId)
{

}

void SvkPluginEditor::applyMap()
{

}

void SvkPluginEditor::setAutoMap()
{
	setAutoMap(controlComponent->getAutoMapState());
}

void SvkPluginEditor::setAutoMap(bool isAutoMapping)
{

}

void SvkPluginEditor::beginMapEditing()
{

}

void SvkPluginEditor::setPeriodShift()
{
	setPeriodShift(controlComponent->getPeriodShift());
}

void SvkPluginEditor::setPeriodShift(int periodsIn)
{

}

void SvkPluginEditor::setMidiChannel()
{
	setMidiChannel(controlComponent->getMidiChannel());
}

void SvkPluginEditor::setMidiChannel(int midiChannelIn)
{

}

void SvkPluginEditor::beginColorEditing()
{
	colorChooserWindow->setVisible(true);
	virtualKeyboard->setUIMode(UIMode::colorMode);
	keyboardEditorBar->allowUserInput(false);
}


void SvkPluginEditor::setNoteNumsVisible()
{
	setNoteNumsVisible(controlComponent->getNoteNumsView());
}

void SvkPluginEditor::setNoteNumsVisible(bool noteNumsVisible)
{

}

void SvkPluginEditor::setKeyStyle()
{
	setKeyStyle(controlComponent->getKeyStyle());
}

void SvkPluginEditor::setKeyStyle(int keyStyleId)
{

}

void SvkPluginEditor::setHighlightStyle()
{
	setHighlightStyle(controlComponent->getHighlightStyle());
}

void SvkPluginEditor::setHighlightStyle(int highlightStyleId)
{

}


//==============================================================================

void SvkPluginEditor::paint(Graphics& g)
{
	g.fillAll(Colours::darkgrey);
}

void SvkPluginEditor::resized()
{
	int viewPositionKeyboardX = view->getViewPositionX();
	AudioProcessorEditor::resized();

	keyboardEditorBar->setBounds(0, 0, getWidth(), 36);
	view->setBounds(0, keyboardEditorBar->getBottom(), getWidth(), getHeight() - keyboardEditorBar->getHeight());
	virtualKeyboard->setBounds(0, 0, virtualKeyboard->getWidthFromHeight(view->getMaximumVisibleHeight()), view->getMaximumVisibleHeight()-1);
	
	view->setViewPosition(viewPositionKeyboardX, 0);
}

//==============================================================================

void SvkPluginEditor::timerCallback()
{
	virtualKeyboard->repaint();
}

//==============================================================================

void SvkPluginEditor::userTriedToCloseWindow()
{
	if (colorChooserWindow->isVisible())
		colorChooserWindow->closeButtonPressed();

	setVisible(false);
}

//==============================================================================

void SvkPluginEditor::mouseDown(const MouseEvent& e)
{
    Key* key = virtualKeyboard->getKeyFromPosition(e);

	if (virtualKeyboard->getUIMode() == UIMode::playMode)
	{
		if (key)
		{
			if (e.mods.isShiftDown() && !e.mods.isAltDown() && key->activeState == 2)
			{
				// note off
				//virtualKeyboard->lastKeyClicked = 0;
				virtualKeyboard->triggerKeyNoteOff(key);
			}
			else
			{
				if (e.mods.isAltDown())
				{
					Key* oldKey = virtualKeyboard->getKey(virtualKeyboard->getLastKeyClicked());
					virtualKeyboard->triggerKeyNoteOff(oldKey);
				}

				virtualKeyboard->triggerKeyNoteOn(key, virtualKeyboard->getKeyVelocity(key, e));
				virtualKeyboard->setLastKeyClicked(key->keyNumber);
			}
		}
	}
	else if (virtualKeyboard->getUIMode() == UIMode::colorMode)
	{
		if (key)
		{
			if (e.mods.isRightButtonDown())
			{
				if (e.mods.isShiftDown())
				{
					virtualKeyboard->resetKeyOrderColors(key->order, true);
				}
				else if (e.mods.isCtrlDown())
				{
					virtualKeyboard->resetKeySingleColor(key->keyNumber);
				}
				else
				{
					virtualKeyboard->resetKeyDegreeColors(key->keyNumber);
				}
			}

			else if (e.mods.isShiftDown())
			{
				if (virtualKeyboard->getKeyDegreeColor(key->scaleDegree).isOpaque())
					virtualKeyboard->resetKeyDegreeColors(key->scaleDegree);

				else if (virtualKeyboard->getKeySingleColor(key->keyNumber).isOpaque())
					virtualKeyboard->resetKeySingleColor(key->keyNumber);

				virtualKeyboard->setKeyColorOrder(key->order, 3, colorSelector->getCurrentColour());
			}
			else if (e.mods.isCtrlDown())
				virtualKeyboard->beginColorEditing(key->keyNumber, 3, colorSelector->getCurrentColour());
			else
				virtualKeyboard->setKeyColorDegree(key->keyNumber, 3, colorSelector->getCurrentColour());       
		}
	}
    else if (virtualKeyboard->getUIMode() == UIMode::mapMode)
    {
        if (key)
        {
            virtualKeyboard->selectKeyToMap(key);
        }
    }
}

void SvkPluginEditor::mouseDrag(const MouseEvent& e)
{
	if (virtualKeyboard->getUIMode() == UIMode::playMode)
	{
		Key* key = virtualKeyboard->getKeyFromPosition(e);

		if (key)
		{
			if (key->keyNumber != virtualKeyboard->getLastKeyClicked())
			{
				Key* oldKey = virtualKeyboard->getKey(virtualKeyboard->getLastKeyClicked());
				if (!e.mods.isShiftDown() || e.mods.isAltDown())
				{
					virtualKeyboard->triggerKeyNoteOff(oldKey);
				}

				virtualKeyboard->triggerKeyNoteOn(key, virtualKeyboard->getKeyVelocity(key, e));
				virtualKeyboard->setLastKeyClicked(key->keyNumber);
				repaint();
			}
		}
	}
}

void SvkPluginEditor::mouseUp(const MouseEvent& e)
{
	if (virtualKeyboard->getUIMode() == UIMode::playMode)
	{
		Key* key = virtualKeyboard->getKeyFromPosition(e);

		if (key)
		{
			if (!e.mods.isShiftDown())
			{
				virtualKeyboard->triggerKeyNoteOff(key);
				key->activeState = 1;
				repaint();
			}
		}
	}
}

void SvkPluginEditor::mouseMove(const MouseEvent& e)
{
	if (virtualKeyboard->getUIMode() != UIMode::colorMode)
	{
		Key* key = virtualKeyboard->getKeyFromPosition(e);

		if (key)
		{
			if (key->activeState == 0)
			{
				key->activeState = 1;
				repaint();
			}
		}
	}
}

//==============================================================================

void SvkPluginEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    // New Mode loaded
    if (source == pluginState)
    {
        virtualKeyboard->resetKeyColors(true);
        update_children_to_preset();
    }
    
    // Color editing has finished
	if (source == colorChooserWindow.get())
	{
		if (virtualKeyboard->getUIMode() == UIMode::colorMode)
		{
			virtualKeyboard->updatePianoNode();

			virtualKeyboard->updateKeys();
			virtualKeyboard->setUIMode(UIMode::playMode);
			keyboardEditorBar->allowUserInput();
		}
	}
    
    // Prepare to play
    if (source == &processor)
    {
        //pluginState->midiStateIn->addListener(virtualKeyboard);
    }
    
    // Root note or Mapping button toggled 
    if (source == keyboardEditorBar.get())
    {
        if (keyboardEditorBar->isMapButtonOn())
        {
            pluginState->getMidiProcessor()->pauseMidiInput();
            virtualKeyboard->setUIMode(UIMode::mapMode);
        }
        else
        {
            pluginState->getMidiProcessor()->pauseMidiInput(false);
            virtualKeyboard->setUIMode(UIMode::playMode);
        }

        midiSettingsComponent->setMode2RootNote(keyboardEditorBar->getOffsetReadout());
		update_children_to_preset();
    }

	// Mode Info Changed
	if (source == modeInfo)
	{
		pluginState->loadMode(pluginState->getModeLoaded()->modeNode);
	}
}

//==============================================================================


File SvkPluginEditor::fileDialog(String message, bool forSaving)
{
	FileChooser chooser(message, File::getSpecialLocation(File::userDocumentsDirectory), "*.svk");

	if (forSaving)
		chooser.browseForFileToSave(true);
	else
		chooser.browseForFileToOpen();

	return chooser.getResult();
}

//==============================================================================

ApplicationCommandTarget* SvkPluginEditor::getNextCommandTarget()
{
	return virtualKeyboard;// findFirstTargetParentComponent();
}

void SvkPluginEditor::getAllCommands(Array< CommandID > &c)
{
	Array<CommandID> commands{
		IDs::CommandIDs::savePresetToFile,
		IDs::CommandIDs::saveMode,
		IDs::CommandIDs::loadPreset,
		IDs::CommandIDs::loadMode,
		IDs::CommandIDs::exportReaperMap,
		IDs::CommandIDs::exportAbletonMap,
		IDs::CommandIDs::commitCustomScale,
		IDs::CommandIDs::setMode1,
		IDs::CommandIDs::setMode2,
		IDs::CommandIDs::setMode1RootNote,
		IDs::CommandIDs::setMode2RootNote,
		IDs::CommandIDs::viewMode1,
		IDs::CommandIDs::viewMode2,
		IDs::CommandIDs::showModeInfo,
		IDs::CommandIDs::setMappingStyle,
		IDs::CommandIDs::applyMapping,
		IDs::CommandIDs::setAutoMap,
		IDs::CommandIDs::beginMapEditing,
		IDs::CommandIDs::setPeriodShift,
		IDs::CommandIDs::setMidiChannelOut,
		IDs::CommandIDs::beginColorEditing,
		IDs::CommandIDs::showMidiNoteNumbers,
		IDs::CommandIDs::setKeyStyle,
		IDs::CommandIDs::beginColorEditing,
        IDs::CommandIDs::showModeInfo,
	};

	c.addArray(commands);
}

void SvkPluginEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo &result)
{
	switch (commandID)
	{
	case IDs::CommandIDs::savePresetToFile:
		result.setInfo("Save Layout", "Save your custom layout to a file.", "Preset", 0);
		break;
	case IDs::CommandIDs::saveMode:
		result.setInfo("Save Mode", "Save the currently viewed mode.", "Preset", 0);
		break;
	case IDs::CommandIDs::loadPreset:
		result.setInfo("Load Layout", "Load a custom layout from a file.", "Preset", 0);
		break;
	case IDs::CommandIDs::loadMode:
		result.setInfo("Load Mode", "Load only the mode of a preset.", "Preset", 0);
		break;
	case IDs::CommandIDs::exportReaperMap:
		result.setInfo("Export to Reaper", "Exports the current preset to a MIDI Note Name text file for use in Reaper's piano roll.", "Preset", 0);
		break;
	case IDs::CommandIDs::exportAbletonMap:
		result.setInfo("Export to Ableton", "Exports the mode mapping to a MIDI file for to use in Ableton's piano roll for folding.", "Preset", 0);
		break;
	case IDs::CommandIDs::commitCustomScale:
		result.setInfo("Commit custom scale", "Registers the entered scale steps as the current custom scale.", "Preset", 0);
		break;
	case IDs::CommandIDs::setMode1:
		result.setInfo("Set Mode 1", "Loads the mode into the Mode 1 slot.", "Preset", 0);
		break;
	case IDs::CommandIDs::setMode2:
		result.setInfo("Set Mode 2", "Loads the mode into the Mode 2 slot.", "Preset", 0);
		break;
	case IDs::CommandIDs::setMode1RootNote:
		result.setInfo("Set Mode 1 Root", "Applies the selected root note for Mode 1.", "Preset", 0);
		break;
	case IDs::CommandIDs::setMode2RootNote:
		result.setInfo("Set Mode 2 Root", "Applies the selected root note for Mode 2.", "Preset", 0);
		break;
	case IDs::CommandIDs::viewMode1:
		result.setInfo("View Mode 1", "Shows the Mode 1 slot on the keyboard.", "Keyboard", 0);
		break;
	case IDs::CommandIDs::viewMode2:
		result.setInfo("View Mode 2", "Shows the Mode 2 slot on the keyboard.", "Keyboard", 0);
		break;
    case IDs::CommandIDs::showModeInfo:
        result.setInfo("Show Mode Info", "Shows information regarding the selected Mode.", "Mode", 0);
		break;
	case IDs::CommandIDs::setMappingStyle:
		result.setInfo("Mapping Style", "Choose a mapping style for remapping MIDI notes.", "Midi", 0);
		break;
	case IDs::CommandIDs::applyMapping:
		result.setInfo("Apply Mapping", "Map incoming MIDI notes to Mode 2 with the selected mapping style.", "Midi", 0);
		break;
    case IDs::CommandIDs::setAutoMap:
        result.setInfo("Auto Map to Scale", "Remap Midi notes when scale changes", "Midi", 0);
        break;
	case IDs::CommandIDs::beginMapEditing:
		result.setInfo("Manual Map", "Map MIDI notes to on-screen keys by selecting desired key on screen and triggering the MIDI Note.", "Midi", 0);
		break;
	case IDs::CommandIDs::setPeriodShift:
		result.setInfo("Shift by Mode 2 Period0", "Shift the outgoing MIDI notes by the selected number of Mode 2 periods.", "Midi", 0);
		break;
	case IDs::CommandIDs::setMidiChannelOut:
		result.setInfo("Set MIDI Channel Out", "Set the outgoing MIDI Notes to the selected MIDI Channel.", "Midi", 0);
		break;
	case IDs::CommandIDs::beginColorEditing:
		result.setInfo("Change Keyboard Colors", "Allows you to change the default colors for the rows of keys.", "Keyboard", 0);
		break;
	case IDs::CommandIDs::showMidiNoteNumbers:
		result.setInfo("Show Midi Note Numbers", "Shows incoming MIDI notes on Mode 1 and outgoing MIDI Notes on Mode 2.", "Keyboard", 0);
		break;
	case IDs::CommandIDs::setKeyStyle:
		result.setInfo("Set Key Style", "Sets the selected style for drawing overlapping degrees between mode degrees.", "Keyboard", 0);
		break;
	case IDs::CommandIDs::setHighlightStyle:
		result.setInfo("Set Highlight Style", "Sets the selected style for drawing triggered notes.", "Keyboard", 0);
		break;
	default:
		break;
	}
}

bool SvkPluginEditor::perform(const InvocationInfo &info)
{
    switch (info.commandID)
    {
        case IDs::CommandIDs::savePresetToFile:
        {
            savePresetToFile();
            break;
        }
		case IDs::CommandIDs::saveMode:
		{
			saveMode();
			break;
		}
        case IDs::CommandIDs::loadPreset:
        {
            loadPreset();
            break;
        }
		case IDs::CommandIDs::loadMode:
		{
			loadMode();
			break;
		}
        case IDs::CommandIDs::exportReaperMap:
        {
            exportReaperMap();
            break;
        }
		case IDs::CommandIDs::exportAbletonMap:
		{
			exportAbletonMap();
			break;
		}
		case IDs::CommandIDs::commitCustomScale:
		{
			commitCustomScale();
			break;
		}
		case IDs::CommandIDs::setMode1:
		{
			setMode1();
			break;
		}
		case IDs::CommandIDs::setMode2:
		{
			setMode2();
			break;
		}
		case IDs::CommandIDs::setMode1RootNote:
		{
			setMode1Root();
			break;
		}
		case IDs::CommandIDs::setMode2RootNote:
		{
			setMode1Root();
			break;
		}
		case IDs::CommandIDs::viewMode1:
		{
			setModeView(1);
			break;
		}
		case IDs::CommandIDs::viewMode2:
		{
			setModeView(2);
			break;
		}
		case IDs::CommandIDs::showModeInfo:
		{
			showModeInfo();
			break;
		}
		case IDs::CommandIDs::setMappingStyle:
		{
			setMappingStyle();
			break;
		}
		case IDs::CommandIDs::applyMapping:
		{
			applyMap();
			break;
		}
		case IDs::CommandIDs::setAutoMap:
		{
			setAutoMap();
			break;
		}
		case IDs::CommandIDs::beginMapEditing:
		{
			beginMapEditing();
			break;
		}
		case IDs::CommandIDs::setPeriodShift:
		{
			setPeriodShift();
			break;
		}
		case IDs::CommandIDs::setMidiChannelOut:
		{
			setMidiChannel();
			break;
		}
		case IDs::CommandIDs::beginColorEditing:
		{
			beginColorEditing();
			break;
		}
		case IDs::CommandIDs::showMidiNoteNumbers:
		{
			setNoteNumsVisible();
			break;
		}
		case IDs::CommandIDs::setKeyStyle:
		{
			setKeyStyle();
			break;
		}
		case IDs::CommandIDs::setHighlightStyle:
		{
			setHighlightStyle();
			break;
		}
        default:
        {
            return false;
        }
            
    }
    return true;
}

