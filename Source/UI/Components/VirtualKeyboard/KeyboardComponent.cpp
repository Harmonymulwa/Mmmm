/*
 ==============================================================================
 
 VirtualKeyboard.h
 Created: 14 Mar 2019 4:50:31pm
 Author:  Vincenzo
 
 ==============================================================================
 */

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "KeyboardComponent.h"

using namespace VirtualKeyboard;

Keyboard::Keyboard()
	: keyPositioner(this)
{
    setName("VirtualKeyboard");
	reset();
	initializeKeys();
	pianoNode = ValueTree(IDs::pianoNode);
    
    modeDefault = Mode("2 2 1 2 2 2 1", "Meantone");
	applyMode(mode);
    
    addMouseListener(this, true);
	setWantsKeyboardFocus(true);
    
    setSize(1000, 250);
    setOpaque(true);
}

Keyboard::Keyboard(ValueTree keyboardNodeIn, Mode* modeIn, NoteMap* inputFilterMapIn)
	: keyPositioner(this)
{
	initializeKeys(); // todo: load keyboard size
	restoreNode(keyboardNodeIn, true);
	
	modeDefault = Mode("2 2 1 2 2 2 1", "Meantone");
	mode = modeIn;
	applyMode(mode);
    
    addMouseListener(this, true);
	setWantsKeyboardFocus(true);

	setSize(1000, 250);
	setOpaque(true);
}

//===============================================================================================

void Keyboard::restoreNode(ValueTree pianoNodeIn, bool resetIfInvalid)
{
	if (pianoNodeIn.hasType(IDs::pianoNode))
	{
		pianoNode = pianoNodeIn;
        
		orientationSelected = pianoNode[IDs::keyboardOrientation];
		keyPlacementSelected = pianoNode[IDs::keyboardKeysStyle];
		lastKeyClicked = pianoNode[IDs::pianoLastKeyClicked];
		keySizeRatio = (float) pianoNode[IDs::pianoWHRatio];
        
        // safeguard
        if (keySizeRatio < 0.01)
            keySizeRatio = 0.25f;
        
        // unpack key data
        ValueTree keyTreeNode = pianoNode.getChildWithName(IDs::pianoKeyTreeNode);
        keys.clear();
        for (auto keyNode : keyTreeNode)
        {
            if (keyNode.hasType(IDs::pianoKeyNode))
                keys.add(new Key(keyNode));
        }
        
		// workaround until key data is finished
		if (keys.size() < 128)
			initializeKeys();

        // unpack color data
        
	}
	else
	{
		DBG("Invalid Piano Node.");
		if (resetIfInvalid)
		{
			reset();
			DBG("Keyboard is reset.");
		}
	}
}

void Keyboard::reset()
{
	pianoNode = ValueTree(IDs::pianoNode);

	setUIMode(UIMode::playMode);
	setOrientation(Orientation::horizontal);
	setKeyStyle(KeyPlacementType::nestedRight);
	setHighlightStyle(HighlightStyle::full);
	setVelocityBehavior(VelocityStyle::linear);
	setScrollingStyle(ScrollingStyle::smooth);

	setShowNoteLabels(false);
	setShowNoteNumbers(false);
	setShowFilteredNumbers(false);
	
	setMidiChannelOut(1);
	setVelocityFixed(1);
	setInputVelocityScaled(false);

	setKeySizeRatio(0.25f);
	setKeyOrderSizeScalar(1);

	modeDefault = Mode("2 2 1 2 2 2 1", "Meantone");
	mode = nullptr;
}

void Keyboard::initializeKeys(int size)
{
	keys.clear();

	for (int i = 0; i < size; i++)
	{
		keys.add(new Key(i));
		addAndMakeVisible(keys[i]);
	}

	keysOn = Array<int>();
	keysByMouseTouch = Array<int>();
}

//===============================================================================================

void Keyboard::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));   // clear the background
}

void Keyboard::paintOverChildren(juce::Graphics& g)
{
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);
}

