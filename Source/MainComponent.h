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

        // load button
        juce::TextButton loadButton;

        // Sliders
        juce::Slider densitySlider;
        juce::Slider lengthSlider;
        juce::Slider lengthRandomSlider;
        juce::Slider positionSlider;
        juce::Slider positionRandomSlider;
        std::atomic<bool> fileLoaded{false};

        // Slider labels
        juce::Label densityLabel;
        juce::Label lengthLabel;
        juce::Label lengthRandomLabel;
        juce::Label positionLabel;
        juce::Label positionRandomLabel;


        // grain parameters
        double density = 10.0;
        float positionNorm = 0.0f;
        int grainLength = 100;
        double currSampleRate = 44100.0;
        int countDownUntilNextGrain = 0;

        // randomness parameters
        float lengthRandomnessParam = 0.2f;
        float positionRandomnessParam = 0.05f;

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