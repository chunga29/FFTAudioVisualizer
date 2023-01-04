# FFTAudioVisualizer
Visualizes audio files using a Fast Fourier Transform by converting the signal into individual spectral components.

![FFTAudioVisualizer](https://user-images.githubusercontent.com/15708632/210482558-97d2a616-f977-465a-bccc-1b2aacbe7d16.png)

For a specified range in time (window), the FFT changes the representation of amplitude over time to amplitude over frequency. Each of these ranges is a window, which is a short sample of the signal. A series of windows gives us time, amplitude, and frequency. Short windows provide inaccurate frequency, but accurate duration and long windows provide the inverse of that.

The spectrumClass deals with the calculations and drawing of the graphics. In addition, the project utilizes the graphic capabilities of the JUCE software, including adding pictures as backgrounds, coloring the spectrum, and using a file loader. Samples are also transferred between classes using a struct of AudioFormatReaderSource.

# Main Features
The application plays music using the AudioAppComponent class. This player is implemented using a transporter and an audio manager. The transport state is in charge of playing and stopping the music, using an enumerator. The user interface includes a play/stop button, a gain slider to raise the level of the visualizer, a “Now Playing” label, a skin menu, and a load file button. Below the HUD, a visualizer is displayed, which retrieves its signal from the file.