void Keyboard::resized()
{
	keyPositioner.parentResized();

	keyWidth = keyPositioner.getKeyWidth();
	keyHeight = getHeight();

	keyPositioner.setLayerKeysWidthRatio(layerKeysWidthRatio);
	keyPositioner.setLayerKeysHeightRatio(layerKeysHeightRatio);

	for (int i = 0; i < keys.size(); i++)
	{
		Key* key = keys[i];
		keyPositioner.resizeKey(*key);
	}

	float keyOrder1Width = keyWidth * layerKeysWidthRatio;
	float adjacentStyleStepWidth;

	// Place Keys

	Array<int> modeKeyNums = keysOrder.getReference(0);
	for (int i = 0; i < numOrder0Keys; i++)
	{
		Key* modeKey = keys[modeKeyNums[i]];

		int keyX = (keyWidth + keyPositioner.getKeyGap()) * i;
		modeKey->setTopLeftPosition(keyX, 0);

		if (modeKey->step > 1)
		{
			adjacentStyleStepWidth = keyOrder1Width / (modeKey->step - 1);

			for (int s = 1; s < modeKey->step; s++)
			{
				Key* stepKey = keys[modeKey->keyNumber + s];

				if (s > 1 && keyPlacementSelected == KeyPlacementType::nestedRight)
				{
					keyX = modeKey->getX() + keyWidth + (keys[modeKey->keyNumber + 1]->getWidth() / 2) - stepKey->getWidth();
				}
				else if (keyPlacementSelected == KeyPlacementType::adjacent)
				{
					keyX = modeKey->getX() + keyWidth - (keyOrder1Width / 2) + (stepKey->order - 1) * adjacentStyleStepWidth;
				}
				else
				{
					keyX = modeKey->getX() + keyWidth - stepKey->getWidth() / 2;
				}

				stepKey->setTopLeftPosition(keyX, 0);
			}
		}
	}
}

//===============================================================================================

void Keyboard::mouseMove(const MouseEvent& e)
{
	if (uiModeSelected == UIMode::playMode)
	{
		keys[lastKeyOver]->repaint();

		Key* key = getKeyFromPositionMouseEvent(e);

		if (key)
		{
			lastKeyOver = key->keyNumber;
			key->repaint();
		}
	}
}

void Keyboard::mouseExit(const MouseEvent& e)
{
	if (uiModeSelected == UIMode::playMode)
	{
		int touchIndex = e.source.getIndex();
		if (keysByMouseTouch[touchIndex] > 0)
		{
			Key* key = keys[keysByMouseTouch[touchIndex]];

			if (!e.source.isDragging())
			{
				triggerKey(key->keyNumber, false);
				keysByMouseTouch.set(touchIndex, -1);
			}
		}

		Key* key = keys[lastKeyOver];
		key->repaint();
	}
}

void Keyboard::mouseDown(const MouseEvent& e)
{
	Key* key = getKeyFromPositionMouseEvent(e);

	if (uiModeSelected == UIMode::playMode)
	{
		if (key)
		{
			if (e.mods.isShiftDown() && !e.mods.isAltDown() && key->isClicked)
			{
				// note off
				lastKeyClicked = key->keyNumber;
				triggerKey(key->keyNumber, false);

			}
			//else if (pluginState->getMappingMode() == 3 && e.mods.isRightButtonDown() && pluginState->getModeViewedNum() == 1)
			//{
			//    if (mappingHelper->isWaitingForKeyInput())
			//    {
			//        //virtualKeyboard->highlightKeyForMapping(mappingHelper->getVirtualKeyToMap(), false);
			//    }

			//    bool allPeriods = true;  

			//    if (e.mods.isCtrlDown())
			//        allPeriods = false;

			//    mappingHelper->prepareKeyToMap(key->keyNumber, false);
			//    //virtualKeyboard->highlightKeyForMapping(key->keyNumber);
			//    DBG("Preparing to map key: " + String(key->keyNumber));
			//}
			else
			{
				// need to implement a velocity class
				triggerKey(key->keyNumber);
				lastKeyClicked = key->keyNumber;
				keysByMouseTouch.set(e.source.getIndex(), key->keyNumber);

				if (e.mods.isAltDown())
				{
					isolateLastNote();
				}

				//if (mappingHelper->isWaitingForKeyInput())
				//{
				//    //virtualKeyboard->highlightKeyForMapping(mappingHelper->getVirtualKeyToMap(), false);
				//    mappingHelper->cancelKeyMap();
				//}

			}
		}
		else
		{
			//if (mappingHelper->isWaitingForKeyInput())
			//{
			//    //virtualKeyboard->highlightKeyForMapping(mappingHelper->getVirtualKeyToMap(), false);
			//    mappingHelper->cancelKeyMap();
			//}
		}
	}
	else if (uiModeSelected == UIMode::editMode)
	{
		if (key)
		{
			if (e.mods.isRightButtonDown())
			{
				if (e.mods.isShiftDown())
				{
					resetKeyColorsInOrder(key->order);
				}
				else if (e.mods.isCtrlDown())
				{
					resetKeyColor(key->keyNumber);
				}
				else
				{
					resetDegreeColors(key->keyNumber);
				}
			}

			else if (e.mods.isShiftDown())
			{
				if (getKeyDegreeColor(key->scaleDegree).isOpaque())
					resetDegreeColors(key->scaleDegree);

				else if (getKeyColor(key->keyNumber).isOpaque())
				    resetKeyColor(key->keyNumber);

				setKeyColorOrder(key->order, Colour::fromString(getProperties()[IDs::colorSelected].toString()));
			}

			else if (e.mods.isCtrlDown())
			{
				key->setCustomColor(Colour::fromString(getProperties()[IDs::colorSelected].toString()));
			}

			else
			{
				setKeyColorDegree(key->scaleDegree, Colour::fromString(getProperties()[IDs::colorSelected].toString()));
			}
			
			updateKeyColors();
		}
	}
}

