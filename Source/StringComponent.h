/*
  ==============================================================================

    StringComponent.h
    Created: 30 Apr 2026 11:35:34am
    Author:  enric

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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