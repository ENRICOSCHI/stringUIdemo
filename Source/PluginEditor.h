#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "StringComponent.h"

//==================================================================
class StringUIdemoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit StringUIdemoAudioProcessorEditor(StringUIdemoAudioProcessor&);
    ~StringUIdemoAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Colori delle corde
    static juce::Colour stringColour(int index)
    {
        const juce::Colour colours[6] = {
            juce::Colour(0xFF008000),//E2
            juce::Colour(0xFF00CF00),//A2
            juce::Colour(0xFF1FFF1F),//D3
            juce::Colour(0xFF6FFF6F),//G3
            juce::Colour(0xFF96FF96),//B3
            juce::Colour(0xFFCCFFCC)//E4
        };
        return colours[index % 6];
    }

private:

    int oldPosFret = -1;
    int oldMidiNote = -1;
 
    void mouseDown(const juce::MouseEvent& e) override { handleMouseEvent(e); }
    
    void mouseDrag(const juce::MouseEvent& e) override {
        handleMouseEvent(e);
    }

    void mouseUp(const juce::MouseEvent& e) override {
        
        //al rilascio del mouse azzero i parametri (-1 perche' 0 e' una pos. del fret)
        oldPosFret = -1; 
        oldMidiNote = -1;
    }
    void handleMouseEvent(const juce::MouseEvent& e);

    /*Editor Methods per impostare gli oggetti della UI*/
    void SetTitle(juce::Graphics&);
    void SetLineaSeparatrice(juce::Graphics&);
    void SetStrings(juce::Graphics&);
    void SetSeparationFret(juce::Graphics&);


    StringUIdemoAudioProcessor& audioProcessor;

    juce::OwnedArray<StringComponent> stringComponents;

    //numero tasti del manico della chitarra
    const int numFret = 12;
    //numero di corde della chitarra
    const int numCorde = 6;

    juce::Label notaSuonataLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringUIdemoAudioProcessorEditor)
};