void Keyboard::mouseDrag(const MouseEvent& e)
{
	if (uiModeSelected == UIMode::playMode && !e.mods.isRightButtonDown())
	{
		Key* key = getKeyFromPositionMouseEvent(e);

		if (key)
		{
			//bool alreadyDown = false;
			int touchIndex = e.source.getIndex();
			int keyIndex = keysByMouseTouch[touchIndex];

			if (key->keyNumber != keyIndex)
			{
				triggerKey(keyIndex, false);
				triggerKey(key->keyNumber);
				lastKeyClicked = key->keyNumber;
				keysByMouseTouch.set(touchIndex, key->keyNumber);
			}
		}
	}

}

void Keyboard::mouseUp(const MouseEvent& e)
{
	if (uiModeSelected == UIMode::playMode)
	{
		int touchIndex = e.source.getIndex();
		int keyIndex = keysByMouseTouch[touchIndex];

		if (keyIndex > 0)
		{
			Key* key = keys[keyIndex];

			if (key)// && mappingHelper->getVirtualKeyToMap() != key->keyNumber)
			{
				keysByMouseTouch.set(touchIndex, -1);

				if (!shiftHeld)
				{
					triggerKey(key->keyNumber, false);
				}
			}
		}
	}
}

//===============================================================================================


bool Keyboard::keyStateChanged(bool isKeyDown)
{
	if (!KeyPress::isKeyCurrentlyDown(KeyPress::upKey) && upHeld)
	{
		upHeld = false;
		return true;
	}

	if (!KeyPress::isKeyCurrentlyDown(KeyPress::downKey) && downHeld)
	{
		downHeld = false;
		return true;
	}

	if (!KeyPress::isKeyCurrentlyDown(KeyPress::leftKey) && leftHeld)
	{
		leftHeld = false;
		return true;
	}

	if (!KeyPress::isKeyCurrentlyDown(KeyPress::rightKey) && rightHeld)
	{
		rightHeld = false;
		return true;
	}

	if (!KeyPress::isKeyCurrentlyDown(KeyPress::spaceKey) && spaceHeld)
	{
		spaceHeld = false;
		return true;
	}

	return false;
}

bool Keyboard::keyPressed(const KeyPress& key)
{
	if (KeyPress::isKeyCurrentlyDown(KeyPress::upKey) && !upHeld)
	{
		upHeld = true;
		if (uiModeSelected == UIMode::playMode)
		{
			if (shiftHeld && keysOn.size() > 0)
			{
				transposeKeysOnChromatically(1);
				repaint();
			}
		}
		return true;
	}

	if (KeyPress::isKeyCurrentlyDown(KeyPress::downKey) && !downHeld)
	{
		downHeld = true;
		if (uiModeSelected == UIMode::playMode)
		{
			if (shiftHeld && keysOn.size() > 0)
			{
				transposeKeysOnChromatically(-1);
				repaint();
			}
		}
		return true;
	}

	if (KeyPress::isKeyCurrentlyDown(KeyPress::leftKey) && !leftHeld)
	{
		leftHeld = true;
		if (uiModeSelected == UIMode::playMode)
		{
			if (shiftHeld && keysOn.size() > 0)
			{
				transposeKeysOnModally(-1);
			}
		}
		return true;
	}

	if (KeyPress::isKeyCurrentlyDown(KeyPress::rightKey) && !rightHeld)
	{
		rightHeld = true;
		if (uiModeSelected == UIMode::playMode)
		{
			if (shiftHeld && keysOn.size() > 0)
			{
				transposeKeysOnModally(1);
			}
		}
		return true;
	}

	if (KeyPress::isKeyCurrentlyDown(KeyPress::spaceKey) && !spaceHeld)
	{
		spaceHeld = true;
		if (uiModeSelected == UIMode::playMode)
		{
			if (shiftHeld && keysOn.size() > 0)
			{
				retriggerNotes();
			}
		}
		return true;
	}

	return false;
}

