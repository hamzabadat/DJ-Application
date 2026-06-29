/*
  ==============================================================================
    DJAudioPlayer.h
    Audio player with transport, 3-band EQ, hot cues, speed control and BPM
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** Represents a single hot cue point */
struct HotCue
{
    bool isSet = false;       ///< Whether this cue has been assigned
    double positionSecs = 0.0;///< Position in seconds
    juce::String label;       ///< Optional label for this cue
};

/**
 * @class DJAudioPlayer
 * @brief Audio player with transport, EQ (3-band), hot cues, speed and gain control.
 *
 * Wraps JUCE audio sources and a DSP chain for a full-featured deck player.
 */
class DJAudioPlayer : public juce::AudioSource
{
public:
    static constexpr int NUM_HOT_CUES = 8;

    /**
     * @brief Construct a DJAudioPlayer.
     * @param formatManager The audio format manager to use for decoding files.
     */
    explicit DJAudioPlayer(juce::AudioFormatManager& formatManager);
    ~DJAudioPlayer() override;

    //==========================================================================
    // AudioSource interface
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==========================================================================
    // Transport

    /** @brief Load audio from a URL. @param audioURL The file or stream URL. */
    void loadURL(juce::URL audioURL);

    /** @brief Start playback. */
    void start();

    /** @brief Stop playback. */
    void stop();

    /** @return True if currently playing. */
    bool isPlaying() const;

    /** @brief Set looping on/off. @param shouldLoop Whether to loop. */
    void setLooping(bool shouldLoop);

    /** @return Whether looping is enabled. */
    bool isLooping() const;

    //==========================================================================
    // Gain / Speed / Position

    /**
     * @brief Set playback gain.
     * @param gain Value in range [0.0, 1.0].
     */
    void setGain(double gain);

    /**
     * @brief Set playback speed via resampling ratio.
     * @param ratio Value in range [0.0, 4.0] where 1.0 = normal speed.
     */
    void setSpeed(double ratio);

    /**
     * @brief Seek to an absolute position.
     * @param posInSecs Position in seconds.
     */
    void setPosition(double posInSecs);

    /**
     * @brief Seek using a normalised [0, 1] position.
     * @param pos Relative position from 0.0 (start) to 1.0 (end).
     */
    void setPositionRelative(double pos);

    /** @return Current position in seconds. */
    double getCurrentPositionSecs() const;

    /** @return Normalised [0, 1] playhead position. */
    double getPositionRelative() const;

    /** @return Total track length in seconds. */
    double getLengthInSeconds() const;

    //==========================================================================
    // EQ (3-band: Bass, Mid, Treble)

    /**
     * @brief Set bass EQ gain.
     * @param gainDb Gain in dB.
     */
    void setBassGain(float gainDb);

    /**
     * @brief Set mid EQ gain.
     * @param gainDb Gain in dB.
     */
    void setMidGain(float gainDb);

    /**
     * @brief Set treble EQ gain.
     * @param gainDb Gain in dB.
     */
    void setTrebleGain(float gainDb);

    /** @return Current bass gain in dB. */
    float getBassGain()   const { return bassGainDb; }

    /** @return Current mid gain in dB. */
    float getMidGain()    const { return midGainDb; }

    /** @return Current treble gain in dB. */
    float getTrebleGain() const { return trebleGainDb; }

    //==========================================================================
    // Hot Cues

    /**
     * @brief Assign a hot cue at the current playback position.
     * @param index Cue index [0, NUM_HOT_CUES).
     */
    void setHotCue(int index);

    /**
     * @brief Jump playback to a hot cue position.
     * @param index Cue index [0, NUM_HOT_CUES).
     */
    void triggerHotCue(int index);

    /**
     * @brief Clear a single hot cue.
     * @param index Cue index [0, NUM_HOT_CUES).
     */
    void clearHotCue(int index);

    /** @brief Clear all hot cues. */
    void clearAllHotCues();

    /**
     * @brief Get a copy of a hot cue.
     * @param index Cue index [0, NUM_HOT_CUES).
     * @return The HotCue struct at that index.
     */
    HotCue getHotCue(int index) const;

    /**
     * @brief Directly set hot cue data (e.g., from persistence).
     * @param index Cue index.
     * @param cue   The HotCue to store.
     */
    void setHotCueData(int index, const HotCue& cue);

    //==========================================================================
    // BPM

    /**
     * @brief Set the BPM value for this deck (manually or after detection).
     * @param bpm Beats per minute.
     */
    void setBPM(double bpm);

    /** @return The current BPM value. */
    double getBPM() const;

    /** @return The latest audio peak level (0.0 – 1.0). Thread-safe. */
    float getPeakLevel() const;

    /** @return The file name of the currently loaded track. */
    juce::String getCurrentFileName() const;

private:
    /** @brief Recalculate all IIR filter coefficients from current gain values. */
    void updateEQFilters();

    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource                  transportSource;
    juce::ResamplingAudioSource                 resampleSource{ &transportSource, false, 2 };

    // DSP EQ chain (per-channel IIR filters)
    double currentSampleRate = 44100.0;

    // Three separate filter pairs (bass, mid, treble) — stereo processing
    juce::IIRFilter bassFilterL, bassFilterR;
    juce::IIRFilter midFilterL, midFilterR;
    juce::IIRFilter trebleFilterL, trebleFilterR;

    float bassGainDb = 0.0f;
    float midGainDb = 0.0f;
    float trebleGainDb = 0.0f;

    std::array<HotCue, NUM_HOT_CUES> hotCues;

    double bpm = 0.0;
    std::atomic<float> peakLevel{ 0.0f };
    juce::String currentFileName;
    bool looping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJAudioPlayer)
};