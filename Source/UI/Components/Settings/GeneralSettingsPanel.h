/*
  ==============================================================================

    GeneralSettingsPanel.h
    Created: 13 Nov 2019 7:11:33pm
    Author:  Vincenzo Sicurella

  ==============================================================================
*/

#pragma once

#include "../../../PluginState.h"

class GeneralSettingsPanel : public Component
{
    
public:
    
    GeneralSettingsPanel(SvkPluginState* pluginStateIn);
    
    ~GeneralSettingsPanel();
    
    void paint(Graphics& g) override;
    void resized() override;
    
private:
    
    SvkPluginState* pluginState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneralSettingsPanel)
};