void Keyboard::modifierKeysChanged(const ModifierKeys& modifiers)
{

	if (!rightMouseHeld && modifiers.isRightButtonDown())
	{
		rightMouseHeld = true;
	}
	if (rightMouseHeld && !modifiers.isRightButtonDown())
	{
		rightMouseHeld = false;
	}

	if (!shiftHeld && modifiers.isShiftDown())
	{
		shiftHeld = true;
	}

	if (shiftHeld && !modifiers.isShiftDown())
	{
		shiftHeld = false;

		if (uiModeSelected == UIMode::playMode)
		{
			isolateLastNote();

			Key* key = keys[lastKeyClicked];

			if (!(key->isMouseOver() && key->isMouseButtonDownAnywhere()))
				triggerKey(lastKeyClicked, false);

		}
	}

	if (!altHeld && modifiers.isAltDown())
	{
		altHeld = true;
		if (uiModeSelected == UIMode::playMode)
		{
			isolateLastNote();
			repaint();
		}
	}

	else if (altHeld && !modifiers.isAltDown())
	{
		altHeld = false;
	}

	if (!ctrlHeld && modifiers.isCtrlDown())
	{
		ctrlHeld = true;
	}

	else if (ctrlHeld && !modifiers.isCtrlDown())
	{
		ctrlHeld = false;
	}
}

//===============================================================================================

//void Keyboard::timerCallback()
//{
	//if (hasDirtyKeys)
	//{
	//	for (auto key : *keys.get())
	//	{
	//		if (key->isDirty)
	//		{
	//			key->repaint();
	//		}
	//	}
	//}

	//hasDirtyKeys = false;
//}

//===============================================================================================

ValueTree Keyboard::getNode()
{
	return pianoNode;
}

Key* Keyboard::getKey(int keyNumIn)
{
	return keys[keyNumIn];
}

ValueTree Keyboard::getKeyNode(int keyNumIn)
{
	return keys[keyNumIn]->node;
}

Array<Key*> Keyboard::getKeysByOrder(int orderIn)
{
	Array<Key*> keysOut;
	Array<int> orderArray = keysOrder[orderIn];

	for (int i = 0; i < orderArray.size(); i++)
	{
		keysOut.add(keys[orderArray[i]]);
	}

	return keysOut;
}

Array<Key*> Keyboard::getKeysByScaleDegree(int degreeIn)
{
	Array<Key*> keysOut;
	Array<int> degreeArray = keysScaleDegree[degreeIn];

	for (int i = 0; i < degreeArray.size(); i++)
	{
		keysOut.add(keys[degreeArray[i]]);
	}

	return keysOut;
}

Array<Key*> Keyboard::getKeysByModalDegree(int degreeIn)
{
	Array<Key*> keysOut;
	Array<int> degreeArray = keysModalDegree[degreeIn];

	for (int i = 0; i < degreeArray.size(); i++)
	{
		keysOut.add(keys[degreeArray[i]]);
	}

	return keysOut;
}

int Keyboard::getLastKeyClicked()
{
	return lastKeyClicked;
}

Rectangle<int> Keyboard::getKeyArea(int midiNoteIn)
{
    // TODO
    return Rectangle<int>();
}

Rectangle<int> Keyboard::getKeyAreaRelative(int midiNoteIn)
{
    // TODO
    return Rectangle<int>();
}

int Keyboard::getKeyWidth()
{
	return keyWidth;
}

int Keyboard::getKeyHeight()
{
	return keyHeight;
}

int Keyboard::getPianoWidth(int heightIn)
{
	return numOrder0Keys * (heightIn * keySizeRatio + keyPositioner.getKeyGap());
}

float Keyboard::getKeySizeRatio()
{
	return keySizeRatio;
}

Point<int> Keyboard::getKeyOrderSize(int orderIn)
{
	//TODO (use the scalar)
 return Point<int>();
}

Key* Keyboard::getKeyFromPosition(Point<int> posIn)
{
	Key* keyOut = nullptr;

	if (reallyContains(posIn, true))
	{
		Component* c = getComponentAt(posIn);

		if (c->getName().startsWith("Key"))
			keyOut = dynamic_cast<Key*>(c);
	}

	return keyOut;
}

Key* Keyboard::getKeyFromPositionRelative(Point<int> posIn)
{
    //TODO
	return nullptr;
}

