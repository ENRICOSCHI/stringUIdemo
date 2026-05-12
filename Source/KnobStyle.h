/*
  ==============================================================================

    KnobStyle.h
    Created: 12 May 2026 12:41:45pm
    Author:  enric

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class KnobStyle  : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
            int x, int y, int width, int height,
            float sliderPos, //indica dove punta l'indicatore
            float rotaryStartAngle,
            float rotaryEndAngle,
            juce::Slider&) override 
    {
        
        
        auto radius = juce::jmin(width, height) / 2.0f - 10.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;

        // Angolo del knob, per sapere verso dove punta
        auto angle = rotaryStartAngle
            + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Cerchio principale
        g.setColour(juce::Colours::purple);
        //disegno il cerchio interno
        g.fillEllipse(centreX - radius,
            centreY - radius,
            radius * 2.0f,
            radius * 2.0f);

        // Bordo
        g.setColour(juce::Colours::white);
        //disegno il cerchio esterno
        g.drawEllipse(centreX - radius,
            centreY - radius,
            radius * 2.0f,
            radius * 2.0f,
            3.0f);

        // Linea indicatore
        juce::Path p;

        auto pointerLength = radius * 0.8f;
        auto pointerThickness = 4.0f;

        //creo la linea del knob
        p.addRectangle(-pointerThickness * 0.5f,
            -radius,
            pointerThickness,
            pointerLength);

        g.setColour(juce::Colours::white);

        //permetto di ruotare la linea
        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    }
};
