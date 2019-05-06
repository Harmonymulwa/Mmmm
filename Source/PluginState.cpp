/*
  ==============================================================================

    PluginState.cpp
    Created: 7 Apr 2019 12:27:01pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "PluginState.h"

struct ModeScaleSorter
{
	// Scale first, then mode, then family

	static int compareElements(const Mode* t1, const Mode* t2)
	{
		int sz1 = t1->getScaleSize();
		int sz2 = t2->getScaleSize();

		int m1 = t1->getModeSize();
		int m2 = t2->getModeSize();

		String f1 = t1->getFamily();
		String f2 = t2->getFamily();

		if (sz1 < sz2) return -1;
		else if (sz1 > sz2) return 1;
		else
		{
			if (m1 < m2) return -1;
			else if (m1 > m2) return 1;
			else
			{
				if (f1 < f2) return  -1;
				else if (f1 > f2) return 1;
				else return 0;
			}
		}
	}
};

struct ModeModeSorter
{
	// Mode first, then scale, then family

	static int compareElements(const Mode* t1, const Mode* t2)
	{
		int sz1 = t1->getScaleSize();
		int sz2 = t2->getScaleSize();

		int m1 = t1->getModeSize();
		int m2 = t2->getModeSize();

		String f1 = t1->getFamily();
		String f2 = t2->getFamily();

		if (m1 < m2) return -1;
		else if (m1 > m2) return 1;
		else
		{
			if (sz1 < sz2) return -1;
			else if (sz1 > sz2) return 1;
			else
			{
				if (f1 < f2) return  -1;
				else if (f1 > f2) return 1;
				else return 0;
			}
		}
	}
};
struct ModeFamilySorter
{
	// Family first, then scale, then mode

	static int compareElements(const Mode* t1, const Mode* t2)
	{
		int sz1 = t1->getScaleSize();
		int sz2 = t2->getScaleSize();

		int m1 = t1->getModeSize();
		int m2 = t2->getModeSize();

		String f1 = t1->getFamily();
		String f2 = t2->getFamily();

		if (f1 < f2) return -1;
		else if (f1 > f2) return 1;
		else
		{
			if (sz1 < sz2) return -1;
			else if (sz1 > sz2) return 1;
			else
			{
				if (m1 < m2) return  -1;
				else if (m1 > m2) return 1;
				else return 0;
			}
		}
	}
};

template <class T>
void SuperVirtualKeyboardPluginState::add_array_to_node(ValueTree nodeIn, const Array<T>& arrayIn, Identifier arrayID, Identifier itemId)
{
	ValueTree arrayTree = ValueTree(arrayID);
	ValueTree item;

	for (int i = 0; i < arrayIn.size(); i++)
	{
		item = ValueTree(itemId);
		item.setProperty("Value", arrayIn[i], nullptr);
		arrayTree.addChild(item, i, nullptr);
	}

	nodeIn.addChild(arrayTree, -1, nullptr);
}

template <class T>
void SuperVirtualKeyboardPluginState::get_array_from_node(const ValueTree nodeIn, Array<T>& arrayIn, Identifier arrayID)
{
	ValueTree childArray = nodeIn.getChildWithName(arrayID);

	if (childArray.isValid())
	{
		for (int i = 0; i < childArray.getNumChildren(); i++)
		{
			arrayIn.add(childArray.getChild(i).getProperty("Value"));
		}
	}
}

void SuperVirtualKeyboardPluginState::add_array_to_node(ValueTree nodeIn, const Array<Colour>& arrayIn, Identifier arrayID, Identifier itemId)
{
	ValueTree arrayTree = ValueTree(arrayID);
	ValueTree item;

	for (int i = 0; i < arrayIn.size(); i++)
	{
		item = ValueTree(itemId);
		item.setProperty("Value", arrayIn[i].toString(), nullptr);
		arrayTree.addChild(item, i, nullptr);
	}

	nodeIn.addChild(arrayTree, -1, nullptr);
}

void SuperVirtualKeyboardPluginState::get_array_from_node(const ValueTree nodeIn, Array<Colour>& arrayIn, Identifier arrayID)
{
	ValueTree childArray = nodeIn.getChildWithName(arrayID);

	if (childArray.isValid())
	{
		for (int i = 0; i < childArray.getNumChildren(); i++)
		{
			arrayIn.add(Colour::fromString(childArray.getChild(i).getProperty("Value").toString()));
		}
	}
}

UndoManager* SuperVirtualKeyboardPluginState::getUndoManager()
{
	return undoManager.get();
}

OwnedArray<Mode>* SuperVirtualKeyboardPluginState::get_presets()
{
	return &presets;
}

Array<Array<Mode*>>* SuperVirtualKeyboardPluginState::get_presets_sorted()
{
	return &presetsSorted;
}

SvkPreset* SuperVirtualKeyboardPluginState::getCurrentPreset()
{
	return presetCurrent.get();
}

int SuperVirtualKeyboardPluginState::get_current_preset_index()
{
	Mode* mode;

	for (int i = 0; i < presets.size(); i++)
	{
		mode = presets.getUnchecked(i);

		if (mode == modeCurrent)
			return i;
	}

	return -1;
}

Mode* SuperVirtualKeyboardPluginState::getCurrentMode()
{
	return modeCurrent;
}

int SuperVirtualKeyboardPluginState::is_mode_in_presets(String stepsStringIn)
{
	int index = 0;

	for (auto mode : presets)
	{
		if (stepsStringIn == mode->getStepsString())
			return index;
		index++;
	}

	return 0;
}

void SuperVirtualKeyboardPluginState::setCurrentMode(int presetIndexIn)
{
	Mode* mode = presets.getUnchecked(presetIndexIn);
	
	if (mode)
	{
		modeCurrent = mode;
		presetCurrentNode.removeAllChildren(undoManager.get());
		presetCurrentNode.setProperty(IDs::libraryIndexOfMode, presetIndexIn, undoManager.get());
		presetCurrentNode.setProperty(IDs::presetHasCustomColor, false, nullptr);
		presetCurrentNode.addChild(modeCurrent->modeNode.createCopy(), 0, undoManager.get());
		// note that mode node is copied
	}
}

void SuperVirtualKeyboardPluginState::setCurrentMode(Mode* modeIn)
{
	if (modeIn)
	{
		modeCurrent = modeIn;
		presets.set(0, modeCurrent, true);
		presetCurrentNode.removeChild(0, undoManager.get());
		presetCurrentNode.setProperty(IDs::libraryIndexOfMode, 0, undoManager.get());
		presetCurrentNode.setProperty(IDs::presetHasCustomColor, false, nullptr);
		presetCurrentNode.addChild(modeCurrent->modeNode, 0, undoManager.get());
		// node that mode node is passed by reference
	}
}

void SuperVirtualKeyboardPluginState::set_current_key_settings(ValueTree pianoNodeIn)
{
	if (pianoNodeIn.isValid())
	{
		presetCurrentNode.removeChild(1, undoManager.get());
		presetCurrentNode.addChild(pianoNodeIn.createCopy(), 1, undoManager.get());
	}
}

bool SuperVirtualKeyboardPluginState::loadPreset()
{
	SvkPreset newPreset = SvkPreset::loadFromFile();

	if (newPreset.parentNode.isValid())
	{
		presetCurrent.reset(new SvkPreset(newPreset));
		return true;
	}

	return false;
}


//==============================================================================

void SuperVirtualKeyboardPluginState::createPresets()
{
	presets.clear();
	presets.add(new Mode(Array<int>({ 1 }), "Custom"));
	presets.add(new Mode(Array<int>({ 1, 2, 1, 2, 1 }), "Mavila"));
	presets.add(new Mode(Array<int>({ 2, 1, 2, 2, 1 }), "Father"));
	presets.add(new Mode(Array<int>({ 1, 1, 2, 1, 1, 1, 2 }), "Mavila"));
	presets.add(new Mode(Array<int>({ 1, 2, 1, 2, 1, 2, 1 }), "Dicot"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 2, 2, 1 }), "Machine"));
	presets.add(new Mode(Array<int>({ 2, 1, 2, 1, 2, 1, 2 }), "Orgone"));
	presets.add(new Mode(Array<int>({ 3, 1, 3, 1, 3, 1 }), "Augmented"));
	presets.add(new Mode(Array<int>({ 2, 2, 1, 2, 2, 2, 1 }), "Meantone"));
	presets.add(new Mode(Array<int>({ 2, 1, 2, 1, 2, 1, 2, 1 }), "Diminished"));
	presets.add(new Mode(Array<int>({ 2, 2, 1, 2, 2, 1, 2, 1 }), "Father"));
	presets.add(new Mode(Array<int>({ 2, 1, 2, 1, 2, 1, 2, 1, 1 }), "Orwell"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 2, 1, 2, 2, 1 }), "Hedgehog"));
	presets.add(new Mode(Array<int>({ 2, 2, 1, 2, 1, 2, 1, 2, 1 }), "Titanium"));
	presets.add(new Mode(Array<int>({ 3, 3, 3, 3, 3 }), "Blackwood"));
	presets.add(new Mode(Array<int>({ 3, 1, 3, 1, 3, 1, 3 }), "Orgone"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 2, 2, 2, 2, 1 }), "Porcupine"));
	presets.add(new Mode(Array<int>({ 1, 2, 1, 2, 1, 1, 2, 1, 1, 2, 1 }), "Orgone"));
	presets.add(new Mode(Array<int>({ 3, 3, 1, 3, 3, 3 }), "Gorgo"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 1, 2, 2, 2, 2, 1 }), "Mavila"));
	presets.add(new Mode(Array<int>({ 2, 1, 2, 1, 2, 2, 1, 2, 1, 2 }), "Lemba"));
	presets.add(new Mode(Array<int>({ 3, 3, 1, 3, 3, 3, 1 }), "Meantone"));
	presets.add(new Mode(Array<int>({ 3, 2, 3, 2, 3, 2, 2 }), "Maqamic"));
	presets.add(new Mode(Array<int>({ 2, 2, 1, 2, 2, 1, 2, 2, 2, 1 }), "Maqamic"));
	presets.add(new Mode(Array<int>({ 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 2 }), "Machine"));
	presets.add(new Mode(Array<int>({ 4, 4, 1, 4, 4, 1 }), "Bicycle"));
	presets.add(new Mode(Array<int>({ 3, 1, 3, 3, 1, 3, 3, 1 }), "Father"));
	presets.add(new Mode(Array<int>({ 3, 3, 2, 3, 3, 3, 2 }), "Meantone"));
	presets.add(new Mode(Array<int>({ 2, 3, 2, 2, 3, 2, 3, 2 }), "Keemun"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 2, 3, 2, 2, 2, 2 }), "Negri"));
	presets.add(new Mode(Array<int>({ 5, 2, 2, 5, 2, 2, 2 }), "Mavila"));
	presets.add(new Mode(Array<int>({ 3, 1, 3, 1, 3, 1, 3, 1, 3, 1 }), "Blackwood"));
	presets.add(new Mode(Array<int>({ 2, 3, 2, 2, 3, 2, 2, 3, 2 }), "Miracle"));
	presets.add(new Mode(Array<int>({ 4, 4, 1, 4, 4, 4, 1 }), "Superpyth"));
	presets.add(new Mode(Array<int>({ 3, 3, 3, 3, 2, 3, 3, 2 }), "Hedgehog"));
	presets.add(new Mode(Array<int>({ 3, 2, 3, 2, 3, 2, 3, 2, 2 }), "Orwell"));
	presets.add(new Mode(Array<int>({ 3, 3, 3, 1, 3, 3, 3, 3, 1 }), "Mavila"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 3, 2, 2, 3, 2, 2, 3 }), "Sephiroth"));
	presets.add(new Mode(Array<int>({ 5, 5, 4, 5, 5 }), "Godzilla"));
	presets.add(new Mode(Array<int>({ 5, 3, 5, 3, 5, 3 }), "Triforce"));
	presets.add(new Mode(Array<int>({ 4, 3, 3, 4, 3, 3, 4 }), "Dastgah-e Sehgah / Maqam Nairuz"));
	presets.add(new Mode(Array<int>({ 4, 4, 2, 4, 4, 4, 2 }), "Meantone"));
	presets.add(new Mode(Array<int>({ 4, 3, 4, 3, 4, 3, 3 }), "Mohajira"));
	presets.add(new Mode(Array<int>({ 4, 4, 3, 4, 4, 4, 3 }), "Meantone"));
	presets.add(new Mode(Array<int>({ 2, 3, 2, 2, 3, 2, 3, 2, 2, 3, 2 }), "Orgone"));
	presets.add(new Mode(Array<int>({ 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 3 }), "Injera"));
	presets.add(new Mode(Array<int>({ 5, 5, 3, 5, 5, 5, 3 }), "Meantone"));
	presets.add(new Mode(Array<int>({ 4, 3, 4, 3, 4, 3, 4, 3, 3, }), "Orwell"));
	presets.add(new Mode(Array<int>({ 2, 1, 1, 2, 1, 2, 1, 2, 1 }), "BP Lambda"));
	presets.add(new Mode(Array<int>({ 2, 1, 1, 2, 1, 2, 1, 1, 2 }), "BP Dur II"));

	// Connect to ValueTree structure
	for (int i = 0; i < presets.size(); i++)
	{
		modeLibraryNode.addChild(presets.getUnchecked(i)->modeNode, i, nullptr);
	}

	presetsSorted.clear();
	presetsSorted.resize(3);

	ModeScaleSorter scaleSort;
	ModeModeSorter modeSort;
	ModeFamilySorter famSort;

	Array<Mode*> sorting;
	for (int i = 1; i < presets.size(); i++)
	{
		sorting.add(presets.getUnchecked(i));
	}

	sorting.sort(scaleSort, false);
	presetsSorted.getReference(SortType::scaleSizeSort).addArray(sorting);

	sorting.sort(modeSort, false);
	presetsSorted.getReference(SortType::modeSizeSort).addArray(sorting);

	sorting.sort(famSort, false);
	presetsSorted.getReference(SortType::familySort).addArray(sorting);
}