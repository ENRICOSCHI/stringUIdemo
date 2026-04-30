#pragma once

#include <JuceHeader.h>

//==============================================================================
class StringSynthesiser
{
public:
    StringSynthesiser(double sampleRate, double frequencyInHz) : fs(sampleRate)
    {
        doPluckForNextBuffer.set(false);
        //allocazione massima per note basse (es. 20Hz)
        maxDelayLength = (size_t)juce::roundToInt(sampleRate / 20.0);
        delayLine.resize(maxDelayLength, 0.0f);
        excitationSample.resize(maxDelayLength, 0.0f);

        setFrequency(frequencyInHz);
    }

    void stringPlucked(float pluckPosition)
    {
        jassert(pluckPosition >= 0.0f && pluckPosition <= 1.0f);
        if (doPluckForNextBuffer.compareAndSetBool(1, 0))
            amplitude = std::sin(juce::MathConstants<float>::pi * pluckPosition);
    }

    void generateAndAddData(float* outBuffer, int numSamples)
    {
        if (doPluckForNextBuffer.compareAndSetBool(0, 1))
            exciteInternalBuffer(); //riempo l'inizio della DelayLine

        for (int i = 0; i < numSamples; ++i)
        {
            //uso currentDelayLenght per il modulo e non delayLine.size()
            auto nextPos = (pos + 1) % currentDelayLength;
            delayLine[nextPos] = (float)(decay * 0.5 * (delayLine[nextPos] + delayLine[pos]));
            outBuffer[i] += delayLine[pos];
            pos = nextPos;
        }
    }

    void setFrequency(double newFrequencyInHz)
    {
        //calcolo il nuovo periodo della nota
        currentDelayLength = (size_t)juce::roundToInt(fs / newFrequencyInHz);
        currentDelayLength = juce::jlimit((size_t)2, maxDelayLength, currentDelayLength);

        //rigenero il rumore solo per la parte necessaria
        std::generate(excitationSample.begin(), excitationSample.begin() + currentDelayLength,
            [] { return (juce::Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f; });//<- lambda []
    }

private:
    void exciteInternalBuffer()
    {
        //copio il rumore nell'area attiva della delay line
        for (size_t i = 0; i < currentDelayLength; ++i)
            delayLine[i] = excitationSample[i] * (float)amplitude;
    }

    double fs;
    size_t maxDelayLength;
    size_t currentDelayLength = 0;

    const double decay = 0.998;
    double amplitude = 0.0;

    juce::Atomic<int> doPluckForNextBuffer;
    std::vector<float> excitationSample, delayLine;
    size_t pos = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringSynthesiser)
};


//==============================================================================
class StringUIdemoAudioProcessor : public juce::AudioProcessor
{
public:
    static const int numStrings = 6;
    //accordatura standard chitarra: E2 A2 D3 G3 B3 E4
    static const int guitarMidiNotes[numStrings];

    StringUIdemoAudioProcessor();
    ~StringUIdemoAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi()  const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //chiamato dall'editor per pizzicare una sola corda
    void pluckString(int stringIndex, float position);

private:
    juce::OwnedArray<StringSynthesiser> stringSynths;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringUIdemoAudioProcessor)
};