#pragma once

#include <JuceHeader.h>
#include "SpectrumClass.h"

//==============================================================================
struct MyAudioFormatReaderSource : public AudioFormatReaderSource {
    const float** sampleArray;
    int numChannels;
    int numSamples;

    MyAudioFormatReaderSource(AudioFormatReader* sourceReader, bool deleteReaderWhenThisIsDeleted) : 
        AudioFormatReaderSource(sourceReader, deleteReaderWhenThisIsDeleted) {
    }

    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override { // Retrieve samples from audio file
        AudioFormatReaderSource::getNextAudioBlock(bufferToFill);

        auto* buffer = bufferToFill.buffer;
        numChannels = buffer->getNumChannels();
        numSamples = buffer->getNumSamples();

        sampleArray = bufferToFill.buffer->getArrayOfReadPointers();
    }

    const float** getSamples() { // Return samples from audio file
        return sampleArray;
    }

    int getNumChannels() { // Return number of channels
        return numChannels;
    }

    int getNumSamples() { // Return number of samples
        return numSamples;
    }
};

class MainComponent : public AudioAppComponent, public Button::Listener, public Slider::Listener, public ComboBox::Listener, public ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    // Load Buffer Functions
    void loadFile();
    void loadSamples();

    // Listener Overrides
    void buttonClicked(Button* button) override;
    void sliderValueChanged(Slider* sliderMoved) override;
    void comboBoxChanged(ComboBox* menu) override;


    // Transport Source
    void transportStateChanged(TransportState newState);
    void changeListenerCallback(ChangeBroadcaster* source) override;

    

private:
    //==============================================================================
    bool isPlaying = false;
    float volumeLevel;

    TextButton loadButton{ "Load File" };
    TextButton playButton{ "Play" };
    Slider volumeSlider;
    Label volumeLabel;
    ComboBox skinMenu;
    Label npLabel{ "Now Playing: " };

    AudioDeviceManager audioManager;
    TransportState state;
    AudioFormatReader* reader;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<MyAudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    File audioFile;
    AnalyserComponent visualBox;
    const float** sampleArray;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
