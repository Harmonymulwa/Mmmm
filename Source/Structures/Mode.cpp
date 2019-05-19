/*
==============================================================================

ModeLayout.cpp
Created: 19 Apr 2019 11:15:14pm
Author:  Vincenzo

==============================================================================
*/

#pragma once

#include "Mode.h"


Mode::Mode()
{
	stepsString = "1";
	family = "undefined";
	rootNote = 60;

	steps = parse_steps(stepsString);
	ordersDefault = steps_to_orders(steps);
	mosClass = interval_sizes(steps);
	scaleSize = ordersDefault.size();
	modeSize = steps.size();
	name = getDescription();
	offset = getOffset() * -1;

	orders = expand_orders(ordersDefault, offset);
	modeDegrees = orders_to_modeDegrees(ordersDefault);
	scaleDegrees = scale_degrees(scaleSize, offset);
	keyboardOrdersSizes = interval_sizes(orders);
	updateStepsOfOrders();

	init_node();
}

Mode::Mode(String stepsIn, String familyIn, int rootNoteIn)
{
	stepsString = stepsIn;
	family = familyIn;
	rootNote = rootNoteIn;

	steps = parse_steps(stepsIn);
	ordersDefault = steps_to_orders(steps);
	mosClass = interval_sizes(steps);
	scaleSize = ordersDefault.size();
	modeSize = steps.size();
    name = getDescription();
	offset = getOffset() * -1;

	orders = expand_orders(ordersDefault, offset);
	modeDegrees = orders_to_modeDegrees(orders);
	scaleDegrees = scale_degrees(scaleSize, offset);
	keyboardOrdersSizes = interval_sizes(orders);
	updateStepsOfOrders();

	init_node();
}

Mode::Mode(Array<int> stepsIn, String familyIn, int rootNoteIn)
{
	stepsString = steps_to_string(stepsIn);
	family = familyIn;
	rootNote = rootNoteIn;

	steps = stepsIn;
	ordersDefault = steps_to_orders(steps);
	mosClass = interval_sizes(steps);
	scaleSize = ordersDefault.size();
	modeSize = steps.size();
	name = getDescription();
	offset = getOffset() * -1;

	orders = expand_orders(ordersDefault, offset);
	modeDegrees = orders_to_modeDegrees(orders);
	scaleDegrees = scale_degrees(scaleSize, offset);
	keyboardOrdersSizes = interval_sizes(orders);
    updateStepsOfOrders();
    
	init_node();
}

Mode::~Mode() {}


void Mode::init_node()
{
	modeNode = ValueTree(IDs::modePresetNode);

	modeNode.setProperty(IDs::modeName, name, nullptr);
	modeNode.setProperty(IDs::scaleSize, scaleSize, nullptr);
	modeNode.setProperty(IDs::modeSize, modeSize, nullptr);
	modeNode.setProperty(IDs::stepString, stepsString, nullptr);
	modeNode.setProperty(IDs::family, family, nullptr);
	modeNode.setProperty(IDs::factoryPreset, false, nullptr);
}

void Mode::restore_from_node(ValueTree nodeIn, int rootNoteIn)
{
	if (nodeIn.hasType(IDs::modePresetNode))
	{
		modeNode.copyPropertiesAndChildrenFrom(nodeIn, nullptr);

		name = modeNode[IDs::modeName];
		scaleSize = modeNode[IDs::scaleSize];
		modeSize = modeNode[IDs::modeSize];
		stepsString = modeNode[IDs::stepString];
		family = modeNode[IDs::family];

		rootNote = rootNoteIn;
		offset = getOffset() * -1;

		steps = parse_steps(stepsString);
		mosClass = interval_sizes(steps);
		ordersDefault = steps_to_orders(steps);
		orders = expand_orders(ordersDefault, offset);
		modeDegrees = orders_to_modeDegrees(orders);
		scaleDegrees = scale_degrees(scaleSize, offset);
		updateStepsOfOrders();
	}
}

String Mode::setFamily(String nameIn)
{
	family = nameIn;
	modeNode.setProperty(IDs::family, family, nullptr);

	name = getDescription();
	modeNode.setProperty(IDs::modeName, name, nullptr);

	return name;
}

void Mode::setRootNote(int rootNoteIn)
{
	if (rootNote != rootNoteIn)
	{
		rootNote = rootNoteIn;
		offset = getOffset() * -1;
		orders = expand_orders(ordersDefault, offset);
		modeDegrees = orders_to_modeDegrees(orders);
		scaleDegrees = scale_degrees(scaleSize, offset);
		keyboardOrdersSizes = interval_sizes(orders);
		updateStepsOfOrders();
	}
}

int Mode::getRootNote()
{
	return rootNote;
}

int Mode::getOffset()
{
	return ((rootNote % scaleSize) + scaleSize) % scaleSize;
}


Array<int> Mode::parse_steps(String stepsIn)
{
	Array<int> stepsOut;

	std::string theSteps = stepsIn.toStdString();
	//char* check;
	//long intCheck;

	char c;
	int step;
	int digits;
	int i = 0;

	while (i < stepsIn.length())
	{
		digits = 0;
		c = stepsIn[i];

		//strtol(&theSteps[i], &check, 10);

		while ((c != ' ' && c != '\t') && (i + digits) < stepsIn.length())
		{
			digits++;
			c = stepsIn[i + digits];
			//strtol(&theSteps[i + digits], &check, 10);
		}

		if (digits > 0)
		{
			step = stepsIn.substring(i, i + digits).getIntValue();
			stepsOut.add(step);
			i += digits + 1;
		}
		else
			i++;
	}

	stepsOut.minimiseStorageOverheads();
	return stepsOut;
}

