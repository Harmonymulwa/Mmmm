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
    appCmdMgr->registerAllCommandsForTarget(this);
    appCmdMgr->setFirstCommandTarget(this);

	controlComponent.reset(new PluginControlComponent(pluginState));
	controlComponent->setBounds(getBounds());
	addAndMakeVisible(controlComponent.get());
    
	view = controlComponent->getViewport();
	virtualKeyboard = pluginState->getKeyboard();
	view->setViewedComponent(virtualKeyboard, false);
    
    keyboardScroll = &view->getHorizontalScrollBar();
    keyboardScroll->addListener(this);
    
    colorChooserWindow.reset(new ColorChooserWindow("Color Chooser", Colours::slateblue, DocumentWindow::closeButton));
    colorChooserWindow->setSize(450, 450);
	colorChooserWindow->addChangeListener(this);
	colorChooserWindow->addToDesktop();

	colorSelector.reset(new ColourSelector());
	colorSelector->setSize(450, 450);
	colorChooserWindow->setContentOwned(colorSelector.get(), true);
    
	pluginState->getMidiProcessor()->resetWithRate(processor.getSampleRate());
    pluginState->addChangeListener(this);
    
	setMouseClickGrabsKeyboardFocus(true);
	addMouseListener(this, true);
    
    setSize(1000, 210);
	setResizeLimits(750, 100, 10e4, 10e4);
    
    initNodeData();
	startTimerHz(60);
}

SvkPluginEditor::~SvkPluginEditor()
{
    pluginState->removeChangeListener(this);
}

//==============================================================================

void SvkPluginEditor::initNodeData()
{
    // Gui is recreated
	if (pluginState->pluginEditorNode.isValid() && pluginState->pluginEditorNode.getNumProperties() > 2)
	{
		pluginEditorNode = pluginState->pluginEditorNode;

		setSize(pluginEditorNode[IDs::windowBoundsW], pluginEditorNode[IDs::windowBoundsH]);
        view->setViewPosition((int)pluginEditorNode[IDs::viewportPosition], 0);
	}
    // Intialization
	else
	{
		pluginEditorNode = ValueTree(IDs::pluginEditorNode);
		pluginState->pluginEditorNode = pluginEditorNode;
		pluginState->pluginStateNode.addChild(pluginEditorNode, -1, nullptr);

		view->setViewPositionProportionately(0.52, 0);
	}
    
    updateUI();
}

void SvkPluginEditor::updateNodeData()
{
	pluginEditorNode.setProperty(IDs::windowBoundsW, getWidth(), nullptr);
	pluginEditorNode.setProperty(IDs::windowBoundsH, getHeight(), nullptr);
	pluginEditorNode.setProperty(IDs::viewportPosition, view->getViewPositionX(), nullptr);
}

void SvkPluginEditor::updateUI()
{
    controlComponent->setMappingMode(pluginState->getMappingMode());
    
	controlComponent->setScaleEntryText(pluginState->getModeViewed()->getStepsString());
	controlComponent->setMappingStyleId(pluginState->getMappingStyle());
	controlComponent->setMode1Root(pluginState->getMode1Root());
	controlComponent->setMode2Root(pluginState->getMode2Root());
	controlComponent->setMode1BoxText(pluginState->getMode1()->getName());
	controlComponent->setMode2BoxText(pluginState->getMode2()->getName());
	controlComponent->setMode1View(pluginState->getModeViewedNum() == 0);
	controlComponent->setPeriodShift(pluginState->getPeriodShift());
	controlComponent->setMidiChannel(pluginState->getMidiChannelOut());
	controlComponent->setNoteNumsView(pluginState->isShowingNoteNums());
	controlComponent->setKeyStyleId(pluginState->getKeyStyle());
	controlComponent->setHighlightStyleId(pluginState->getHighlightStyle());
    
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
	return pluginState->saveModeViewedToFile();
}

