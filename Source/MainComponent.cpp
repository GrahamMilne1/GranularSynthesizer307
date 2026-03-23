#include "MainComponent.h"

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
    //To be used later
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    // Grains are generated here
    bufferToFill.clearActiveBufferRegion();
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

    // read audio data into buffer
    fileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&fileBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    fileSampleRate = (int)reader->sampleRate;

    // delete reader
    delete reader;
}