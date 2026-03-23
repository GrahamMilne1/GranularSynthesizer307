#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "MainComponent.h"

class GranularSynthApp : public juce::JUCEApplication
{ 
    public: 
    const juce::String getApplicationName() override 
    {
        return "GranularSynth";
    }

    const juce::String getApplicationVersion() override
    {
        return "0.1.0";
    }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }
    class MainWindow : public juce::DocumentWindow 
    { 
    public: 
        MainWindow(juce::String name) : DocumentWindow(name, juce::Colours::darkgrey, allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            centreWithSize(800, 600);
            setVisible(true);

        }
        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
    private: 
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(GranularSynthApp)