void SvkPluginEditor::showSaveMenu()
{
    return;
}

bool SvkPluginEditor::loadPreset()
{
	if (pluginState->loadPresetFromFile(true))
	{
		//virtualKeyboard->restoreDataNode(pluginState->getPresetLoaded()->theKeyboardNode);
		updateUI();

		return true;
	}

	return false;
}

bool SvkPluginEditor::loadMode()
{
	if (pluginState->loadModeFromFile())
	{
		return true;
	}

	return false;
}

bool SvkPluginEditor::exportReaperMap()
{
	ReaperWriter rpp = ReaperWriter(pluginState->getModeViewed());
	return rpp.write_file();
}

bool SvkPluginEditor::exportAbletonMap()
{
	return false;
}

void SvkPluginEditor::showSettingsDialog()
{
    pluginSettingsDialog = new PluginSettingsDialog(pluginState->getPluginSettings());
    CallOutBox::launchAsynchronously(pluginSettingsDialog, getScreenBounds(), nullptr);
}

void SvkPluginEditor::commitCustomScale()
{
	String scaleSteps = controlComponent->getScaleEntryText();
	pluginState->setModeCustom(scaleSteps);
}

void SvkPluginEditor::setMode1()
{
	setMode1(controlComponent->getMode1BoxSelection());
}

void SvkPluginEditor::setMode1(int idIn)
{
    pluginState->handleModeSelection(0, idIn);
    updateUI();
}

void SvkPluginEditor::setMode2()
{
	setMode2(controlComponent->getMode2BoxSelection());
}

void SvkPluginEditor::setMode2(int idIn)
{
    pluginState->handleModeSelection(1, idIn);
    updateUI();
}

void SvkPluginEditor::setMode1Root()
{
	setMode1Root(controlComponent->getMode1Root());
}

void SvkPluginEditor::setMode1Root(int rootIn)
{
	pluginState->setMode1Root(rootIn);
}

void SvkPluginEditor::setMode2Root()
{
	setMode2Root(controlComponent->getMode2Root());
}

void SvkPluginEditor::setMode2Root(int rootIn)
{
	pluginState->setMode2Root(rootIn);
}

void SvkPluginEditor::setModeView()
{
	setModeView(controlComponent->getModeViewed());
}

void SvkPluginEditor::setModeView(int modeNumberIn)
{
	pluginState->setModeViewed(modeNumberIn);
    controlComponent->setScaleEntryText(pluginState->getModeViewed()->getStepsString());
}

void SvkPluginEditor::showModeInfo()
{
	modeInfo = new ModeInfoDialog(pluginState->getModeViewed());
	modeInfo->addChangeListener(this);

	CallOutBox::launchAsynchronously(modeInfo, controlComponent->getScaleTextEditor()->getScreenBounds(), nullptr);
}

void SvkPluginEditor::setMappingStyle()
{
	setMappingStyle(controlComponent->getMappingStyle());
}

void SvkPluginEditor::setMappingStyle(int mapStyleId)
{
	pluginState->setMapStyle(mapStyleId);
}

void SvkPluginEditor::showMapOrderEditDialog()
{
    mapByOrderDialog = new MapByOrderDialog(pluginState);
    CallOutBox::launchAsynchronously(mapByOrderDialog, controlComponent->getMappingStyleBox()->getScreenBounds(), nullptr);
}

void SvkPluginEditor::applyMap()
{
	pluginState->doMapping();
}

void SvkPluginEditor::setMappingMode()
{
	setMappingMode(controlComponent->getMappingMode());
}

void SvkPluginEditor::setMappingMode(int mappingModeId)
{
	pluginState->setMapMode(mappingModeId);
    
    if (mappingModeId == 3)
    {
        mappingHelper.reset(new MappingHelper(*pluginState->getMidiInputMap(), pluginState->getMode1(), pluginState->getMode2()));
        
    }
}