Key* Keyboard::getKeyFromPositionMouseEvent(const MouseEvent& e)
{
	Point<int> ep = Point<int>(e.getScreenX() - getScreenX(), e.getPosition().getY());
	return getKeyFromPosition(ep);
}

float Keyboard::getKeyVelocity(Key* keyIn, Point<int> posIn)
{
    //TODO
    return velocityFixed;
}

int Keyboard::getMidiChannelOut()
{
	return midiChannelOut;
}

//===============================================================================================

int Keyboard::getUIMode()
{
	return uiModeSelected;
}

int Keyboard::getOrientation()
{
    return orientationSelected;
}

int Keyboard::getKeyPlacementStyle()
{
	return keyPlacementSelected;
}

bool Keyboard::isShowingNoteNumbers()
{
	return showNoteNumbers;
}

bool Keyboard::isShowingFilteredNumbers()
{
    return showFilteredNoteNums;
}

bool Keyboard::isShowingNoteNames()
{
    return showNoteLabels;
}

int Keyboard::getHighlightStyle()
{
	return highlightSelected;
}

int Keyboard::getVelocityStyle()
{
	return velocitySelected;
}

float Keyboard::getVelocityFixed()
{
    return velocityFixed;
}

bool Keyboard::isInputVelocityScaled()
{
	return scaleMidiInputVelocity;
}

int Keyboard::getScrollingStyle()
{
	return scrollingSelected;
}


//===============================================================================================


void Keyboard::applyMode(Mode* modeIn)
{
	mode = modeIn;

	if (mode == nullptr)
		mode = &modeDefault;

	keysOrder.clear();
	keysOrder.resize(mode->getMaxStep());

	numOrder0Keys = mode->getKeyboardOrdersSize(0);
	keyPositioner.setNumModeKeys(numOrder0Keys);

	// resize arrays based on new properties
	keyColorsOrders.clear();
	keyColorsOrders.resize(mode->getMaxStep());
	keyColorsDegrees.clear();
	keyColorsDegrees.resize(mode->getScaleSize());

	int period = 0;

	for (int i = 0; i < keys.size(); i++)
	{
		Key* key = keys[i];
		period = i / mode->getScaleSize();

		key->order = mode->getOrder(i);
		key->scaleDegree = mode->getScaleDegree(i);
		key->modeDegree = mode->getModeDegree(i);
		key->step = mode->getNoteStep(i);

		keysOrder.getReference(key->order).add(key->keyNumber);
	}

	// needs to be implemented
	//keysOrder = mode->getNotesOrders();
	keysScaleDegree = mode->getNotesInScaleDegrees();
	keysModalDegree = mode->getNotesInModalDegrees();

	// add from back to front
	for (int o = 0; o < keysOrder.size(); o++)
	{
		Array<int>& orderArray = keysOrder.getReference(o);

		for (int keyNum = 0; keyNum < orderArray.size(); keyNum++)
		{
			Key* key = keys[(orderArray.getReference(keyNum))];
			//addAndMakeVisible(key);
			key->toFront(false);
		}
	}

	updateKeyColors();
	hasDirtyKeys = true;
}

void Keyboard::applyKeyData(ValueTree keyDataTreeIn)
{
	ValueTree keyNode;

	int numKeys = jmin(keys.size(), keyDataTreeIn.getNumChildren());

	for (int i = 0; i < numKeys; i++)
	{
		Key* key = keys[i];
		key->applyParameters(keyDataTreeIn.getChild(i));
	}
}

void Keyboard::displayKeyboardState(MidiKeyboardState* keyboardStateIn)
{
	if (externalKeyboardToDisplay != nullptr)
		externalKeyboardToDisplay->removeListener(this);

	externalKeyboardToDisplay = keyboardStateIn;

	if (externalKeyboardToDisplay != nullptr)
		externalKeyboardToDisplay->addListener(this);
}

//===============================================================================================

void Keyboard::setUIMode(int uiModeIn)
{
    uiModeSelected = uiModeIn;
	pianoNode.setProperty(IDs::pianoUIMode, uiModeSelected, nullptr);
	
	if (uiModeIn == playMode)
	{
		updateKeyColors();
		repaint();
	}

	// Do stuff
}

void Keyboard::setOrientation(int orientationIn)
{
	orientationSelected = orientationIn;
	pianoNode.setProperty(IDs::keyboardOrientation, orientationSelected, nullptr);

	// do stuff
}

void Keyboard::setNumRows(int numRowsIn)
{
    numRows = jlimit(1, 16, numRowsIn);
    pianoNode.setProperty(IDs::keyboardNumRows, numRows, nullptr);
    applyMode(mode);
}

