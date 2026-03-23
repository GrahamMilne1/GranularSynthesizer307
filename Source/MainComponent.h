#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

class MainComponent : public juce::AudioAppComponent
{
    public:
        MainComponent();

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
        void releaseResources() override;
        void paint(juce::Graphics &g) override;
        void resized() override;
        void loadFile();

        ~MainComponent() override;

    private:
        juce::AudioFormatManager formatManager;
        juce::AudioBuffer<float> fileBuffer;
        int fileSampleRate = 0;
        juce::TextButton loadButton;
};