/*
 ==============================================================================
 
 ViewPianoComponent.h
 Created: 14 Mar 2019 4:50:31pm
 Author:  Vincenzo
 
 ==============================================================================
 */

#pragma once

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../PluginState.h"
#include "../../PluginIDs.h"
#include "../../Structures/Mode.h"
#include "VirtualKeyboardKey.h"
#include "VirtualKeyboardGrid.h"

#include <iostream>     


//==============================================================================
/*
 */

namespace VirtualKeyboard
{
    enum UIMode
    {
        playMode = 0,
        editMode
    };
    
    enum Orientation
    {
        horizontal = 0,
        verticalLeft,
        verticalRight
    };
    
    enum KeyPlacementType
    {
        nestedRight = 0,
        nestedCenter,
        adjacent
    };
    
    class Keyboard :
        public Component,
        public ApplicationCommandTarget,
        public MidiKeyboardStateListener // Only for displaying external MIDI input
    {
        
    public:
        //===============================================================================================
        
        Keyboard(SuperVirtualKeyboardPluginState* pluginStateIn, ApplicationCommandManager* cmdMgrIn);
        ~Keyboard() {};
        
        //===============================================================================================
        
        void initiateDataNode();
        
        void restoreDataNode(ValueTree pianoNodeIn);

		void updatePianoNode();

		void updateKeyProperties();
        
        ValueTree getNode();
        
        //===============================================================================================
        
        MidiKeyboardState* getMidiKeyboardState();

		Key* getKey(int keyNumIn);

		Array<Key*>* getKeysByOrder(int orderIn);

		Array<Key*> getKeysByScaleDegree(int scaleDegreeIn);

		int getLastKeyClicked();
        
        Point<int> getPositionOfKey(int midiNoteIn);
        
        Key* getKeyFromPosition(Point<int> posIn);
        
        Key* getKeyFromPosition(const MouseEvent& e);

        float getKeyVelocity(Key* keyIn, const MouseEvent& e);

		Mode* getMode();
        
		int getWidthFromHeight(int heightIn);

		//===============================================================================================

		int getUIMode();

        void setUIMode(UIMode uiModeIn);
        
        void setKeyPlacement(KeyPlacementType placementIn);
        
        void setKeyProportions(Key* keyIn);

		// might want to restructure so this is not necessary
		void setLastKeyClicked(int keyNumIn);

        //===============================================================================================
        
        Colour getKeyColor(Key* keyIn);

		Colour getKeyOrderColor(int orderIn);

		Colour getKeyDegreeColor(int degIn);

		void setKeyColor(Key* keyIn, int colorIndex, Colour colorIn, bool useColor = true);
		
		void setKeyColor(int keyNumIn, int colorIndex, Colour colorIn, bool useColor = true);

		void setKeyColorOrder(int orderIn, int colorIndex, Colour colorIn);

		void setKeyColorDegree(int tuningDegreeIn, int colorIndex, Colour colorIn);

		void resetKeyOrderColors(int orderIn, bool resetDegrees = false);

		void resetKeyDegreeColors(int tuningDegreeIn);

		void resetKeyColors(bool resetDegrees=false);

		//===============================================================================================
        
        void applyMode(Mode* layoutIn);
                
        void allNoteOff();
        
        void isolateLastNote();
        
        void retriggerNotes();
        
        bool keysAreInSameOrder(int& orderDetected);
        
        Key* transposeKeyModally(Key* key, int stepsIn);
        
        Key* transposeKeyChromatically(Key* key, int stepsIn);
        
        bool transposeKeysOnModally(int modalStepsIn);
        
        void transposeKeysOnChromatically(int modalStepsIn);
        
        //===============================================================================================

		void paint(Graphics& g) override;

		void resized() override;

		//===============================================================================================

		void mouseMove(const MouseEvent& e) override;

		//===============================================================================================
        
        bool keyStateChanged(bool isKeyDown) override;
        
        bool keyPressed(const KeyPress& key) override;
        
        void modifierKeysChanged(const ModifierKeys& modifiers) override;
        
        //===============================================================================================
        
        void triggerKeyNoteOn(Key* key, float velocityIn);
        
        void triggerKeyNoteOff(Key* key);
        
        void handleNoteOn(MidiKeyboardState* source, int midiChannel, int midiNote, float velocity) override;
        
        void handleNoteOff(MidiKeyboardState* source, int midiChannel, int midiNote, float velocity) override;
        
        //===============================================================================================

		ApplicationCommandTarget* getNextCommandTarget() override;

		void getAllCommands(Array< CommandID > &commands) override;

		void getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) override;

		bool perform(const InvocationInfo &info) override;

		//===============================================================================================       
        
    private:
        
		// Application pointers
        SuperVirtualKeyboardPluginState* pluginState;
        ApplicationCommandManager* appCmdMgr;
		UndoManager* undo;
        
		// Functionality
        std::unique_ptr<KeyboardGrid> grid;
        MidiKeyboardState keyboardState;
        MidiBuffer buffer;
		Array<Key*> keysPause;

		// Parameters
		int uiModeSelected = 0;
		int orientationSelected = 0;
		int keyPlacementSelected = 0;

		int midiChannelSelected = 1;
		int midiNoteOffset = 0;
		bool mpeOn = false;
        
        // Data
        ValueTree pianoNode;
		ValueTree keyPlacePresetNode;
		ValueTree keyRatioPresetNode;
		ValueTree keyColorPresetNode;

        OwnedArray<Key> keys;
        
        Array<Array<Key*>> keysOrder;
        Array<Key*> keysOn;
        Mode* mode;

		Array<int> keyMidiNoteMappings;
                
		Array<Colour> keyColorsOrder = { Colours::white, Colours::black, Colours::maroon, Colours::darkslateblue, Colours::forestgreen,
            Colours::darkgoldenrod, Colours::mediumpurple, Colours::orangered, Colours::saddlebrown };
		
		Array<Colour> keyColorsDegree;
		Array<Colour> keyColorsSingle; // organized by (keyNumber - Mode offset)

		Array<Point<float>> keyPlacesOrder;
		Array<Point<float>> keyPlacesDegree;
		Array<Point<float>> keyPlacesSingle;

		Array<Point<float>> keyRatiosOrder;
		Array<Point<float>> keyRatiosDegree;
		Array<Point<float>> keyRatiosSingle;

        
        // Properties
        int keyWidth = 50;
        int keyHeight = 200;
        float defaultKeyWHRatio = 0.25;

		int lastKeyOver = 0;
		int lastKeyClicked = 0;
        
        float pianoWidth;
        float minWindowHeight;

		double resolution = 10e5;
        
        // Locks
        bool rightMouseHeld = false;
        bool shiftHeld = false;
        bool altHeld = false;
        bool ctrlHeld = false;
        bool upHeld = false;
        bool downHeld = false;
        bool leftHeld = false;
        bool rightHeld = false;
        bool spaceHeld = false;
        
        bool displayIsReady = false;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
    };
}
