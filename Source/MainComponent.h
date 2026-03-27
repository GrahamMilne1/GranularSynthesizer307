#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>
#include "grainEngine/GrainEngine.h"
#include "HandTracking/HandTracking.h"

class MainComponent : public juce::AudioAppComponent, public juce::Timer
{
    public:
        MainComponent();

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
        void releaseResources() override;
        void paint(juce::Graphics &g) override;
        void resized() override;
        void loadFile();
        void timerCallback() override;

        float gain;
        float targetGain;

        HandTracking handTracker;

        ~MainComponent() override;

    private:

        GrainEngine grainEngine;

        // load button
        juce::TextButton loadButton;

        // Sliders
        juce::Slider densitySlider;
        juce::Slider lengthSlider;
        juce::Slider lengthRandomSlider;
        juce::Slider positionSlider;
        juce::Slider positionRandomSlider;
        juce::Slider grainPitch;

        // Slider labels
        juce::Label densityLabel;
        juce::Label lengthLabel;
        juce::Label lengthRandomLabel;
        juce::Label positionLabel;
        juce::Label positionRandomLabel;
        juce::Label pitchLabel;
};