void SvkPluginEditor::beginMapEditing()
{
	// TODO
}

void SvkPluginEditor::setPeriodShift()
{
	setPeriodShift(controlComponent->getPeriodShift());
}

void SvkPluginEditor::setPeriodShift(int periodsIn)
{
	pluginState->setPeriodShift(periodsIn);
}

void SvkPluginEditor::setMidiChannel()
{
	setMidiChannel(controlComponent->getMidiChannel());
}

void SvkPluginEditor::setMidiChannel(int midiChannelIn)
{
	pluginState->setMidiChannel(midiChannelIn);
}

void SvkPluginEditor::beginColorEditing()
{
	colorChooserWindow->setVisible(true);
	virtualKeyboard->setUIMode(UIMode::colorMode);
}

void SvkPluginEditor::setNoteNumsVisible()
{
	setNoteNumsVisible(controlComponent->getNoteNumsView());
}

void SvkPluginEditor::setNoteNumsVisible(bool noteNumsVisible)
{
	pluginState->setNoteNumsShowing(noteNumsVisible);
}

void SvkPluginEditor::setKeyStyle()
{
	setKeyStyle(controlComponent->getKeyStyle());
}

void SvkPluginEditor::setKeyStyle(int keyStyleId)
{
	pluginState->setKeyStyle(keyStyleId);
}

void SvkPluginEditor::setHighlightStyle()
{
	setHighlightStyle(controlComponent->getHighlightStyle());
}

void SvkPluginEditor::setHighlightStyle(int highlightStyleId)
{
	pluginState->setHighlightStyle(highlightStyleId);
}

//==============================================================================

void SvkPluginEditor::paint(Graphics& g)
{
	g.fillAll(Colours::darkgrey);
}

void SvkPluginEditor::resized()
{
	float viewXProportion = (float) view->getViewPositionX() / view->getMaximumVisibleWidth();
	AudioProcessorEditor::resized();
	controlComponent->setSize(getWidth(), getHeight());

	//view->setBounds(0, keyboardEditorBar->getBottom(), getWidth(), getHeight() - keyboardEditorBar->getHeight());
	//virtualKeyboard->setSize(getWidth(), getHeight());
	
	view->setViewPosition((int)(viewXProportion * view->getMaximumVisibleWidth()), 0);
    
    if (pluginEditorNode.isValid())
        updateNodeData();
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

    updateNodeData();
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
    else if (pluginState->getMappingMode() == 3 && e.mods.isRightButtonDown() && pluginState->getModeViewedNum() == 1)
    {
        if (key)
        {
            bool allPeriods = true;
            
            if (e.mods.isCtrlDown())
                allPeriods = false;
            
            mappingHelper->setKeysToMap(key->keyNumber, allPeriods);
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
        updateUI();
		pluginState->updateModeViewed(false);
    }
    
    // Color editing has finished
	if (source == colorChooserWindow.get())
	{
		if (virtualKeyboard->getUIMode() == UIMode::colorMode)
		{
			virtualKeyboard->updatePianoNode();
			virtualKeyboard->updateKeyColors();
			virtualKeyboard->setUIMode(UIMode::playMode);
		}
	}
    
    // Prepare to play
    if (source == &processor)
    {
        //pluginState->midiStateIn->addListener(virtualKeyboard);
    }

	// Mode Info Changed
	if (source == modeInfo)
	{

		pluginState->commitModeInfo();
	}
}

void SvkPluginEditor::scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == keyboardScroll)
    {
        pluginEditorNode.setProperty(IDs::viewportPosition, view->getViewPositionX(), nullptr);
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
    return findFirstTargetParentComponent();
}

