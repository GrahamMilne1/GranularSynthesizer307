#include "MainComponent.h"
#include <iostream>

MainComponent::MainComponent() 
{
    // Constructor
    formatManager.registerBasicFormats();
    loadButton.setButtonText("Load File");
    loadButton.onClick = [this] { loadFile(); };
    addAndMakeVisible(loadButton);
    setSize(800, 600);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    //Destructor
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
    //store current output sample rate
    currSampleRate = sampleRate;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    
    // clear the buffer
    bufferToFill.clearActiveBufferRegion();

    
    // early return if no file is loaded in buffer
    if (!fileLoaded) {
        return;
    }
    
    // set total channels
    int channels = fileBuffer.getNumChannels();
   
    // loop over every sample
    for (int i = 0; i < bufferToFill.numSamples; i++)
    {
        // spawn new grain if countdown is 0
        if (countDownUntilNextGrain <= 0) {
            spawnGrain();
            countDownUntilNextGrain = (int)(currSampleRate / density);
        }
        countDownUntilNextGrain--;

        // loop over every grain
        for (int g = 0; g < maxGrains; g++) 
        {
            if (grains[g].isActive == true) {

                // get read position
                int readPos = grains[g].startPos + grains[g].currentPos;
    
                // if the grain is not active, skip it
                if (readPos >= fileBuffer.getNumSamples()) {
                    grains[g].isActive = false;
                    break;
                }
    
                // get phase and evelope
                float phase = (float)grains[g].currentPos / (float)grains[g].length;
                float envelope = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));

                // if the grain is active, loop over every channel and play grain
                for (int channel = 0; channel < channels; channel++) 
                {
                    // add grain to buffer 
                    float sample = fileBuffer.getSample(channel, readPos) * envelope;
                    bufferToFill.buffer->addSample(channel, i + bufferToFill.startSample, sample);
                }
                // increase current position and deactivation check
                grains[g].currentPos++;
                if (grains[g].currentPos >= grains[g].length) {
                    grains[g].isActive = false;
                }
            }
        }
    }
}

void MainComponent::spawnGrain() {
    for (int g = 0; g < maxGrains; g++) {
        if (grains[g].isActive == false) {
            grains[g].isActive = true;
            grains[g].currentPos = 0;
            grains[g].startPos = (int)(positionNorm * fileBuffer.getNumSamples());
            grains[g].length = (int)(currSampleRate * grainLength * 0.001);
            return;
        }
    }
}

void MainComponent::releaseResources() 
{
    //Empty for now
}

void MainComponent::paint (juce::Graphics &g) 
{
    // Fills the BG colour
    g.fillAll(juce::Colours::darkgrey);
}

void MainComponent::resized() 
{
    // Gives components size and pos
    loadButton.setBounds(10, 10, 100, 30);
}

void MainComponent::loadFile() 
{
    juce::FileChooser chooser("Select an audio file", {}, "*.wav;*.aiff;*.mp3");

    // return early if no file is chosen
    if (!chooser.browseForFileToOpen()) {
        return;
    }
    juce::File file = chooser.getResult();
    auto* reader = formatManager.createReaderFor(file);

    // return early if reader is null
    if (reader == nullptr) {
        return;
    }

    fileLoaded = false;
    

    // read audio data into buffer
    fileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&fileBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    fileSampleRate = (int)reader->sampleRate;

    fileLoaded = true;

    delete reader;
}