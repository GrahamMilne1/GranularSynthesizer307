#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>

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

        void spawnGrain();

        ~MainComponent() override;

    private:
        juce::AudioFormatManager formatManager;
        juce::AudioBuffer<float> fileBuffer;
        int fileSampleRate = 0;
        juce::TextButton loadButton;
        std::atomic<bool> fileLoaded{false};

        // grain parameters
        double density = 10.0;
        float positionNorm = 0.0f;
        int grainLength = 100;
        double currSampleRate = 44100.0;
        int countDownUntilNextGrain = 0;

        struct Grain 
        {
            bool isActive = false;
            int startPos = 0;
            int currentPos = 0;
            int length = 0;
        };
        static constexpr int maxGrains = 16;
        Grain grains[maxGrains];
};