void Keyboard::setKeyStyle(int placementIn)
{
	keyPlacementSelected = placementIn;
	keyPositioner.setKeyPlacement(keyPlacementSelected);
	
	pianoNode.setProperty(IDs::keyboardKeysStyle, keyPlacementSelected, nullptr);
}

void Keyboard::setHighlightStyle(int styleIn)
{
	highlightSelected = styleIn;
	pianoNode.setProperty(IDs::keyboardHighlightStyle, highlightSelected, nullptr);
}

void Keyboard::setVelocityBehavior(int behaviorNumIn, bool scaleInputVelocity)
{
	velocitySelected = behaviorNumIn;
	pianoNode.setProperty(IDs::pianoVelocityBehavior, velocitySelected, nullptr);

	scaleMidiInputVelocity = scaleInputVelocity;
	pianoNode.setProperty(IDs::pianoVelocityScaleInput, scaleMidiInputVelocity, nullptr);
}

void Keyboard::setScrollingStyle(int scrollingStyleIn)
{
	scrollingSelected = scrollingStyleIn;
	pianoNode.setProperty(IDs::keyboardScrollingStyle, scrollingSelected, nullptr);

	// do stuff?
}

void Keyboard::setMidiChannelOut(int midiChannelOutIn)
{
	midiChannelOut = jlimit(1, 16, midiChannelOutIn);
	pianoNode.setProperty(IDs::keyboardMidiChannel, midiChannelOut, nullptr);
}

void Keyboard::setVelocityFixed(float velocityIn)
{
	velocityFixed = velocityIn;
	pianoNode.setProperty(IDs::pianoVelocityValue, velocityFixed, nullptr);
}

void Keyboard::setInputVelocityScaled(bool shouldBeScaled)
{
	scaleMidiInputVelocity = shouldBeScaled;
	pianoNode.setProperty(IDs::pianoVelocityScaleInput, scaleMidiInputVelocity, nullptr);
}

void Keyboard::setShowNoteNumbers(bool shouldShowNumbers)
{
	showNoteNumbers = shouldShowNumbers;
	pianoNode.setProperty(IDs::pianoKeyShowNumber, showNoteNumbers, nullptr);

	for (int i = 0; i < keys.size(); i++)
	{
		Key* key = keys[i];
		key->showNoteNumber = shouldShowNumbers;
		key->repaint();
	}
}

void Keyboard::setShowFilteredNumbers(bool shouldShowNumbers)
{
	showFilteredNoteNums = shouldShowNumbers;
	pianoNode.setProperty(IDs::pianoKeysShowFilteredNotes, showFilteredNoteNums, nullptr);
}

void Keyboard::setShowNoteLabels(bool shouldShowPitchNames)
{
	showNoteLabels = shouldShowPitchNames;
	pianoNode.setProperty(IDs::keyboardShowsNoteLabels, showNoteLabels, nullptr);
}

// should be replaced, but keeping here just in case
void Keyboard::setKeyProportions(Key* keyIn)
{
	float spread = 4;
	float stepHeight;
	float keyHeight;
	float width;

	switch (keyPlacementSelected)
	{

	case KeyPlacementType::nestedCenter:
		break;



	case KeyPlacementType::flat:

		stepHeight = 0.55 + (keyIn->step - 2) / 100.0f * spread;
		keyHeight = stepHeight - stepHeight * (keyIn->order - 1) / keyIn->step;
		width = 0.8;

		break;

	default: // Nested Right

		stepHeight = 0.55 + (keyIn->step - 2) / 100.0f * spread;
		keyHeight = stepHeight - stepHeight * (keyIn->order - 1) / keyIn->step;
		width = 0.8 - (keyIn->order - 1) * 0.1;

		break;
	}
    
    if (keyIn->order == 0)
    {
        keyHeight = 1;
        width = 1;
    }

	keyIn->setSize(keyHeight, keyWidth);
}

void Keyboard::setLastKeyClicked(int keyNumIn)
{
	lastKeyClicked = keyNumIn;
	pianoNode.setProperty(IDs::pianoLastKeyClicked, lastKeyClicked, nullptr);
}

void Keyboard::setInputNoteMap(NoteMap* noteMapIn)
{
    noteMapOnDisplay = noteMapIn;
}

//===============================================================================================

void Keyboard::selectKeyToMap(Key* keyIn, bool mapAllPeriods)
{
    // do stuff
}

void Keyboard::highlightKey(int keyNumberIn, Colour colorIn, int blinkRateMs)
{
	// do stuff

}

