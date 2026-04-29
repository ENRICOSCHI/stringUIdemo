#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    Corda vibrante orizzontale (Karplus-Strong visivo).
    Tutte le 6 corde hanno la stessa lunghezza grafica (come sulla chitarra).
*/
class StringComponent final : public juce::Component,
    private juce::Timer
{
public:
    StringComponent(juce::Colour stringColour)
        : colour(stringColour)
    {
        setInterceptsMouseClicks(false, false);
        startTimerHz(60);
    }

    void stringPlucked(float pluckPositionRelative)
    {
        amplitude = maxAmplitude * std::sin(pluckPositionRelative * juce::MathConstants<float>::pi);
        phase = juce::MathConstants<float>::pi;
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(colour);
        g.strokePath(generateStringPath(), juce::PathStrokeType(2.0f));
    }

    juce::Path generateStringPath() const
    {
        float w = (float)getWidth();
        float y = (float)getHeight() / 2.0f;

        juce::Path p;
        p.startNewSubPath(0.0f, y);
        p.quadraticTo(w / 2.0f, y + std::sin(phase) * amplitude, w, y);
        return p;
    }

private:
    void timerCallback() override
    {
        amplitude *= 0.99f;

        float phaseStep = 400.0f / (float)juce::jmax(1, getWidth());
        phase += phaseStep;

        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        repaint();
    }

    juce::Colour colour;
    float amplitude = 0.0f;
    const float maxAmplitude = 12.0f;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringComponent)
};

//==============================================================================
class StringUIdemoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit StringUIdemoAudioProcessorEditor(StringUIdemoAudioProcessor&);
    ~StringUIdemoAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Colori delle corde (pubblico per usarlo nel paint)
    static juce::Colour stringColour(int index)
    {
        // E2=oro, A2=argento, D3=bronzo, G3=azzurro, B3=verde, E4=rosso
        const juce::Colour colours[6] = {
            juce::Colour(0xFFD4A017),
            juce::Colour(0xFFC0C0C0),
            juce::Colour(0xFFCD7F32),
            juce::Colour(0xFF4FC3F7),
            juce::Colour(0xFF81C784),
            juce::Colour(0xFFEF5350)
        };
        return colours[index % 6];
    }

private:
    void mouseDown(const juce::MouseEvent& e) override { handleMouseEvent(e); }
    void mouseDrag(const juce::MouseEvent& e) override { handleMouseEvent(e); }
    void handleMouseEvent(const juce::MouseEvent& e);

    StringUIdemoAudioProcessor& audioProcessor;

    juce::OwnedArray<StringComponent> stringComponents;

    //numero tasti del manico della chitarra
    const int numFret = 12;

    juce::Label notaSuonataLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringUIdemoAudioProcessorEditor)
};