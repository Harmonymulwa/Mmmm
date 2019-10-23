/*
  ==============================================================================

    VirtualKeyboardKey.cpp
    Created: 17 Apr 2019 2:11:53pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "VirtualKeyboardKey.h"

using namespace VirtualKeyboard;

Key::Key()
{
    setName("Key");
    node = ValueTree(IDs::pianoKeyNode);
}

Key::Key(int keyNumIn)
: Key()
{
    keyNumber = keyNumIn;
}

Key::Key(int keyNumIn, int orderIn, int scaleDegreeIn, int modeDegreeIn, int stepIn,
	String pitchNameIn, int widthModIn, int heightModIn, int xOff, int yOff,
	bool showNoteNumIn, bool showNoteNameIn, Colour colorIn)
: Key()
{
	keyNumber = keyNumIn;
	order = orderIn;
	scaleDegree = scaleDegreeIn;
	modeDegree = modeDegreeIn;
	step = stepIn;
	pitchName = pitchNameIn;
	widthMod = widthModIn;
	heightMod = heightModIn;
	xOffset = xOff;
	yOffset = yOff;
	showNoteNumber = showNoteNumIn;
	showNoteName = showNoteNameIn;
	color = colorIn;
}

Key::Key(ValueTree keyNodeIn)
    : Key()
{
    if (keyNodeIn.hasType(IDs::pianoKeyNode))
    {
        applyParameters(keyNodeIn);
        node = keyNodeIn;
    }
}

Key::Key(const Key& keyToCopy)
: Key(keyToCopy.keyNumber, keyToCopy.order, keyToCopy.scaleDegree, keyToCopy.modeDegree, keyToCopy.step,
      keyToCopy.pitchName, keyToCopy.widthMod, keyToCopy.heightMod, keyToCopy.xOffset, keyToCopy.yOffset,
      keyToCopy.showNoteNumber, keyToCopy.showNoteName, keyToCopy.color)
{
    node = keyToCopy.node.createCopy();
}


void Key::applyParameters(ValueTree nodeIn)
{
	Identifier id;

	for (int i = 0; i < nodeIn.getNumProperties(); i++)
	{
		id = nodeIn.getPropertyName(i);

		if (id == IDs::pianoKeyNumber)
			keyNumber = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyWidthMod)
			widthMod = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyHeightMod)
			heightMod = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyXOffset)
			xOffset = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyYOffset)
			yOffset = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyShowNumber)
			showNoteNumber = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyShowName)
			showNoteName = nodeIn.getProperty(id);
		else if (id == IDs::pianoKeyColor)
			color = Colour::fromString(nodeIn.getProperty(id).toString());
	}
}

void Key::paint(Graphics& g)
{
    Colour colorToUse = color;
    float contrast = 0;
    
    if (isMouseOver())
    {
        contrast = 0.5f;
    }
    
    if (isPressed)
    {
        contrast = 1;
    }
    else if (exInputColor.isOpaque())
    {
        colorToUse = exInputColor;
    }
    
    g.setColour(colorToUse.contrasting(contrast));
    g.fillRect(getLocalBounds());
}

void Key::resized()
{
    
}

void Key::mouseExit(const MouseEvent& e)
{
    repaint();
}