void Keyboard::highlightKeys(Array<int> keyNumsIn, Colour colorIn, int blinkRateMs)
{
	// do stuff
}

//===============================================================================================

Colour Keyboard::getKeyColor(int keyNumIn)
{
	Colour c;
	Key* key = keys[keyNumIn];

	if (key->getCustomColor().isOpaque()) // need to fix this for switching modes
		c = key->getCustomColor();

	else if (keyColorsDegrees[key->scaleDegree].isOpaque())
			c = keyColorsDegrees[key->scaleDegree];

	else if (keyColorsOrders[key->order].isOpaque())
			c = keyColorsOrders[key->order];
	
	else 
		c = colorsDefaultOrders[key->order];
    
  return c;
}

Colour Keyboard::getKeyOrderColor(int orderIn)
{
	Colour c;

	if (keyColorsOrders[orderIn].isOpaque())
			c = keyColorsOrders[orderIn];

	else
		c = colorsDefaultOrders[orderIn];

	return c;
}

Colour Keyboard::getKeyDegreeColor(int degIn)
{
	Colour c;

	if (keyColorsDegrees[degIn].isOpaque())
			c = keyColorsDegrees[degIn];
	
	else // this deg->order conversion is probably wrong
		c = colorsDefaultOrders[mode->getOrder(degIn)];

	return c;
}

void Keyboard::setKeyColorOrder(int orderIn, Colour colorIn)
{
	keyColorsOrders.set(orderIn, colorIn);
}

void Keyboard::setKeyColorDegree(int tuningDegreeIn, Colour colorIn)
{
	keyColorsDegrees.set(tuningDegreeIn, colorIn);
}

void Keyboard::updateKeyColors()
{
	for (int i = 0; i < keys.size(); i++)
	{
		Key* key = keys[i];
		key->setDefaultColor(getKeyColor(i));
	}
}

void Keyboard::resetKeyColorsInOrder(int orderIn)
{
	keyColorsOrders.clear();
	keyColorsOrders.resize(mode->getMaxStep());
}

void Keyboard::resetDegreeColors(int tuningDegreeIn)
{
	keyColorsDegrees.clear();
	keyColorsDegrees.resize(mode->getScaleSize());
	
}

void Keyboard::resetKeyColor(int keyNumberIn)
{
	Key* key = keys[keyNumberIn];
	key->setDefaultColor(Colours::transparentBlack);
}

void Keyboard::resetKeyColors()
{
	for (int i = 0; i < keys.size(); i++)
	{
		resetKeyColor(i);
	}
}

//===============================================================================================

void Keyboard::setKeySizeRatio(float keySizeRatioIn)
{
	keySizeRatio = keySizeRatioIn;
	pianoNode.setProperty(IDs::pianoWHRatio, keySizeRatio, nullptr);
}

void Keyboard::setKeyWidthSize(int widthSizeIn)
{
	// do stuff
}

void Keyboard::setKeyOrderSizeScalar(float scalarIn)
{
	// do stuff
}

//===============================================================================================

void Keyboard::allNotesOff()
{
    MidiKeyboardState::allNotesOff(midiChannelOut);

	for (int i = 0; i < keys.size(); i++)
	{
			triggerKey(i, false);
	}
}

void Keyboard::isolateLastNote()
{
    if (lastKeyClicked >= 0 && lastKeyClicked < 128)
    {
		for (int i = 0; i < keys.size(); i++)
		{
			if (i != lastKeyClicked)
				triggerKey(i, false);
		}
    }
}

void Keyboard::retriggerNotes()
{
	Array<int> retrigger = Array<int>(keysOn);

	allNotesOff();

	for (int i = 0; i < retrigger.size(); i++)
		noteOn(midiChannelOut, retrigger[i], 0.6f);
}

void Keyboard::triggerKey(int keyNumberIn, bool doNoteOn, float velocity)
{
	if (keyNumberIn < 0 || keyNumberIn > keys.size())
		return;

	Key* key = keys[keyNumberIn];

	if (doNoteOn)
	{
		noteOn(midiChannelOut, keyNumberIn, velocity);
		key->isClicked = true;
		keysOn.addIfNotAlreadyThere(keyNumberIn);
	}
	else
	{
		noteOff(midiChannelOut, keyNumberIn, 0);
		key->isClicked = false;
		keysOn.removeAllInstancesOf(keyNumberIn);
	}

	key->repaint();
}


void Keyboard::triggerKeys(Array<int> keyNumbers, bool doNoteOn, float velocity)
{
	for (int i = 0; i < keyNumbers.size(); i++)
	{
		triggerKey(keyNumbers[i], doNoteOn, velocity);
	}
}

