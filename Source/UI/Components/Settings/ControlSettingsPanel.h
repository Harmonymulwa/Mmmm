/*
  ==============================================================================

    ControlSettingsPanel.h
    Created: 13 Nov 2019 7:11:08pm
    Author:  Vincenzo Sicurella

  ==============================================================================
*/

#pragma once

#include "../../SvkUiPanel.h"
#include "../../../PluginState.h"

class ControlSettingsPanel : public SvkUiPanel
{
    
public:
    
    ControlSettingsPanel(SvkPluginState* pluginStateIn);
    
    ~ControlSettingsPanel();
    
    void connectToProcessor(AudioProcessorValueTreeState& processorTree) override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
private:
    
    SvkPluginState* pluginState;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlSettingsPanel)
};
