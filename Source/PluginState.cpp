/*
  ==============================================================================

    PluginState.cpp
    Created: 7 Apr 2019 12:27:01pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "PluginState.h"

SvkPluginState::SvkPluginState()
{
    appCmdMgr.reset(new ApplicationCommandManager());
    undoManager.reset(new UndoManager());

	pluginStateNode = ValueTree(IDs::pluginStateNode);

	pluginSettings.reset(new SvkPluginSettings());
	pluginSettingsNode = pluginSettings->pluginSettingsNode;
	pluginStateNode.addChild(pluginSettingsNode, -1, nullptr);
    
    midiProcessor.reset(new SvkMidiProcessor());
    midiSettingsNode = midiProcessor->midiSettingsNode;
    pluginStateNode.addChild(midiSettingsNode, -1, nullptr);

	presetManager.reset(new SvkPresetManager(pluginSettingsNode));
	modeLibraryNode = presetManager->modeLibraryNode;
	presetManager->addChangeListener(this);

	virtualKeyboard.reset(new VirtualKeyboard::Keyboard(midiProcessor.get()));
	virtualKeyboard->addListener(midiProcessor.get()); // generates MIDI from UI
	pianoNode = virtualKeyboard->getNode();

	modeMapper.reset(new ModeMapper());

	pluginStateNode.addChild(pianoNode, -1, nullptr);

	textFilterIntOrSpace.reset(new TextFilterIntOrSpace());
	textFilterInt.reset(new TextFilterInt());
    
    updatePluginToPresetLoaded();
}

void SvkPluginState::recallState(ValueTree nodeIn)
{
    if (nodeIn.hasType(IDs::presetNode))
    {
        presetManager->loadPreset(nodeIn);
        return;
    }
    
    ValueTree childNode = nodeIn.getChildWithName(IDs::presetNode);
    
    if (childNode.hasType(IDs::presetNode))
    {
        presetManager->loadPreset(childNode);
    }
    else
    {
        updatePluginToPresetLoaded();
        return;
    }
    
    childNode = nodeIn.getChildWithName(IDs::pluginSettingsNode);
    
    if (childNode.isValid())
        pluginSettings->restoreNode(childNode);
    
    childNode = nodeIn.getChildWithName(IDs::pluginEditorNode);
    
    if (childNode.isValid())
        pluginEditorNode = childNode.createCopy();
    
    childNode = nodeIn.getChildWithName(IDs::pianoNode);
    
    if (childNode.isValid())
        virtualKeyboard->restoreDataNode(childNode);
}

//==============================================================================

SvkPresetManager* SvkPluginState::getPresetManager()
{
	return presetManager.get();
}

SvkMidiProcessor* SvkPluginState::getMidiProcessor()
{
	return midiProcessor.get();
}

SvkPluginSettings* SvkPluginState::getPluginSettings()
{
	return pluginSettings.get();
}

ApplicationCommandManager* SvkPluginState::getAppCmdMgr()
{
	return appCmdMgr.get();
}

UndoManager* SvkPluginState::getUndoManager()
{
	return undoManager.get();
}

VirtualKeyboard::Keyboard* SvkPluginState::getKeyboard()
{
	return virtualKeyboard.get();
}

ModeMapper* SvkPluginState::getModeMapper()
{
	return modeMapper.get();
}

NoteMap* SvkPluginState::getMidiInputMap()
{
    return midiProcessor->getMidiInputFilter()->getNoteMap();
}
NoteMap* SvkPluginState::getMidiOutputMap()
{
    return midiProcessor->getMidiOutputFilter()->getNoteMap();
}

SvkPreset* SvkPluginState::getPresetinSlot(int slotNumIn)
{
    return presetManager->getPresetLoaded(slotNumIn);
}

SvkPreset* SvkPluginState::getPresetViewed()
{
    return presetViewed;
}

Mode* SvkPluginState::getModeInSlot(int slotNumIn)
{
    return presetManager->getModeInSlots(presetSlotNumViewed, slotNumIn);
}

Mode* SvkPluginState::getModeViewed()
{
    return modeViewed;
}

//==============================================================================

bool SvkPluginState::isPresetEdited()
{
    return presetEdited;
}

void SvkPluginState::setPresetViewed(int presetViewedIn)
{
    presetSlotNumViewed = presetViewedIn;
    presetViewed = &presetManager->getPresetLoaded()[presetSlotNumViewed];
}

void SvkPluginState::setModeViewed(int modeViewedIn)
{
    modeViewedNum = modeViewedIn;
    modeViewed = presetManager->getModeInSlots(presetSlotNumViewed, modeViewedIn);
}

void SvkPluginState::changeModeInCurrentSlot(int modeLibraryIndexIn)
{
    ValueTree modeNode = presetManager->getModeInLibrary(modeLibraryIndexIn);
    presetViewed->setModeSlot(modeNode, modePresetSlotNum);
    
    presetEdited = true;
}

void SvkPluginState::addModeToNewSlot(int modeLibraryIndexIn)
{
    ValueTree modeNode = presetManager->getModeInLibrary(modeLibraryIndexIn);
    addModeToNewSlot(modeNode);
}

void SvkPluginState::addModeToNewSlot(ValueTree modePresetNodeIn)
{
    presetViewed->addMode(modePresetNodeIn);
    
    presetEdited = true;
}

void SvkPluginState::setModeViewedSlotNumber(int slotNumberIn)
{
    if (modeViewed) // Mode index 1, AKA Mode2
    {
        presetViewed->setMode2SlotNumber(slotNumberIn);
    }
    else // Mode1
    {
        presetViewed->setMode1SlotNumber(slotNumberIn);
    }
    
    presetEdited = true;
}

void SvkPluginState::setModeViewedRoot(int rootNoteIn)
{
    rootNoteIn = totalModulus(rootNoteIn, 128);
    midiProcessor->setRootNote(rootNoteIn);
    modeViewed->setRootNote(rootNoteIn);
    
    presetEdited = true;
}

void SvkPluginState::setMidiInputMap(NoteMap noteMapIn)
{
	midiProcessor->setMidiInputMap(noteMapIn);
}

void SvkPluginState::setMidiOutputMap(NoteMap noteMapIn)
{
	midiProcessor->setMidiOutputMap(noteMapIn);
}

void SvkPluginState::updatePluginToPresetLoaded()
{
    DBG("Updating plugin from this loaded preset:");

    presetWorking = SvkPreset(*presetManager->getPresetLoaded());
	presetWorkingNode = presetWorking.parentNode;
    presetEdited = false;

	DBG(presetManager->getPresetLoaded()->parentNode.toXmlString());

	pluginStateNode.removeChild(pluginStateNode.getChildWithName(IDs::presetNode), nullptr);
	pluginStateNode.addChild(presetWorkingNode, -1, nullptr);

    modeLoaded->restoreNode(presetWorking.theModeNode, false);
	modePresetNode = modeLoaded->modeNode;
	
	midiProcessor->restoreFromNode(presetWorking.theMidiSettingsNode);
    midiProcessor->setScaleSize(modeLoaded->getScaleSize());
	
	//virtualKeyboard->restoreDataNode(presetWorking.theKeyboardNode);
    
    sendChangeMessage();
}

void SvkPluginState::commitPresetChanges()
{
    presetWorking.updateModeNode(modeLoaded->modeNode);
    presetWorking.updateKeyboardNode(pianoNode);
    presetWorking.updateMidiNode(midiProcessor->midiSettingsNode);
    presetManager->commitPresetNode(presetWorking.parentNode);
    
    presetWorking = SvkPreset(*presetManager->getPresetLoaded());
	presetWorkingNode = presetWorking.parentNode;
    presetEdited = false;

	modePresetNode = modeLoaded->modeNode;
}

bool SvkPluginState::savePresetViewedToFile()
{
    commitPresetChanges();
    return presetManager->savePresetToFile(presetSlotNumViewed);
}

bool SvkPluginState::loadPresetFromFile(bool replaceViewed)
{
	int slotNumber = presetSlotNumViewed;

	if (!replaceViewed)
		slotNumber = presetManager->getNumPresetsLoaded();

    return presetManager->loadPreset(slotNumber);
}

void SvkPluginState::changeListenerCallback(ChangeBroadcaster* source)
{
	// Preset loaded
	if (source == presetManager.get())
	{
        updatePluginToPresetLoaded();
	}
}