int Keyboard::getOrderOfNotesOn()
{
	float orderSum = 0;
	int orderDetected;
    
    for (int i = 0; i < keysOn.size(); i++)
    {
		Key* key = keys[keysOn[i]];
		orderSum += (key->order + 1);
    }

	orderDetected = orderSum / keysOn.size();

	if (orderDetected == (int)orderDetected)
		orderDetected -= 1;
	else
		orderDetected = -1;

    return orderDetected;
}

int Keyboard::transposeKeyModally(int keyNumIn, int stepsIn)
{
	Key* key = keys[keyNumIn];
	Array<int> orderArray = keysOrder[key->order];
    int newKey = -1;
    
    for (int i = 0; i < orderArray.size(); i++)
	{
		if (key->keyNumber == orderArray.getUnchecked(i))
        {
            newKey = i;
            break;
        }
    }
    
    newKey += stepsIn;
    
	if (newKey >= 0 && newKey < orderArray.size())
		return orderArray[newKey];
	else
		return -1;
}

int Keyboard::transposeKeyChromatically(int keyNumIn, int stepsIn)
{
    int newKey = keyNumIn + stepsIn;
    
	if (newKey >= 0 && newKey <= keys.size())
		return newKey;
	else
		return -1;
}

void Keyboard::transposeKeysOnModally(int modalStepsIn, bool needsSameOrder, bool useLastClickedRoot)
{
	int keyRoot;
	int rootOrder;

	int newKey;
	Array<int> oldKeys = Array<int>(keysOn);
	Array<int> newKeys;

	if (needsSameOrder)
	{
		rootOrder = keys[keysOn[0]]->order;

		if (rootOrder != getOrderOfNotesOn())
			return;

		for (int i = 0; i < oldKeys.size(); i++)
		{
			newKey = transposeKeyModally(oldKeys[i], modalStepsIn);

			if (newKey > -1)
				newKeys.add(newKey);
		}
	}

	else
	{
		int stepsToUse; // TODO: implement this so that the chord quality is retained

		if (useLastClickedRoot)
		{
			keyRoot = lastKeyClicked;
		}
		else // find lowest key
		{
			keyRoot = keys.size();

			for (int i = 0; i < keysOn.size(); i++)
			{
				if (keysOn[i] < keyRoot)
					keyRoot = keysOn[i];
			}
		}

		rootOrder = keys[keyRoot]->order;

		for (int i = 0; i < oldKeys.size(); i++)
		{
			newKey = transposeKeyModally(oldKeys[i], modalStepsIn);

			if (newKey > -1)
				newKeys.add(newKey);
		}
	}

	// TODO: implement velocities further
	lastKeyClicked = transposeKeyModally(lastKeyClicked, modalStepsIn);
	triggerKeys(oldKeys, false);
	triggerKeys(newKeys);
}

void Keyboard::transposeKeysOnChromatically(int stepsIn)
{
    Array<int> oldKeys = Array<int>(keysOn);
	Array<int> newKeys;
    int keyNum;
    int newKey;

    float velocity;
    
    for (int i = 0; i < oldKeys.size(); i++)
    {
		keyNum = oldKeys.getUnchecked(i);

		Key* key = keys[keyNum];
        velocity = key->velocity;

		newKey = transposeKeyChromatically(keyNum, stepsIn);
        
		if (newKey > -1)
			newKeys.add(newKey);
    }
    
	// TODO: implement velocities further
	lastKeyClicked = transposeKeyChromatically(lastKeyClicked, stepsIn);
	triggerKeys(oldKeys, false);
	triggerKeys(newKeys);
}

//===============================================================================================

void Keyboard::handleNoteOn(MidiKeyboardState* source, int midiChannel, int midiNote, float velocity)
{
    int keyTriggered = midiNote;

    if (uiModeSelected == playMode)
    {
        Key* key = keys[keyTriggered];
		key->exPressed = true;
		key->exInputColor = Colour(powf((midiChannel - 1.0 / 16), 2) * 128, 0.667f, 0.667f, 1.0f);
		key->isDirty = true;
		hasDirtyKeys = true;
    }
}

void Keyboard::handleNoteOff(MidiKeyboardState* source, int midiChannel, int midiNote, float velocity)
{
    Key* key;
    int keyTriggered = midiNote;

    if (uiModeSelected == playMode)
    {
        Key* key = keys[keyTriggered];
		key->exPressed = false;
		key->isDirty = true;
		hasDirtyKeys = true;
    }
}
