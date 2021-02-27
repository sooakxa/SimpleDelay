/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleDelayAudioProcessorEditor::SimpleDelayAudioProcessorEditor (SimpleDelayAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), treeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 280);

    // Delay time
    delayTimeValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "delayTime", delayTimeSlider);
    delayTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    delayTimeSlider.setRange(0.0f, 5000.0f, 1.0f);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&delayTimeSlider);

    addAndMakeVisible(delayTimeLabel);
    delayTimeLabel.setText("Delay time (samples)", juce::dontSendNotification);
    delayTimeLabel.attachToComponent(&delayTimeSlider, false);

    // Feedback
    feedbackValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "feedback", feedbackSlider);
    feedbackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    feedbackSlider.setRange(0.0f, 0.99f, 0.01f);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&feedbackSlider);

    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider, false);
}

SimpleDelayAudioProcessorEditor::~SimpleDelayAudioProcessorEditor()
{
}

//==============================================================================
void SimpleDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::grey);
    g.setColour(juce::Colours::white);
    g.setFont(25);
    g.drawFittedText("Simple Delay", 150, 20, 130, 30, juce::Justification::centred, 1, 0.0f);
}

void SimpleDelayAudioProcessorEditor::resized()
{
    delayTimeSlider.setBounds(50, 110, 320, 50);
    feedbackSlider.setBounds(50, 180, 320, 50);
}
