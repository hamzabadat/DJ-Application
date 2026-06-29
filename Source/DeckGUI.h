/*
  ==============================================================================
    DeckGUI.h
    DJ deck UI component with transport, EQ, hot cues, waveform, VU meter and vinyl spinner
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "CueLibrary.h"

/**
 * @class RotaryKnob
 * @brief Labelled rotary slider for EQ controls.
 */
class RotaryKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;

    /**
     * @brief Construct a RotaryKnob.
     * @param labelText  Label shown below knob.
     * @param minVal     Minimum value.
     * @param maxVal     Maximum value.
     * @param defaultVal Starting value.
     */
    RotaryKnob(const juce::String& labelText,
        double minVal, double maxVal, double defaultVal);

    void resized() override;
};

/**
 * @class VUMeter
 * @brief Animated vertical VU meter that shows real-time audio level.
 *
 * Call setLevel() from the audio thread via an atomic, then the
 * timer in DeckGUI repaints it at ~20fps.
 */
class VUMeter : public juce::Component
{
public:
    VUMeter();

    /**
     * @brief Set the current peak level (0.0 – 1.0). Thread-safe.
     * @param newLevel Normalised audio level.
     */
    void setLevel(float newLevel);

    /** @brief Draw the segmented VU bar and peak-hold marker. */
    void paint(juce::Graphics& g) override;

private:
    std::atomic<float> level{ 0.0f };
    float displayLevel = 0.0f;   ///< Smoothed display value
    float peakLevel = 0.0f;   ///< Held peak
    int   peakHoldTimer = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};

/**
 * @class VinylSpinner
 * @brief Draws a spinning vinyl record graphic that rotates while playing.
 */
class VinylSpinner : public juce::Component
{
public:
    VinylSpinner();

    /**
     * @brief Advance the rotation angle. Call from timer when playing.
     * @param degreesPerTick Rotation increment per timer tick.
     */
    void tick(float degreesPerTick);

    /** @brief Set whether the record is currently spinning. */
    void setPlaying(bool playing);

    /** @brief Update the album art shown at the centre of the record. */
    void setAlbumArt(const juce::Image& art);

    /** @brief Draw the vinyl record graphic at the current rotation angle. */
    void paint(juce::Graphics& g) override;

private:
    float        angle = 0.0f;
    bool         isPlaying = false;
    juce::Image  albumArt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VinylSpinner)
};

/**
 * @class TimeDisplay
 * @brief Large CDJ-style digital time readout showing elapsed and remaining time.
 */
class TimeDisplay : public juce::Component
{
public:
    TimeDisplay();

    /**
     * @brief Update the displayed times.
     * @param positionSecs  Current playback position in seconds.
     * @param lengthSecs    Total track length in seconds.
     */
    void setTime(double positionSecs, double lengthSecs);

    /** @brief Draw elapsed and remaining time side-by-side with a progress bar. */
    void paint(juce::Graphics& g) override;

private:
    /**
     * @brief Format a duration in seconds into a display string.
     * @param secs      Duration in seconds.
     * @param showMillis If true, appends centiseconds (.xx).
     * @return Formatted string e.g. "3:47" or "3:47.08".
     */
    static juce::String formatTime(double secs, bool showMillis);

    double position = 0.0;
    double length = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeDisplay)
};

/**
 * @class DeckGUI
 * @brief Full DJ deck UI: transport, EQ, hot cues, VU meter, vinyl spinner, waveform.
 */
class DeckGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::FileDragAndDropTarget,
    public juce::Timer
{
public:
    /**
     * @brief Construct a DeckGUI.
     * @param player             Pointer to the DJAudioPlayer to control.
     * @param formatManagerToUse Format manager for waveform display.
     * @param cacheToUse         Thumbnail cache for waveform display.
     * @param deckName           Display name e.g. "Deck 1".
     */
    DeckGUI(DJAudioPlayer* player,
        juce::AudioFormatManager& formatManagerToUse,
        juce::AudioThumbnailCache& cacheToUse,
        const juce::String& deckName,
        CueLibrary& cueLibrary);

    ~DeckGUI() override;

    /** @brief Paint the deck header gradient and accent line. */
    void paint(juce::Graphics& g) override;

    /** @brief Lay out all child components within the deck bounds. */
    void resized() override;

    /** @brief Handle button click events. */
    void buttonClicked(juce::Button* button) override;

    /** @brief Handle slider value change events. */
    void sliderValueChanged(juce::Slider* slider) override;

    /** @return True — accepts dragged audio files. */
    bool isInterestedInFileDrag(const juce::StringArray& files) override;

    /** @brief Load dropped audio files onto this deck. */
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /** @brief Timer callback: updates playhead, VU meter, vinyl spin. */
    void timerCallback() override;

    /**
     * @brief Load a track from a URL.
     * @param url       File URL.
     * @param trackName Display name for the track.
     * @param art       Optional album art image.
     */
    void loadTrack(const juce::URL& url,
        const juce::String& trackName,
        const juce::Image& art = {},
        const juce::String& filePath = {});

    /** @brief Serialise EQ and hot cue state for persistence. */
    juce::ValueTree saveState() const;

    /** @brief Restore state from a previously saved ValueTree. */
    void loadState(const juce::ValueTree& state);

    /**
     * @brief Feed the latest audio buffer peak so the VU meter stays current.
     * @param peak Normalised peak level 0.0 – 1.0.
     */
    void pushAudioLevel(float peak);

private:
    /** @brief Refresh hot cue button colours to reflect current set/unset state. */
    void updateHotCueButtons();

    /** @brief Write the current deck's cue array to CueLibrary for the loaded track. */
    void saveCuesForCurrentTrack();

    DJAudioPlayer* player;
    juce::String   deckName;
    juce::String   currentTrackName;

    // Transport
    juce::TextButton loadButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton loopButton;

    // Sliders
    juce::Slider volSlider;
    juce::Label  volLabel;
    juce::Slider speedSlider;
    juce::Label  speedLabel;
    juce::Slider posSlider;
    juce::Label  posLabel;

    // EQ
    RotaryKnob bassKnob;
    RotaryKnob midKnob;
    RotaryKnob trebleKnob;

    // Track info
    juce::Label trackNameLabel;

    // Waveform
    WaveformDisplay waveformDisplay;

    // Hot cues
    std::array<juce::TextButton, DJAudioPlayer::NUM_HOT_CUES> hotCueButtons;
    juce::TextButton clearAllCuesButton;

    // VU meter
    VUMeter vuMeter;

    // Time display
    TimeDisplay timeDisplay;

    // BPM tap tempo
    juce::TextButton tapButton;
    juce::Label      bpmLabel;
    juce::int64      lastTapMs = 0;
    int              tapCount = 0;
    double           tapBpmAccum = 0.0;

    // Per-track cue persistence
    CueLibrary& cueLibrary;
    juce::String currentFilePath;

    // Vinyl spinner
    VinylSpinner vinylSpinner;

    // Right-click listeners for individual cue clear
    struct CueRightClickListener : public juce::MouseListener
    {
        std::function<void()> onRightClick;
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (e.mods.isRightButtonDown() && onRightClick)
                onRightClick();
        }
    };
    CueRightClickListener cueListeners[DJAudioPlayer::NUM_HOT_CUES];

    juce::FileChooser fChooser{ "Select an audio file...",
                                juce::File::getSpecialLocation(juce::File::userMusicDirectory),
                                "*.mp3;*.wav;*.aiff;*.flac;*.ogg" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};