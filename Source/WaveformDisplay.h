/*
  ==============================================================================
    WaveformDisplay.h
    Waveform display component with playhead and hot cue markers
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * @class WaveformDisplay
 * @brief Displays an audio waveform thumbnail with a playhead indicator and hot cue markers.
 */
class WaveformDisplay : public juce::Component,
    public juce::ChangeListener
{
public:
    /**
     * @brief Construct a WaveformDisplay.
     * @param formatManagerToUse The format manager for decoding audio.
     * @param cacheToUse         The thumbnail cache for rendering.
     */
    WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
        juce::AudioThumbnailCache& cacheToUse);
    ~WaveformDisplay() override;

    /** @brief Render the waveform, playhead and cue markers. */
    void paint(juce::Graphics& g) override;

    /** @brief Required override; waveform fills the full component bounds. */
    void resized() override;

    /** @brief Called when the thumbnail data changes. */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /**
     * @brief Load waveform data from a URL.
     * @param audioURL The URL of the audio file.
     */
    void loadURL(juce::URL audioURL);

    /**
     * @brief Update the playhead position.
     * @param pos Normalised position [0, 1].
     */
    void setPositionRelative(double pos);

    /**
     * @brief Set hot cue positions for display as coloured markers.
     * @param cuePositions Vector of normalised [0,1] cue positions (-1 = unset).
     */
    void setHotCuePositions(const std::vector<double>& cuePositions);

private:
    juce::AudioThumbnail audioThumb;
    bool   fileLoaded = false;
    double position = 0.0;

    std::vector<double> hotCuePositions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};