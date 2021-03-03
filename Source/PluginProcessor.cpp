/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleDelayAudioProcessor::SimpleDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, juce::Identifier("PARAMETERS"),
                           { std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay (samples)", 0.f, 5000.f, 1000.f),
                             std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback 0-1", 0.f, 0.99f, 0.3f) })
#endif
{
    treeState.addParameterListener("delayTime", this);
    treeState.addParameterListener("feedback", this);
}

SimpleDelayAudioProcessor::~SimpleDelayAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleDelayAudioProcessor::getCurrentProgram()
{

    return 0;
}

void SimpleDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // Note that in order to use this you will need to uncomment the following line
    // Instantiates a new delayline with max size changed to sampleRate. You change to sampleRate * 2 or whatever you want
    
    mDelayLine = std::unique_ptr<juce::dsp::DelayLine<float>>(new juce::dsp::DelayLine<float>(sampleRate));
    
    // Not that if you use the above you will need to use -> instead of the . operator when accessing members functions of mDelayLine
    // Example below
  

    mDelayLine->reset(); // Comment this line
    //mDelayLine->reset(); // Uncomment this line (not change in access operator)
    mDelayLine->prepare(spec);

    mOscillator[0].reset();
    mOscillator[1].reset();

    mOscillator[0].prepare(spec);
    mOscillator[1].prepare(spec);

    mOscillator[0].initialise([](float x) { return std::sin(x); }, 128);
    mOscillator[1].initialise([](float x) { return std::sin(x); }, 128);

    mOscillator[0].setFrequency(0.2);
    mOscillator[1].setFrequency(0.2);

}

void SimpleDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float in = channelData[i];
            float lfoOut = mOscillator[channel].processSample(0.0f);

            // Remap range from -1 to 1 to 100 samps to 500 samps
            float delayMod = juce::jmap(lfoOut, -1.0f, 1.0f, 100.0f, 500.0f);

            // Implement Two taps
            float temp = mDelayLine->popSample(channel, mDelayTime + delayMod, false);
            float temp2 = mDelayLine->popSample(channel, mDelayTime + 500 + delayMod, true); // Set to true to update read pointer

            mDelayLine->pushSample(channel, in + (temp * mFeedback));
            channelData[i] = (in + temp + temp2) * 0.33f;
        }
    }
}

//==============================================================================
bool SimpleDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleDelayAudioProcessor::createEditor()
{
    return new SimpleDelayAudioProcessorEditor (*this, treeState);
}

//==============================================================================
void SimpleDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleDelayAudioProcessor();
}

// Function called when parameter is changed
void SimpleDelayAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "delayTime")
    {
        mDelayTime = newValue;
        mDelayLine->setDelay(newValue);
    }

    else if (parameterID == "feedback")
    {
        mFeedback = newValue;
    }
}