#include "MainComponent.h"
#include "SpectrumClass.h"

//==============================================================================
MainComponent::MainComponent() : state(Stopped), AudioAppComponent(audioManager)
{
    audioManager.initialise(0, 2, nullptr, true);

    setSize (900, 400);
    setAudioChannels (0, 2);

    addAndMakeVisible(loadButton);
    loadButton.addListener(this);
    addAndMakeVisible(playButton);
    playButton.setEnabled(false);
    playButton.addListener(this);

    addAndMakeVisible(skinMenu);
    skinMenu.addListener(this);
    skinMenu.addItem("Default", 1);
    skinMenu.addItem("Sunset", 2);
    skinMenu.addItem("Synthwave", 3);
    skinMenu.addItem("Trippy", 4);
    skinMenu.setSelectedItemIndex(0);

    addAndMakeVisible(volumeSlider);
    volumeSlider.addListener(this);
    volumeSlider.setRange(0.0, 100.0);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    volumeSlider.setSkewFactorFromMidPoint(10.0);
    volumeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, 0, 0, 0);
    volumeSlider.setValue(0.1);

    addAndMakeVisible(volumeLabel);
    volumeLabel.setText("Visualizer Gain: ", dontSendNotification);

    npLabel.setText("Now Playing: ", juce::dontSendNotification);
    npLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(npLabel);

    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    addAndMakeVisible(visualBox);


    setVisible(true);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
        {
            transportStateChanged(Starting);
        }
        else
        {
            transportStateChanged(Stopped);
        }
    }
}

void MainComponent::buttonClicked(Button* button) {
    if (button == &loadButton) {
        DBG("Clicked");
        loadFile();
    } 
    else if (button == &playButton) {
        DBG("Clicked");
        if (!isPlaying) { // PLAY
            isPlaying = true;
            playButton.setButtonText("Stop");
            transportStateChanged(Starting);
        }
        else { // STOP
            isPlaying = false;
            playButton.setButtonText("Play");
            transportStateChanged(Stopping);
            
        }
    }
}

void MainComponent::sliderValueChanged(Slider* slider) {
    if (slider == &volumeSlider) {
        if (volumeSlider.getValue() == 0.0) {
            volumeLevel = 0.00001;
        }
        else {
            volumeLevel = volumeSlider.getValue() / 5;
        }
    }
}

void MainComponent::comboBoxChanged(ComboBox* menu) {
    if (menu == &skinMenu) {
        int colorNum = skinMenu.getSelectedItemIndex();
        visualBox.giveComboBoxIndex(skinMenu.getSelectedItemIndex());
        repaint();
    }
}

void MainComponent::transportStateChanged(TransportState newState) { // State Changes are triggered by the button clicks, start and stop audio
    if (newState != state) {
        state = newState;

        switch (state) {
        case Stopped:
            transportSource.setPosition(0.0);
            break;
        case Starting:
            transportSource.start();
            break;
        case Stopping:
            transportSource.stop();
            break;
        case Playing:
            break;
        }
    }
}

void MainComponent::loadFile()
{
    FileChooser myChooser("Please select the song you want to load...",
        File::getSpecialLocation(File::userHomeDirectory),
        "*.wav; *.mp3; *.aiff");

    if (myChooser.browseForFileToOpen())
    {
        File newFile(myChooser.getResult());

        audioFile = newFile;

        reader = formatManager.createReaderFor(audioFile); // Loads file into Reader

        std::unique_ptr<MyAudioFormatReaderSource> tempSource(new MyAudioFormatReaderSource(reader, true));
        transportSource.setSource(tempSource.get(), 0, nullptr, reader->sampleRate);

        readerSource.reset(tempSource.release());
    }
    if (audioFile.existsAsFile()) {
        DBG("WORKS");
    }

    if (audioFile.existsAsFile()) {
        playButton.setEnabled(true);
    }

    npLabel.setText("Now Playing: " + audioFile.getFileName(), dontSendNotification);
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    sampleArray = readerSource->getSamples(); // Get Samples from File (Stored in Reader Source)
    auto numChannels = readerSource->getNumChannels();
    auto numSamples = readerSource->getNumSamples();
    
    for (auto channel = 0; channel < numChannels; ++channel) {
        for (auto sample = 0; sample < numSamples; ++sample) {
            float newSample = sampleArray[channel][sample] * volumeLevel;

            if (newSample >= 1.0) {
                newSample = 1.0;
            }

            visualBox.pushNextSampleIntoFifo(newSample); // Send Samples to Fourier Transformer
        }
    }

    transportSource.getNextAudioBlock(bufferToFill); // Tell Transport to get next Audio Block
}



void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    if (skinMenu.getSelectedItemIndex() == 0) {
        g.fillAll(Colours::black);
        npLabel.setColour(juce::Label::textColourId, Colours::white);
        volumeLabel.setColour(juce::Label::textColourId, Colours::white);
    }
    else if (skinMenu.getSelectedItemIndex() == 1) { // Sunset
        g.fillAll(juce::Colour::fromRGB(82, 20, 61));
        npLabel.setColour(juce::Label::textColourId, Colours::white);
        volumeLabel.setColour(juce::Label::textColourId, Colours::white);
    }
    else if (skinMenu.getSelectedItemIndex() == 2) { // Synthwave
        g.fillAll(juce::Colours::black);
        npLabel.setColour(juce::Label::textColourId, Colours::white);
        volumeLabel.setColour(juce::Label::textColourId, Colours::white);
    }
    else if (skinMenu.getSelectedItemIndex() == 3) { // Trippy
        g.fillAll(juce::Colour::fromRGB(253, 213, 152));
        npLabel.setColour(juce::Label::textColourId, Colours::black);
        volumeLabel.setColour(juce::Label::textColourId, Colours::black);
    }
    
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    auto buttonLine = bounds.removeFromTop(30);

    loadButton.setBounds(buttonLine.removeFromRight(100));
    playButton.setBounds(buttonLine.removeFromLeft(100));

    buttonLine.removeFromLeft(8);
    buttonLine.removeFromRight(8);

    skinMenu.setBounds(buttonLine.removeFromRight(200));

    volumeLabel.setBounds(buttonLine.removeFromLeft(100));
    volumeSlider.setBounds(buttonLine.removeFromLeft(100));
    npLabel.setBounds(buttonLine);

    bounds.removeFromTop(12);

    visualBox.setBounds(bounds);
}
