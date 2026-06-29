/*
  ==============================================================================
    MainComponent.h
    Top-level application component hosting both decks, players, and the playlist
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "CueLibrary.h"

/**
 * @class MainComponent
 * @brief Top-level UI component.
 *
 * Contains two DeckGUI instances, their backing DJAudioPlayers, a MixerAudioSource,
 * and the PlaylistComponent. Manages audio routing and session persistence.
 */
class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    /** @brief Prepare all audio sources for playback at the given sample rate. */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /** @brief Fill the next audio buffer from the mixer source. */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    /** @brief Release audio resources on all sources. */
    void releaseResources() override;

    /** @brief Paint the application background and deck dividers. */
    void paint(juce::Graphics& g) override;

    /** @brief Lay out the two decks and playlist within the window. */
    void resized() override;

private:
    /** @brief Save deck states (EQ, hot cues) to disk. */
    void saveDeckStates();

    /** @brief Load deck states from disk on startup. */
    void loadDeckStates();

    /** @return Path to the deck state XML file. */
    juce::File getDeckStateFile() const;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbCache{ 100 };

    CueLibrary cueLibrary;

    DJAudioPlayer player1{ formatManager };
    DJAudioPlayer player2{ formatManager };

    DeckGUI deckGUI1{ &player1, formatManager, thumbCache, "Deck 1", cueLibrary };
    DeckGUI deckGUI2{ &player2, formatManager, thumbCache, "Deck 2", cueLibrary };

    juce::MixerAudioSource mixerSource;

    PlaylistComponent playlistComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};