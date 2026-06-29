/*
  ==============================================================================
    PlaylistComponent.h
    Music library with drag-to-reorder, album art, search, and persistence
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

/** Holds metadata for a single track in the library */
struct TrackInfo
{
    juce::String title;            ///< Track name (filename without extension)
    juce::String filePath;         ///< Full absolute file path
    juce::String duration;         ///< Formatted duration string "mm:ss"
    juce::String fileType;         ///< Extension e.g. ".mp3"
    double       durationSecs = 0.0; ///< Raw duration in seconds for sorting
    juce::Image  albumArt;         ///< Embedded album art (may be null)
};

/**
 * @class PlaylistComponent
 * @brief Music library panel with drag-to-reorder rows and album art.
 *
 * Column layout (hard-coded px from left of each row):
 *   Art(44) | Track Title(flex) | Length(65) | File(55) | Deck1(65) | Deck2(65) | Delete(50)
 */
class PlaylistComponent : public juce::Component,
    public juce::ListBoxModel,
    public juce::Button::Listener,
    public juce::FileDragAndDropTarget
{
public:
    using LoadCallback = std::function<void(const juce::URL&, const juce::String&, const juce::Image&, const juce::String&)>;

    PlaylistComponent();
    ~PlaylistComponent() override;

    /** @brief Draw the column headers and search bar background. */
    void paint(juce::Graphics& g) override;

    /** @brief Lay out the list box, search bar and add-files button. */
    void resized() override;

    // ListBoxModel
    /** @return Number of rows in the filtered track list. */
    int getNumRows() override;

    /** @brief Not used — rows are fully custom components. */
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
        int width, int height, bool rowIsSelected) override;

    /**
     * @brief Create or reconfigure the custom component for a list row.
     * @param rowNumber        Row index in the filtered list.
     * @param isRowSelected    Whether this row is currently selected.
     * @param existingComponent Previously returned component to reuse, or nullptr.
     * @return Configured PlaylistRow component.
     */
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected,
        juce::Component* existingComponent) override;

    /** @brief Called by ListBox drag-to-reorder machinery */
    void moveRow(int fromIndex, int toIndex);

    // Button::Listener
    /** @brief Handle clicks on the add-files button. */
    void buttonClicked(juce::Button* button) override;

    // FileDragAndDropTarget
    /** @return True — the playlist accepts any dragged files. */
    bool isInterestedInFileDrag(const juce::StringArray& files) override;

    /** @brief Add all valid audio files from a drag-and-drop operation. */
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /** @brief Set callback for loading a track to Deck 1. */
    void setDeck1LoadCallback(LoadCallback cb);

    /** @brief Set callback for loading a track to Deck 2. */
    void setDeck2LoadCallback(LoadCallback cb);

    /** @brief Add audio files to the library. */
    void addFiles(const juce::StringArray& files);

    /** @brief Persist library to disk. */
    void saveLibrary();

    /** @brief Load library from disk. */
    void loadLibrary();

private:
    /** @brief Format a duration in seconds to a "m:ss" string. */
    static juce::String formatDuration(double seconds);

    /** @brief Read the duration of an audio file in seconds. */
    double getFileDuration(const juce::File& file);

    /** @brief Search the track's folder for a cover image file. */
    juce::Image extractAlbumArt(const juce::File& file);

    /** @brief Rebuild filteredIndices based on the current search text. */
    void applySearch();

    /** @return The TrackInfo for a row in the filtered view. */
    const TrackInfo& getFilteredTrack(int filteredRow) const;

    /** @return The index into the master tracks array for a filtered row. */
    int getMasterIndex(int filteredRow) const;

    /** @brief Remove a track by its index in the master tracks array. */
    void removeTrack(int masterIndex);

    /** @return The path to the library.xml persistence file on disk. */
    juce::File getLibraryFile() const;

    juce::ListBox    listBox;
    juce::TextButton loadFilesButton;
    juce::TextEditor searchBox;

    std::vector<TrackInfo> tracks;
    std::vector<int>       filteredIndices;

    juce::AudioFormatManager formatManager;
    LoadCallback deck1Callback;
    LoadCallback deck2Callback;

    // Drag-reorder state
    int dragSourceRow = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};