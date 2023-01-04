#pragma once
#include "MainComponent.h"

class AnalyserComponent : public Component,
    private Timer
{
public:
    enum
    {
        fftOrder = 11, // SIZE OF FFT WINDOW
        fftSize = 1 << fftOrder,
        scopeSize = 512 // Number of points in visualization
    };

    AnalyserComponent()
        : forwardFFT(fftOrder),
        window(fftSize, juce::dsp::WindowingFunction<float>::rectangular)
    {
        setOpaque(true);

        startTimerHz(300);
        setSize(700, 500);
    }

    ~AnalyserComponent() override
    {

    }

    //==============================================================================

    void giveComboBoxIndex(int index) {
        comboBoxIndex = index;
    }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        float averageValue = 0.0;
        for (int i = 0; i < 10; ++i) {
            averageValue += scopeData[i];
        }

        averageValue = averageValue / scopeSize;
        
        juce::Rectangle<float> area(getLocalBounds().getWidth(), getLocalBounds().getHeight());
        if (comboBoxIndex == 0) {
            g.fillAll(Colours::black);
            g.setColour(Colours::white);
        }
        else if (comboBoxIndex == 1) { // Sunset Background
            Image background = ImageCache::getFromMemory(BinaryData::visualBackground_jpg, BinaryData::visualBackground_jpgSize);
            g.drawImage(background, area, RectanglePlacement::fillDestination, false);
            g.setColour(sunsetColours[(int)(scopeData[10] * 10)]);
        }
        else if (comboBoxIndex == 2) { // Synthwave Background
            Image background = ImageCache::getFromMemory(BinaryData::synthwave_jpg, BinaryData::synthwave_jpgSize);
            g.drawImage(background, area, RectanglePlacement::fillDestination, false);
            g.setColour(synthwaveColours[(int)(scopeData[10] * 10)]);
        }
        else if (comboBoxIndex == 3) { // Trippy Background
            if (randCounter == 3) {
                randColour = randomGenerator.nextFloat();
                randCounter = 0;
            }
            else {
                randCounter++;
            }
            
            Image background = ImageCache::getFromMemory(BinaryData::trippy_jpg, BinaryData::trippy_jpgSize);
            g.drawImage(background, area, RectanglePlacement::fillDestination, false);
            g.setColour(trippyColours[(int)(randColour * 10)]);
        }

        drawFrame(g);
    }

    void timerCallback() override // Timer which is set to display every startTimerHz() times a second.
    {
        if (nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum();
            nextFFTBlockReady = false;
            repaint(); // Calls the visualiser to repaint the graphic
        }
    }

    void pushNextSampleIntoFifo(float sample) noexcept // Pushes the next sample of data into the array (Called by the getNextAudioBlock of MainComponent
    {
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady)
            {
                juce::zeromem(fftData, sizeof(fftData));
                memcpy(fftData, fifo, sizeof(fifo));
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample; // This sample is retrieved from the reader
    }

    void drawNextFrameOfSpectrum() // Called by the timer to calculate the next set of audio data
    {
        window.multiplyWithWindowingTable(fftData, fftSize); // Windowing Function
        forwardFFT.performFrequencyOnlyForwardTransform(fftData); // FFT Algorithm

        auto mindB = -100.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < scopeSize; ++i) // Skews the data in a way that represents the amplitudes accurate to hearing (Logarithmic)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(fftData[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)fftSize)),
                mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }

    void drawFrame(juce::Graphics& g) // In charge of drawing the lines
    {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto width = getLocalBounds().getWidth();
            auto height = getLocalBounds().getHeight();

            auto chooseScope = scopeData[i - 1];
            auto chooseScope2 = scopeData[i];

            if (comboBoxIndex == 3) {
                g.drawLine({ (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width), 0,
                          (float)juce::jmap(i, 0, scopeSize - 1, 0, width), juce::jmap(chooseScope2, 0.0f, 1.0f, (float)height, 0.0f) });
            }
            else {
                g.drawLine({ (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width), juce::jmap(chooseScope, 0.0f, 1.0f, (float)height, 0.0f),
                          (float)juce::jmap(i, 0, scopeSize - 1, 0, width), juce::jmap(chooseScope2, 0.0f, 1.0f, (float)height, 0.0f) });
            }
        }
    }

private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    float fifo[fftSize];                    // Sample Array
    float fftData[2 * fftSize];             // FFT Calculations
    int fifoIndex = 0;                      // Keeps count of samples in fifo
    bool nextFFTBlockReady = false;         // Ready for the next FFT block to be rendered
    float scopeData[scopeSize];             // Contains display data

    Random randomGenerator;
    int randCounter = 0;
    float randColour;

    int comboBoxIndex;

    juce::Array<juce::Colour> sunsetColours{  juce::Colour::fromRGB(253, 207, 206), // 0
                                        juce::Colour::fromRGB(252, 183, 182), // 1
                                        juce::Colour::fromRGB(251, 159, 157), // 2
                                        juce::Colour::fromRGB(250, 135, 133), // 3
                                        juce::Colour::fromRGB(249, 110, 108), // 4
                                        juce::Colour::fromRGB(248, 86, 84), // 5
                                        juce::Colour::fromRGB(247, 62, 59), // 6
                                        juce::Colour::fromRGB(246, 38, 35), // 7
                                        juce::Colour::fromRGB(245, 14, 10), // 8
                                        juce::Colour::fromRGB(220, 13, 9), // 9
                                        juce::Colour::fromRGB(196, 11, 8) }; // 10

    juce::Array<juce::Colour> trippyColours{ juce::Colour::fromRGB(255, 204, 255), // 0
                                        juce::Colour::fromRGB(179, 218, 255), // 1
                                        juce::Colour::fromRGB(153, 221, 255), // 2
                                        juce::Colour::fromRGB(153, 255, 255), // 3
                                        juce::Colour::fromRGB(153, 255, 204), // 4
                                        juce::Colour::fromRGB(204, 255, 153), // 5
                                        juce::Colour::fromRGB(255, 255, 153), // 6
                                        juce::Colour::fromRGB(255, 191, 128), // 7
                                        juce::Colour::fromRGB(255, 153, 153), // 8
                                        juce::Colour::fromRGB(255, 102, 153), // 9
                                        juce::Colour::fromRGB(255, 102, 102) }; // 10
                                        
    juce::Array<juce::Colour> synthwaveColours{ juce::Colour::fromRGB(153, 230, 255), // 0
                                        juce::Colour::fromRGB(128, 223, 255), // 1
                                        juce::Colour::fromRGB(102, 217, 255), // 2
                                        juce::Colour::fromRGB(77, 210, 255), // 3
                                        juce::Colour::fromRGB(51, 204, 255), // 4
                                        juce::Colour::fromRGB(26, 198, 255), // 5
                                        juce::Colour::fromRGB(0, 191, 255), // 6
                                        juce::Colour::fromRGB(0, 172, 230), // 7
                                        juce::Colour::fromRGB(0, 153, 204), // 8
                                        juce::Colour::fromRGB(0, 134, 179), // 9
                                        juce::Colour::fromRGB(0, 115, 153) }; // 10

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalyserComponent)
};
