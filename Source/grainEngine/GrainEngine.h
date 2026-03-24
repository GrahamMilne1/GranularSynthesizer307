#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>

class GrainEngine
{
    public:
        GrainEngine();

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        void processGrains(const juce::AudioSourceChannelInfo &bufferToFill);
        void loadFile();

        //setting methods
        void setDensity(double density);
        void setPosition(float position);
        void setRandomPosition(float rPosition);
        void setLength(int length);
        void setRandomLength(float rLength);
        void setPitch(float pitch);

        // getting methods
        const juce::AudioBuffer<float>& getFileBuffer();
        const bool isFileLoaded();
        const int getFileSampleRate();
        const float getPositionNorm();
        const int getGrainLength();

        ~GrainEngine();

    private:
        juce::AudioFormatManager formatManager;
        juce::AudioBuffer<float> fileBuffer;
        int fileSampleRate = 0;

        void spawnGrain();

        std::atomic<bool> fileLoaded{false};

        // grain parameters
        double density = 10.0;
        float positionNorm = 0.0f;
        int grainLength = 100;
        double currSampleRate = 44100.0;
        int countDownUntilNextGrain = 0;
        float pitch = 1.0f;

        // randomness parameters
        float lengthRandomnessParam = 0.2f;
        float positionRandomnessParam = 0.05f;

        struct Grain 
        {
            bool isActive = false;
            int startPos = 0;
            int currentGrainPos = 0;
            int length = 0;
            float currentBufferPos = 0.0f;
            float playbackRate = 0.0f;
        };
        static constexpr int maxGrains = 16;
        Grain grains[maxGrains];
};