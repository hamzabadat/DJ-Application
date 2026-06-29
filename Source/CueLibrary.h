/*
  ==============================================================================
    CueLibrary.h
    Per-track hot cue persistence, keyed by absolute file path
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include <unordered_map>

/**
 * @class CueLibrary
 * @brief Stores and persists hot cue sets keyed by absolute file path.

 */
class CueLibrary
{
public:
    CueLibrary();

    /**
     * @brief Save the current cue array for a given file path.
     * @param filePath Absolute path of the track.
     * @param cues     Array of NUM_HOT_CUES cues.
     */
    void saveCues(const juce::String& filePath,
        const std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES>& cues);

    /**
     * @brief Load saved cues for a file path into an array.
     * @param filePath Absolute path of the track.
     * @param cues     Destination array — reset to empty if no record found.
     * @return True if cues were found and loaded.
     */
    bool loadCues(const juce::String& filePath,
        std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES>& cues) const;

    /** @brief Write the full cue map to disk. */
    void persist() const;

    /** @brief Load the cue map from disk. */
    void load();

private:
    /** @return The path to the cues.xml persistence file on disk. */
    juce::File getFile() const;

    // Maps absolute file path to the array of hot cues for that track
    std::unordered_map<std::string,
        std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES>> cueMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueLibrary)
};