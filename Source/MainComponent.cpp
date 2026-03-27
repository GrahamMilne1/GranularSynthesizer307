#include "MainComponent.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>

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
        repaint(10, 300, 380, 160);
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
        repaint(10, 300, 380, 160);
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

    // set gain
    gain = 1.0;
    targetGain = 1.0;

    // start painting for grain visualizations
    startTimerHz(60);

    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    //Destructor
    shutdownAudio();
}

void MainComponent::timerCallback() {
    repaint();

    // get hand tracking positions from handTracker and update values in grain engine
    if (handTracker.isTracking == true)
    {
        if (handTracker.currentGesture == HandTracking::Gesture::PALM) {
            // set position parameter and slider
            grainEngine.setPosition(handTracker.handX);
            positionSlider.setValue(handTracker.handX, juce::dontSendNotification);

            // set length paremeter and slider
            grainEngine.setLength(handTracker.handY * (1000 - 1) + 1);
            lengthSlider.setValue(handTracker.handY * (1000 - 1) + 1, juce::dontSendNotification);

        } else if (handTracker.currentGesture == HandTracking::Gesture::POINT) {
            // set random position parameter and slider
            grainEngine.setRandomPosition(handTracker.handX);
            positionRandomSlider.setValue(handTracker.handX, juce::dontSendNotification);

            // set random length parameter and slider
            grainEngine.setRandomLength(handTracker.handY);
            lengthRandomSlider.setValue(handTracker.handY, juce::dontSendNotification);
        } else if (handTracker.currentGesture == HandTracking::Gesture::PINCH) {
            // set density parameter and slider
            grainEngine.setDensity(handTracker.handX * (50 - 1) + 1);
            densitySlider.setValue(handTracker.handX * (50 - 1) + 1, juce::dontSendNotification);

            // set pitch parameter and slider
            grainEngine.setPitch(handTracker.handY * 8 - 4);   
            grainPitch.setValue(handTracker.handY * 8 - 4, juce::dontSendNotification);
        }
    }
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

    // set gain based on if fist is detected
    if (handTracker.currentGesture == HandTracking::Gesture::FIST) {
        targetGain = 0.0;
    } else {
        targetGain = 1.0;
    }

    // dont process if no file is loaded in buffer
    if (grainEngine.isFileLoaded()) {
        grainEngine.processGrains(bufferToFill);
    }

    // fade audio either up or down to match target gain
    for (int i = 0; i < bufferToFill.numSamples; i++)
    {
        if (gain < targetGain) {
            gain += 0.00002;
            gain = std::clamp(gain, 0.0f, 1.0f);
        } else if (gain > targetGain) {
            gain -= 0.00002;
            gain = std::clamp(gain, 0.0f, 1.0f);      
        } 
        for (int j = 0; j < bufferToFill.buffer->getNumChannels(); j++)
        {
            bufferToFill.buffer->getWritePointer(j)[i] *= gain;
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

        // get grain snapshot
        std::array<GrainEngine::Snapshot, GrainEngine::maxGrains> snapshot_local = grainEngine.getSnapshot();

        // paint individual grain positions
        for (int i = 0; i < GrainEngine::maxGrains; i++) {
            if (snapshot_local[i].isActive) {
                float opacity = 1.0f - ((float)snapshot_local[i].currentGrainPos / (float)snapshot_local[i].length);

                g.setColour(juce::Colours::red.withAlpha(opacity));
                int localReadPos = (snapshot_local[i].currentBufferPos + snapshot_local[i].startPos);
                int localLinePos = 10 + ((float)localReadPos / (float)grainEngine.getFileBuffer().getNumSamples()) * 380;
                g.fillRect(localLinePos, 300, 2, 160);
            }
        }
    }

    // render video output
    cv::Mat image = handTracker.getImage();
    if (!image.empty()) {
        // convert to JUCE image
        cv::cvtColor(image, image, cv::COLOR_BGR2BGRA);
        juce::Image jImage(juce::Image::PixelFormat::ARGB, image.cols , image.rows, false);

        juce::Image::BitmapData bitmapData(jImage, juce::Image::BitmapData::writeOnly);

        for (int r = 0; r < image.rows; r++) {
            memcpy(bitmapData.getLinePointer(r), image.ptr(r), image.cols * 4);
        }

        // render image

        // stop camera from flickering, not sure why this happens in the first place
        g.setOpacity(1.0f);
        g.drawImage(jImage, juce::Rectangle<float>(410.0f, 300.0f, 300.0f, 160.0f));
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

    repaint(10, 300, 380, 160);
}