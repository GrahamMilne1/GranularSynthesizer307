#include "GrainEngine.h"

GrainEngine::GrainEngine() 
{
    // Constructor
    
    formatManager.registerBasicFormats();
}

GrainEngine::~GrainEngine()
{
    //Destructor
}

void GrainEngine::setDensity(double density) 
{
    this->density = density;
}

void GrainEngine::setPosition(float position) 
{
    positionNorm = position;
}

void GrainEngine::setRandomPosition(float rPosition) 
{
    positionRandomnessParam = rPosition;
}

void GrainEngine::setLength(int length) 
{
    grainLength = length;
}

void GrainEngine::setRandomLength(float rLength) 
{
    lengthRandomnessParam = rLength;
}

void GrainEngine::setPitch(float pitch) 
{
    pitch = this->pitch;
}

void GrainEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
    //store current output sample rate
    currSampleRate = sampleRate;
}

void GrainEngine::processGrains(const juce::AudioSourceChannelInfo &bufferToFill) 
{
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

                // fractional part of current grain postion
                float CGPFraction = grains[g].currentBufferPos - (std::floor(grains[g].currentBufferPos));

                // get read position
                int readPos = std::floor(grains[g].startPos + grains[g].currentBufferPos);
    
                // if read position is outside of buffer, deactivate it
                if (readPos + 1 >= fileBuffer.getNumSamples() || readPos < 0) {
                    grains[g].isActive = false;
                    continue;
                }
    
                // get phase and evelope
                float phase = (float)grains[g].currentGrainPos / (float)grains[g].length;
                float envelope = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));

                // if the grain is active, loop over every channel and add current sample
                for (int channel = 0; channel < channels; channel++) 
                {
                    // get sample with linear interpolation
                    float sample = ((fileBuffer.getSample(channel, readPos)) * (1.0f - CGPFraction)) + ((fileBuffer.getSample(channel, readPos + 1)) * CGPFraction);

                    // apply envelope to sample
                    sample *= envelope;

                    // add sample to buffer
                    bufferToFill.buffer->addSample(channel, i + bufferToFill.startSample, sample);
                }

                // increase current position and deactivation check
                grains[g].currentGrainPos++;
                grains[g].currentBufferPos += grains[g].playbackRate;

                if (grains[g].currentGrainPos >= grains[g].length) {
                    grains[g].isActive = false;
                }
            }
        }
    }
}

void GrainEngine::spawnGrain() {
    for (int g = 0; g < maxGrains; g++) {
        if (grains[g].isActive == false) {
    
            // this section is kind of a mess

            // spawn grain if not active
            grains[g].isActive = true;
            grains[g].currentBufferPos = 0;
            grains[g].currentGrainPos = 0;

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

            // get playback rate
            grains[g].playbackRate = pitch;

            return;
        }
    }
}

void GrainEngine::loadFile() 
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

const juce::AudioBuffer<float>& GrainEngine::getFileBuffer() 
{
    return fileBuffer;
}

const bool GrainEngine::isFileLoaded() 
{
    return fileLoaded;
}

const int GrainEngine::getFileSampleRate() 
{
    return fileSampleRate;
}

const float GrainEngine::getPositionNorm()
{
    return positionNorm;
}

const int GrainEngine::getGrainLength() 
{
    return grainLength;
}