#include "MainComponent.h"
#include <iostream>

MainComponent::MainComponent() 
{
    // Constructor
    formatManager.registerBasicFormats();

    // load button 
    loadButton.setButtonText("Load File");
    loadButton.onClick = [this] { loadFile(); };
    addAndMakeVisible(loadButton);
    setSize(800, 600);

    // density slider
    densityLabel.setText("Grain Density", juce::dontSendNotification);
    addAndMakeVisible(densityLabel);

    densitySlider.setRange(1, 50, 1);
    densitySlider.setValue(density);
    densitySlider.onValueChange = [this]
    { density = densitySlider.getValue(); };
    addAndMakeVisible(densitySlider);

    // length slider
    lengthLabel.setText("Grain Length (ms)", juce::dontSendNotification);
    addAndMakeVisible(lengthLabel);

    lengthSlider.setRange(1, 1000, 5);
    lengthSlider.setValue(grainLength);
    lengthSlider.onValueChange = [this]
    { 
        grainLength = lengthSlider.getValue();
        repaint(10, 260, 380, 160);
    };
    addAndMakeVisible(lengthSlider);

    // length randomnness slider
    lengthRandomLabel.setText("Grain Length Randomness", juce::dontSendNotification);
    addAndMakeVisible(lengthRandomLabel);

    lengthRandomSlider.setRange(0, 1, 0.01);
    lengthRandomSlider.setValue(lengthRandomnessParam);
    lengthRandomSlider.onValueChange = [this]
    { lengthRandomnessParam = lengthRandomSlider.getValue(); };
    addAndMakeVisible(lengthRandomSlider);

    // position slider
    positionLabel.setText("Grain Position", juce::dontSendNotification);
    addAndMakeVisible(positionLabel);

    positionSlider.setRange(0, 1, 0.01);
    positionSlider.setValue(positionNorm);
    positionSlider.onValueChange = [this]
    { 
        positionNorm = positionSlider.getValue();
        repaint(10, 260, 380, 160);
    };
    addAndMakeVisible(positionSlider);

    // position randomnness slider
    positionRandomLabel.setText("Grain Position Randomness", juce::dontSendNotification);
    addAndMakeVisible(positionRandomLabel);

    positionRandomSlider.setRange(0, 1, 0.01);
    positionRandomSlider.setValue(positionRandomnessParam);
    positionRandomSlider.onValueChange = [this]
    { positionRandomnessParam = positionRandomSlider.getValue(); };
    addAndMakeVisible(positionRandomSlider);

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
    
            // this section is kind of a mess

            // spawn grain if not active
            grains[g].isActive = true;
            grains[g].currentPos = 0;

            // calculate base position and random offset
            int basePosition = positionNorm * fileBuffer.getNumSamples();
            int randomOffset = positionRandomnessParam * (juce::Random::getSystemRandom().nextFloat() * 2 - 1) * fileBuffer.getNumSamples();

            // get start position with randomness applied
            grains[g].startPos = juce::jlimit(0, fileBuffer.getNumSamples(),(int)(basePosition + randomOffset));

            // get length with randomness applied
            grains[g].length =  
               (int)((currSampleRate * grainLength * 0.001) * 
                (1 + lengthRandomnessParam * (
                    juce::Random::getSystemRandom().nextFloat() * 2 - 1))
            );
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
    g.fillAll(juce::Colours::grey);

    // waveform area
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(10, 260, 380, 160);
    g.fillRect(10, 260, 380, 160);

    if (fileLoaded) {

        // draw the waveform
        const float *samples = fileBuffer.getReadPointer(0);
        float samplesPerPixel = fileBuffer.getNumSamples() / 380.0f;
        g.setColour(juce::Colours::green);
        for (int i = 0; i < 379; i++)
        {
            float chunkMax = 0.0f;
            float chunkMin = 0.0f;
            // find min and max sample values for each chunk
            int chunkStart = i * samplesPerPixel;
            int chunkEnd = chunkStart + samplesPerPixel;
            for (int j = chunkStart; j < chunkEnd; j++)
            {
                if (fileBuffer.getSample(0, j) > chunkMax) {
                    chunkMax = fileBuffer.getSample(0, j);
                } else if (fileBuffer.getSample(0, j) < chunkMin) {
                    chunkMin = fileBuffer.getSample(0, j);
                }
            }
            //normalize min and max
            int lineMax = 340 - (chunkMax * 80);
            int lineMin = 340 - (chunkMin * 80);
            // draw waveform lines
            g.drawVerticalLine(10 + i, lineMax, lineMin);
        }

        // normalize grain position and draw line
        int linePos = 10 + positionNorm * 380;
        g.setColour(juce::Colours::cyan);
        g.fillRect(linePos, 260, 2, 160);

        // normalize grain length and draw highlighted area

        int GLAreaWidth = ((grainLength * 0.001 * fileSampleRate) / (fileBuffer.getNumSamples())) * 380;
        g.setColour(juce::Colours::cyan.withAlpha(0.5f));
        g.fillRect(linePos, 260, GLAreaWidth, 160);
    }
}

// Gives components size and pos
void MainComponent::resized() 
{   
    // file load button
    loadButton.setBounds(10, 10, 100, 30);

    // density slider
    densityLabel.setBounds(10, 50, 190, 30);
    densitySlider.setBounds(200, 50, 580, 30);

    // length slider
    lengthLabel.setBounds(10, 90, 190, 30);
    lengthSlider.setBounds(200, 90, 580, 30);

    // length random slider
    lengthRandomLabel.setBounds(10, 130, 190, 30);
    lengthRandomSlider.setBounds(200, 130, 580, 30);

    // position slider
    positionLabel.setBounds(10, 170, 190, 30);
    positionSlider.setBounds(200, 170, 580, 30);

    // position random slider
    positionRandomLabel.setBounds(10, 210, 190, 30);
    positionRandomSlider.setBounds(200, 210, 580, 30);
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

    repaint(10, 260, 380, 160);

    delete reader;
}