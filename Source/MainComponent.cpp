#include "MainComponent.h"
#include <iostream>

MainComponent::MainComponent() 
{
    // Constructor
 
    // load button 
    loadButton.setButtonText("Load File");
    loadButton.onClick = [this] { loadFile(); };
    addAndMakeVisible(loadButton);
    setSize(800, 600);

    // density slider
    densityLabel.setText("Grain Density", juce::dontSendNotification);
    addAndMakeVisible(densityLabel);

    densitySlider.setRange(1, 50, 1);
    densitySlider.setValue(10.0f);
    densitySlider.onValueChange = [this]
    { grainEngine.setDensity(densitySlider.getValue()); };
    addAndMakeVisible(densitySlider);

    // length slider
    lengthLabel.setText("Grain Length (ms)", juce::dontSendNotification);
    addAndMakeVisible(lengthLabel);

    lengthSlider.setRange(1, 1000, 5);
    lengthSlider.setValue(100);
    lengthSlider.onValueChange = [this]
    { 
        grainEngine.setLength(lengthSlider.getValue());
        repaint(10, 260, 380, 160);
    };
    addAndMakeVisible(lengthSlider);

    // length randomnness slider
    lengthRandomLabel.setText("Grain Length Randomness", juce::dontSendNotification);
    addAndMakeVisible(lengthRandomLabel);

    lengthRandomSlider.setRange(0, 1, 0.01);
    lengthRandomSlider.setValue(0.2f);
    lengthRandomSlider.onValueChange = [this]
    { grainEngine.setRandomLength(lengthRandomSlider.getValue()); };
    addAndMakeVisible(lengthRandomSlider);

    // position slider
    positionLabel.setText("Grain Position", juce::dontSendNotification);
    addAndMakeVisible(positionLabel);

    positionSlider.setRange(0, 1, 0.01);
    positionSlider.setValue(0.0f);
    positionSlider.onValueChange = [this]
    { 
        grainEngine.setPosition(positionSlider.getValue());
        repaint(10, 260, 380, 160);
    };
    addAndMakeVisible(positionSlider);

    // position randomnness slider
    positionRandomLabel.setText("Grain Position Randomness", juce::dontSendNotification);
    addAndMakeVisible(positionRandomLabel);

    positionRandomSlider.setRange(0, 1, 0.01);
    positionRandomSlider.setValue(0.05f);
    positionRandomSlider.onValueChange = [this]
    { grainEngine.setRandomPosition(positionRandomSlider.getValue()); };
    addAndMakeVisible(positionRandomSlider);

    // pitch slider
    pitchLabel.setText("Grain Pitch", juce::dontSendNotification);
    addAndMakeVisible(pitchLabel);

    grainPitch.setRange(-4, 4, (1.0f/12.0f));
    grainPitch.setValue(1.0f);
    grainPitch.onValueChange = [this]
    { grainEngine.setPitch(grainPitch.getValue()); };
    addAndMakeVisible(grainPitch);

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
    grainEngine.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    
    // clear the buffer
    bufferToFill.clearActiveBufferRegion();

    // early return if no file is loaded in buffer
    if (grainEngine.isFileLoaded()) {
        grainEngine.processGrains(bufferToFill);
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
    g.drawRect(10, 300, 380, 160);
    g.fillRect(10, 300, 380, 160);

    if (grainEngine.isFileLoaded()) {

        // draw the waveform
        const float *samples = grainEngine.getFileBuffer().getReadPointer(0);
        float samplesPerPixel = grainEngine.getFileBuffer().getNumSamples() / 380.0f;
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
                if (grainEngine.getFileBuffer().getSample(0, j) > chunkMax) {
                    chunkMax = grainEngine.getFileBuffer().getSample(0, j);
                } else if (grainEngine.getFileBuffer().getSample(0, j) < chunkMin) {
                    chunkMin = grainEngine.getFileBuffer().getSample(0, j);
                }
            }
            //normalize min and max
            int lineMax = 390 - (chunkMax * 80);
            int lineMin = 390 - (chunkMin * 80);
            // draw waveform lines
            g.drawVerticalLine(10 + i, lineMax, lineMin);
        }

        // normalize grain position and draw line
        int linePos = 10 + grainEngine.getPositionNorm() * 380;
        g.setColour(juce::Colours::cyan);
        g.fillRect(linePos, 300, 2, 160);

        // normalize grain length and draw highlighted area

        int GLAreaWidth = ((grainEngine.getGrainLength() * 0.001 * grainEngine.getFileSampleRate()) / (grainEngine.getFileBuffer().getNumSamples())) * 380;
        g.setColour(juce::Colours::cyan.withAlpha(0.5f));
        g.fillRect(linePos, 300, GLAreaWidth, 160);
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

    // pitch slider
    pitchLabel.setBounds(10, 250, 190, 30);
    grainPitch.setBounds(200, 250, 580, 30);
}

void MainComponent::loadFile() 
{
    grainEngine.loadFile();

    repaint(10, 260, 380, 160);
}