Array<int> Mode::steps_to_orders(Array<int> stepsIn)
{
	Array<int> ordersOut;

	for (int i = 0; i < stepsIn.size(); i++)
	{
		for (int j = 0; j < stepsIn[i]; j++)
			ordersOut.add(j);
	}

	ordersOut.minimiseStorageOverheads();
	return ordersOut;
}

Array<int> Mode::expand_orders(Array<int> ordersIn, int offsetIn)
{
	Array<int> ordersOut;
	int period = ordersIn.size();
	int off = ((offsetIn % ordersIn.size()) + ordersIn.size()) % ordersIn.size();

	int index;

	for (int i = 0; i < 128; i++)
	{
		index = (off + i) % period;

		// adjust first few notes if it starts in the middle of a whole step
		if (i == 0 && ordersIn[index] != 0)
		{
			int localOffset = ordersIn[index];

			while (ordersIn[index] != 0)
			{
				ordersOut.add(ordersIn[index] - localOffset);
				i++;
				index = (off + i) % period;
			}
		}

		ordersOut.add(ordersIn[index]);
	}

	jassert(ordersOut.size() == 128);
	ordersOut.minimiseStorageOverheads();
	return ordersOut;
}

Array<int> Mode::expand_steps(Array<int> stepsIn)
{
    Array<int> stepsOut;
    
    for (int i = 0; i < stepsIn.size(); i++)
    {
        for (int s = 0; s < stepsIn[i]; s++)
            stepsOut.add(stepsIn[i]);
    }
    
    stepsOut.minimiseStorageOverheads();
    return stepsOut;
}

Array<int> Mode::orders_to_steps(Array<int> layoutIn)
{
	Array<int> stepsOut;

	int i = 0;
	int j = 0;
	int step = 0;

	while (i < layoutIn.size())
	{
		if (layoutIn[i] == 0)
		{
			j = i + 1;
			step = 1;

			while (layoutIn[j] != 0 && j < layoutIn.size())
			{
				j++;
				step++;
			}
		}
		i = j;
		stepsOut.add(step);
	}
	stepsOut.minimiseStorageOverheads();
	return stepsOut;
}

Array<float> Mode::orders_to_modeDegrees(Array<int> ordersIn)
{
	Array<float> degreesOut;
	float deg = -1;

	int stepSize = 0;
	int i = 0;
	while (degreesOut.size() < ordersIn.size())
	{
		stepSize++;

		if ((i + stepSize) == (ordersIn.size() - 1) || 
			ordersIn[i + stepSize] == 0)
		{
			deg++;

			for (int j = 0; j < stepSize; j++)
				degreesOut.add(deg + (j / (float) stepSize));

			i += stepSize;
			stepSize = 0;
		}
	}

	degreesOut.minimiseStorageOverheads();
	return degreesOut;
}

Array<int> Mode::scale_degrees(int scaleSize, int offset)
{
	Array<int> degreesOut;

	auto mod = [](int a, int n) {return ((a % n) + n) % n; };

	for (int i = 0; i < scaleSize; i++)
	{
		degreesOut.add(mod(i - offset, scaleSize));
	}

	degreesOut.minimiseStorageOverheads();
	return degreesOut;
}


Array<int> Mode::interval_sizes(Array<int> stepsIn)
{
	Array<int> intervals;
	Array<int> intervalsOut;
	int step;

	for (int i = 0; i < stepsIn.size(); i++)
	{
		step = stepsIn[i];

		if (intervals.size() < step)
			intervals.resize(step);

		intervals.set(step, intervals[step] + 1);
	}

	// reverse an array
	for (int i = 0; i < intervals.size(); i++)
	{
		int val = intervals[intervals.size() - i - 1];
		if (val != 0)
			intervalsOut.add(val);
	}

	intervalsOut.minimiseStorageOverheads();

	return intervalsOut;
}


int Mode::getScaleSize() const
{
	return scaleSize;
}

int Mode::getModeSize() const
{
	return modeSize;
}


Array<int> Mode::getStepsOfOrders()
{
    return stepsOfOrders;
}

String Mode::getFamily() const
{
	return family;
}

Array<int> Mode::getSteps()
{
	return steps;
}

String Mode::getStepsString()
{
	return stepsString;
}

Array<int>* Mode::getKeyboardOrdersSizes()
{
	return &keyboardOrdersSizes;
}

int Mode::getKeyboardOrdersSize(int ordersIn)
{
	int o = ordersIn % getMaxStep();
	return keyboardOrdersSizes[o];
} 

Array<int> Mode::getOrdersDefault()
{
	return ordersDefault;
}

Array<int> Mode::getOrders()
{
	return orders;
}

Array<float> Mode::getModalDegrees()
{
	return modeDegrees;
}

Array<int> Mode::getScaleDegrees()
{
	return scaleDegrees;
}


int Mode::getMaxStep()
{
	int step = 0;

	for (int i = 0; i < steps.size(); i++)
	{
		if (steps[i] > step)
			step = steps[i];
	}

	return step;
}

String Mode::steps_to_string(Array<int> stepsIn)
{
	String out;

	for (int i = 0; i < stepsIn.size(); i++)
	{
		out += String(stepsIn[i]);

		if (i < stepsIn.size() - 1)
			out += ' ';
	}

	return out;
}

Array<int> Mode::getMOSClass()
{
	return mosClass;
}

String Mode::getDescription()
{
	return family + "[" + String(modeSize) + "] " + String(scaleSize);
}

String Mode::getScaleDescription()
{
	return String(scaleSize) + " " + getDescription().dropLastCharacters(2);
}

String Mode::getModeDescription()
{
	return String(modeSize) + " " + family + " " + String(scaleSize);
}

void Mode::updateStepsOfOrders()
{
    stepsOfOrders = expand_steps(orders_to_steps(orders));
}
