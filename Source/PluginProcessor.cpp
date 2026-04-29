#include "PluginProcessor.h"
#include "PluginEditor.h"

// Accordatura standard: E2=40, A2=45, D3=50, G3=55, B3=59, E4=64
const int StringUIdemoAudioProcessor::guitarMidiNotes[StringUIdemoAudioProcessor::numStrings]
= { 40, 45, 50, 55, 59, 64 };

//==============================================================================
StringUIdemoAudioProcessor::StringUIdemoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

StringUIdemoAudioProcessor::~StringUIdemoAudioProcessor() {}

//==============================================================================
void StringUIdemoAudioProcessor::pluckString(int stringIndex, float position)
{
    if (stringIndex >= 0 && stringIndex < stringSynths.size())
    {
        // Dividiamo la corda in 12 segmenti (12 tasti)
        const int numFrets = 12;

        // Calcoliamo quale tasto è stato premuto in base alla posizione X (0.0 a 1.0)
        int fret = juce::jlimit(0, numFrets, (int)(position * (float)numFrets));

        // Nota MIDI finale = Nota base della corda + tasto
        int midiNote = guitarMidiNotes[stringIndex] + fret;
        double freqHz = juce::MidiMessage::getMidiNoteInHertz(midiNote);

        auto* synth = stringSynths.getUnchecked(stringIndex);

        // 1. Aggiorna la frequenza del synth
        synth->setFrequency(freqHz);

        // 2. Pizzica la corda
        synth->stringPlucked(position);
    }
}

//==============================================================================
const juce::String StringUIdemoAudioProcessor::getName() const { return JucePlugin_Name; }

bool StringUIdemoAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool StringUIdemoAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool StringUIdemoAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double StringUIdemoAudioProcessor::getTailLengthSeconds() const { return 2.0; }

int StringUIdemoAudioProcessor::getNumPrograms() { return 1; }
int StringUIdemoAudioProcessor::getCurrentProgram() { return 0; }
void StringUIdemoAudioProcessor::setCurrentProgram(int) {}
const juce::String StringUIdemoAudioProcessor::getProgramName(int) { return {}; }
void StringUIdemoAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void StringUIdemoAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    stringSynths.clear();

    for (int i = 0; i < numStrings; ++i)
    {
        double freq = juce::MidiMessage::getMidiNoteInHertz(guitarMidiNotes[i]);
        stringSynths.add(new StringSynthesiser(sampleRate, freq));
    }
}

void StringUIdemoAudioProcessor::releaseResources()
{
    stringSynths.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StringUIdemoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void StringUIdemoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    // Genera tutte le corde sul canale 0
    float* channelData = buffer.getWritePointer(0);

    for (int i = 0; i < stringSynths.size(); ++i)
        stringSynths.getUnchecked(i)->generateAndAddData(channelData, buffer.getNumSamples());

    // Copia canale 0 su tutti gli altri (stereo)
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, buffer, 0, 0, buffer.getNumSamples());
}

//==============================================================================
bool StringUIdemoAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* StringUIdemoAudioProcessor::createEditor()
{
    return new StringUIdemoAudioProcessorEditor(*this);
}

//==============================================================================
void StringUIdemoAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void StringUIdemoAudioProcessor::setStateInformation(const void*, int) {}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StringUIdemoAudioProcessor();
}