void SvkPluginEditor::getAllCommands(Array<CommandID>& c)
{
	Array<CommandID> commands{
		IDs::CommandIDs::savePresetToFile,
		IDs::CommandIDs::saveMode,
		IDs::CommandIDs::loadPreset,
		IDs::CommandIDs::loadMode,
		IDs::CommandIDs::exportReaperMap,
		IDs::CommandIDs::exportAbletonMap,
        IDs::CommandIDs::showSettingsDialog,
		IDs::CommandIDs::commitCustomScale,
		IDs::CommandIDs::setMode1,
		IDs::CommandIDs::setMode2,
		IDs::CommandIDs::setMode1RootNote,
		IDs::CommandIDs::setMode2RootNote,
		IDs::CommandIDs::setModeViewed,
		IDs::CommandIDs::showModeInfo,
        IDs::CommandIDs::setMappingMode,
		IDs::CommandIDs::setMappingStyle,
        IDs::CommandIDs::showMapOrderEdit,
		IDs::CommandIDs::applyMapping,
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
		result.setInfo("Save Preset", "Save your custom layout to a file.", "Preset", 0);
		break;
	case IDs::CommandIDs::saveMode:
		result.setInfo("Save Mode", "Save the currently viewed mode.", "Preset", 0);
		break;
    case IDs::CommandIDs::showSaveMenu:
        result.setInfo("Show Saving Options", "Save current mode or whole preset.", "Preset", 0);
        break;
	case IDs::CommandIDs::loadPreset:
		result.setInfo("Load Preset", "Load a custom layout from a file.", "Preset", 0);
		break;
	case IDs::CommandIDs::loadMode:
		result.setInfo("Load Mode", "Load only the mode of a preset.", "Preset", 0);
		break;
    case IDs::CommandIDs::showLoadMenu:
        result.setInfo("Show Loading Options", "Load a mode or whole preset.", "Preset", 0);
        break;
	case IDs::CommandIDs::exportReaperMap:
		result.setInfo("Export for Reaper", "Exports the current preset to a MIDI Note Name text file for use in Reaper's piano roll.", "Preset", 0);
		break;
	case IDs::CommandIDs::exportAbletonMap:
		result.setInfo("Export for Ableton", "Exports the mode mapping to a MIDI file for to use in Ableton's piano roll for folding.", "Preset", 0);
		break;
    case IDs::CommandIDs::showExportMenu:
        result.setInfo("Show Export Options", "Shows different ways you can export a mode or preset.", "Preset", 0);
        break;
    case IDs::CommandIDs::showSettingsDialog:
        result.setInfo("Show Settings Dialog", "Change default directories", "Settings", 0);
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
	case IDs::CommandIDs::setModeViewed:
		result.setInfo("Set Mode Viewed", "Shows the mode slot on the keyboard.", "Keyboard", 0);
		break;
    case IDs::CommandIDs::showModeInfo:
        result.setInfo("Show Mode Info", "Shows information regarding the selected Mode.", "Mode", 0);
		break;
	case IDs::CommandIDs::setMappingStyle:
		result.setInfo("Mapping Style", "Choose a mapping style for remapping MIDI notes.", "Midi", 0);
		break;
    case IDs::CommandIDs::showMapOrderEdit:
        result.setInfo("Edit Mappings by Order", "Choose how to map modes with the order mapping method.", "Preset", 0);
        break;
	case IDs::CommandIDs::applyMapping:
		result.setInfo("Apply Mapping", "Map incoming MIDI notes to Mode 2 with the selected mapping style.", "Midi", 0);
		break;
    case IDs::CommandIDs::setMappingMode:
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
        case IDs::CommandIDs::showSettingsDialog:
        {
            showSettingsDialog();
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
			setMode2Root();
			break;
		}
		case IDs::CommandIDs::setModeViewed:
		{
			setModeView();
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
        case IDs::CommandIDs::showMapOrderEdit:
        {
            showMapOrderEditDialog();
            break;
        }
		case IDs::CommandIDs::applyMapping:
		{
			applyMap();
			break;
		}
		case IDs::CommandIDs::setMappingMode:
		{
			setMappingMode();
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

