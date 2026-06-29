/*
  ==============================================================================
    PlaylistComponent.cpp
  ==============================================================================
*/

#include "PlaylistComponent.h"

static const juce::Colour BG_DEEP{ 0xff0a0a0f };
static const juce::Colour BG_ROW_SEL{ 0xff1c1c3a };
static const juce::Colour BG_HEADER{ 0xff08080e };
static const juce::Colour ACCENT_PINK{ 0xfffc3c6e };
static const juce::Colour ACCENT_BLUE{ 0xff3d9bff };
static const juce::Colour ACCENT_TEAL{ 0xff30d5c8 };
static const juce::Colour TEXT_PRIMARY{ 0xffffffff };
static const juce::Colour TEXT_DIMMED{ 0xff8e8e9a };
static const juce::Colour DIVIDER{ 0xff222230 };

// Column x-positions (px). Left edge: 18px drag handle + 36px art + 8px gap = TITLE at 62
static const int ART_W = 44;
static const int TITLE_X = 62;
static const int TITLE_W = 476;
static const int DUR_X = 538;
static const int DUR_W = 58;
static const int FILE_X = 596;
static const int FILE_W = 50;
static const int DECK1_X = 646;
static const int DECK1_W = 65;
static const int DECK2_X = 711;
static const int DECK2_W = 65;
static const int DEL_X = 776;
static const int DEL_W = 44;

//==============================================================================
// PlaylistRow

class PlaylistRow : public juce::Component,
    public juce::Button::Listener
{
public:
    std::function<void()>    onDeck1Click;
    std::function<void()>    onDeck2Click;
    std::function<void()>    onDeleteClick;
    std::function<void(int)> onDragMove;
    std::function<void(int)> onDragEnd;

    int rowIndex = -1;

    PlaylistRow()
    {
        auto makeBtn = [this](juce::TextButton& btn, const juce::String& txt,
            juce::Colour fg, juce::Colour bg)
            {
                btn.setButtonText(txt);
                btn.setColour(juce::TextButton::buttonColourId, bg);
                btn.setColour(juce::TextButton::buttonOnColourId, fg.withAlpha(0.3f));
                btn.setColour(juce::TextButton::textColourOffId, fg);
                btn.addListener(this);
                addAndMakeVisible(btn);
            };

        makeBtn(deck1Btn, "D1", ACCENT_BLUE, juce::Colour(0xff151525));
        makeBtn(deck2Btn, "D2", ACCENT_PINK, juce::Colour(0xff151525));
        makeBtn(deleteBtn, "X", TEXT_DIMMED, juce::Colour(0xff151525));
    }

    void update(const juce::String& title, const juce::String& duration,
        const juce::String& fileType, const juce::Image& art,
        bool selected, int row)
    {
        trackTitle = title;
        trackDur = duration;
        trackFile = fileType;
        albumArt = art;
        isSelected = selected;
        rowIndex = row;
        repaint();
    }

    void resized() override
    {
        int pad = 4;
        int btnH = getHeight() - pad * 2;
        deck1Btn.setBounds(DECK1_X + pad, pad, DECK1_W - pad * 2, btnH);
        deck2Btn.setBounds(DECK2_X + pad, pad, DECK2_W - pad * 2, btnH);
        deleteBtn.setBounds(DEL_X + pad, pad, DEL_W - pad * 2, btnH);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(isDragging ? juce::Colour(0xff252540)
            : (isSelected ? BG_ROW_SEL : juce::Colour(0xff0d0d14)));

        if (isSelected && !isDragging)
        {
            g.setColour(ACCENT_PINK);
            g.fillRect(0, 0, 3, getHeight());
        }

        // Drag handle — three dots on left edge
        g.setColour(isDragging ? ACCENT_TEAL : TEXT_DIMMED.withAlpha(0.4f));
        float cx = 8.0f, cy = getHeight() * 0.5f;
        for (int d = -1; d <= 1; ++d)
            g.fillEllipse(cx - 2.0f, cy + d * 5.0f - 2.0f, 4.0f, 4.0f);

        g.setColour(DIVIDER);
        g.fillRect(TITLE_X, getHeight() - 1, getWidth() - TITLE_X, 1);

        juce::Rectangle<float> artRect(18.0f, 4.0f,
            (float)(ART_W - 8), (float)(getHeight() - 8));
        if (albumArt.isValid())
        {
            juce::Path clip;
            clip.addRoundedRectangle(artRect, 4.0f);
            juce::Graphics::ScopedSaveState ss(g);
            g.reduceClipRegion(clip);
            g.drawImageWithin(albumArt,
                (int)artRect.getX(), (int)artRect.getY(),
                (int)artRect.getWidth(), (int)artRect.getHeight(),
                juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
        }
        else
        {
            juce::ColourGradient grad(juce::Colour(0xff2a1a3a), artRect.getTopLeft(),
                juce::Colour(0xff1a1a2e), artRect.getBottomRight(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(artRect, 4.0f);

            // Geometric music note placeholder: two note heads + stem + beam
            g.setColour(TEXT_DIMMED.withAlpha(0.45f));
            float nx = artRect.getCentreX();
            float ny = artRect.getCentreY();
            float nr = artRect.getWidth() * 0.13f;
            g.fillEllipse(nx - nr * 2.8f, ny + nr * 0.6f, nr * 2.0f, nr * 1.5f);
            g.fillEllipse(nx + nr * 0.2f, ny + nr * 0.2f, nr * 2.0f, nr * 1.5f);
            g.fillRect(nx + nr * 2.1f, ny - nr * 2.5f, nr * 0.45f, nr * 3.6f);
            g.fillRect(nx - nr * 0.9f, ny - nr * 2.5f, nr * 3.0f, nr * 0.45f);
        }

        g.setColour(isSelected ? juce::Colours::white : TEXT_PRIMARY.withAlpha(0.92f));
        g.setFont(juce::Font(13.5f, juce::Font::bold));
        g.drawText(trackTitle, TITLE_X, 0, TITLE_W, getHeight(),
            juce::Justification::centredLeft, true);

        g.setColour(TEXT_DIMMED);
        g.setFont(12.0f);
        g.drawText(trackDur, DUR_X, 0, DUR_W, getHeight(), juce::Justification::centred, true);
        g.drawText(trackFile, FILE_X, 0, FILE_W, getHeight(), juce::Justification::centred, true);

        if (showDropAbove)
        {
            g.setColour(ACCENT_TEAL);
            g.fillRect(0, 0, getWidth(), 2);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (auto* lb = findParentComponentOfClass<juce::ListBox>())
            lb->selectRow(rowIndex);

        if (e.x < 18)
        {
            isDragging = true;
            dragStartY = e.getScreenPosition().y;
            repaint();
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (!isDragging) return;
        int parentY = e.getScreenPosition().y - getParentComponent()->getScreenY();
        if (onDragMove) onDragMove(parentY);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (!isDragging) return;
        isDragging = false;
        int parentY = e.getScreenPosition().y - getParentComponent()->getScreenY();
        if (onDragEnd) onDragEnd(parentY);
        repaint();
    }

    void setShowDropAbove(bool show)
    {
        if (showDropAbove != show) { showDropAbove = show; repaint(); }
    }

    void buttonClicked(juce::Button* btn) override
    {
        if (btn == &deck1Btn && onDeck1Click)  onDeck1Click();
        if (btn == &deck2Btn && onDeck2Click)  onDeck2Click();
        if (btn == &deleteBtn && onDeleteClick) onDeleteClick();
    }

private:
    juce::TextButton deck1Btn, deck2Btn, deleteBtn;
    juce::String     trackTitle, trackDur, trackFile;
    juce::Image      albumArt;
    bool isSelected = false;
    bool isDragging = false;
    bool showDropAbove = false;
    int  dragStartY = 0;
};

//==============================================================================
// PlaylistComponent

PlaylistComponent::PlaylistComponent()
{
    formatManager.registerBasicFormats();

    searchBox.setTextToShowWhenEmpty("Search library...", TEXT_DIMMED);
    searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1c1c2e));
    searchBox.setColour(juce::TextEditor::textColourId, TEXT_PRIMARY);
    searchBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff2a2a3a));
    searchBox.setColour(juce::TextEditor::focusedOutlineColourId, ACCENT_BLUE);
    searchBox.onTextChange = [this] { applySearch(); };
    addAndMakeVisible(searchBox);

    loadFilesButton.setButtonText("+ Add Tracks");
    loadFilesButton.setColour(juce::TextButton::buttonColourId, ACCENT_PINK);
    loadFilesButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadFilesButton.addListener(this);
    addAndMakeVisible(loadFilesButton);

    listBox.setModel(this);
    listBox.setColour(juce::ListBox::backgroundColourId, BG_DEEP);
    listBox.setRowHeight(44);
    listBox.setMultipleSelectionEnabled(false);
    addAndMakeVisible(listBox);

    loadLibrary();
    applySearch();
}

PlaylistComponent::~PlaylistComponent()
{
    saveLibrary();
}

void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(BG_DEEP);

    g.setColour(juce::Colour(0xff0a0a12));
    g.fillRect(0, 0, getWidth(), 46);

    int headerY = 46;
    int headerH = 26;
    g.setColour(BG_HEADER);
    g.fillRect(0, headerY, getWidth(), headerH);

    g.setColour(TEXT_DIMMED);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("TITLE", TITLE_X, headerY, TITLE_W, headerH, juce::Justification::centredLeft);
    g.drawText("TIME", DUR_X, headerY, DUR_W, headerH, juce::Justification::centred);
    g.drawText("TYPE", FILE_X, headerY, FILE_W, headerH, juce::Justification::centred);
    g.drawText("DECK 1", DECK1_X, headerY, DECK1_W, headerH, juce::Justification::centred);
    g.drawText("DECK 2", DECK2_X, headerY, DECK2_W, headerH, juce::Justification::centred);
    g.drawText("DEL", DEL_X, headerY, DEL_W, headerH, juce::Justification::centred);

    g.setColour(ACCENT_PINK.withAlpha(0.4f));
    g.fillRect(0, headerY + headerH, getWidth(), 1);
}

void PlaylistComponent::resized()
{
    loadFilesButton.setBounds(8, 8, 110, 30);
    searchBox.setBounds(126, 8, getWidth() - 132, 30);

    // List starts below toolbar (46px) + column header (26px) + divider (1px)
    listBox.setBounds(0, 73, getWidth(), getHeight() - 73);
}

//==============================================================================
// ListBoxModel

int PlaylistComponent::getNumRows()
{
    return static_cast<int>(filteredIndices.size());
}

void PlaylistComponent::paintListBoxItem(int, juce::Graphics&, int, int, bool) {}

juce::Component* PlaylistComponent::refreshComponentForRow(int rowNumber, bool isRowSelected,
    juce::Component* existingComponent)
{
    PlaylistRow* row = dynamic_cast<PlaylistRow*>(existingComponent);
    if (row == nullptr)
        row = new PlaylistRow();

    if (rowNumber < 0 || rowNumber >= (int)filteredIndices.size())
        return row;

    const TrackInfo& track = getFilteredTrack(rowNumber);
    row->update(track.title, track.duration, track.fileType,
        track.albumArt, isRowSelected, rowNumber);

    int masterIdx = getMasterIndex(rowNumber);

    row->onDeck1Click = [this, masterIdx]()
        {
            if (masterIdx < (int)tracks.size() && deck1Callback)
                deck1Callback(juce::URL{ juce::File{ tracks[masterIdx].filePath } },
                    tracks[masterIdx].title,
                    tracks[masterIdx].albumArt,
                    tracks[masterIdx].filePath);
        };

    row->onDeck2Click = [this, masterIdx]()
        {
            if (masterIdx < (int)tracks.size() && deck2Callback)
                deck2Callback(juce::URL{ juce::File{ tracks[masterIdx].filePath } },
                    tracks[masterIdx].title,
                    tracks[masterIdx].albumArt,
                    tracks[masterIdx].filePath);
        };

    row->onDeleteClick = [this, masterIdx]()
        {
            removeTrack(masterIdx);
        };

    row->onDragMove = [this](int yInListBox)
        {
            int targetRow = listBox.getRowContainingPosition(0, yInListBox);
            int numRows = (int)filteredIndices.size();
            for (int i = 0; i < numRows; ++i)
                if (auto* r = dynamic_cast<PlaylistRow*>(listBox.getComponentForRowNumber(i)))
                    r->setShowDropAbove(i == targetRow);
        };

    row->onDragEnd = [this, rowNumber](int yInListBox)
        {
            int numRows = (int)filteredIndices.size();
            for (int i = 0; i < numRows; ++i)
                if (auto* r = dynamic_cast<PlaylistRow*>(listBox.getComponentForRowNumber(i)))
                    r->setShowDropAbove(false);

            int targetRow = listBox.getRowContainingPosition(0, yInListBox);
            if (rowNumber >= 0 && targetRow >= 0 && rowNumber != targetRow)
                moveRow(rowNumber, targetRow);
        };

    return row;
}

void PlaylistComponent::moveRow(int fromIndex, int toIndex)
{
    if (fromIndex == toIndex) return;
    if (fromIndex < 0 || fromIndex >= (int)filteredIndices.size()) return;
    if (toIndex < 0 || toIndex >= (int)filteredIndices.size()) return;

    int fromMaster = filteredIndices[fromIndex];
    int toMaster = filteredIndices[toIndex];

    TrackInfo moved = tracks[fromMaster];
    tracks.erase(tracks.begin() + fromMaster);

    // Adjust target index if the erase shifted it
    if (toMaster > fromMaster) --toMaster;
    tracks.insert(tracks.begin() + toMaster, std::move(moved));

    applySearch();
    saveLibrary();
}

//==============================================================================
// Button::Listener

void PlaylistComponent::buttonClicked(juce::Button* button)
{
    if (button != &loadFilesButton) return;

    auto chooser = std::make_shared<juce::FileChooser>(
        "Select audio files...",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.mp3;*.wav;*.aiff;*.flac;*.ogg");

    auto flags = juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::canSelectMultipleItems;

    chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc)
        {
            juce::StringArray paths;
            for (auto& f : fc.getResults())
                paths.add(f.getFullPathName());
            if (!paths.isEmpty())
                addFiles(paths);
        });
}

//==============================================================================
// FileDragAndDropTarget

bool PlaylistComponent::isInterestedInFileDrag(const juce::StringArray&) { return true; }

void PlaylistComponent::filesDropped(const juce::StringArray& files, int, int)
{
    addFiles(files);
}

//==============================================================================
// Public API

void PlaylistComponent::setDeck1LoadCallback(LoadCallback cb) { deck1Callback = std::move(cb); }
void PlaylistComponent::setDeck2LoadCallback(LoadCallback cb) { deck2Callback = std::move(cb); }

void PlaylistComponent::addFiles(const juce::StringArray& files)
{
    for (const auto& path : files)
    {
        juce::File file{ path };
        if (!file.existsAsFile()) continue;

        bool duplicate = false;
        for (const auto& t : tracks)
            if (t.filePath == path) { duplicate = true; break; }
        if (duplicate) continue;

        TrackInfo info;
        info.filePath = path;
        info.fileType = file.getFileExtension();
        info.title = file.getFileNameWithoutExtension();
        info.durationSecs = getFileDuration(file);
        info.duration = formatDuration(info.durationSecs);
        info.albumArt = extractAlbumArt(file);

        tracks.push_back(std::move(info));
    }

    applySearch();
    saveLibrary();
}

void PlaylistComponent::saveLibrary()
{
    juce::XmlElement root("Library");
    for (const auto& track : tracks)
    {
        auto* elem = root.createNewChildElement("Track");
        elem->setAttribute("title", track.title);
        elem->setAttribute("filePath", track.filePath);
        elem->setAttribute("duration", track.duration);
        elem->setAttribute("durationSecs", track.durationSecs);
        elem->setAttribute("fileType", track.fileType);
    }
    getLibraryFile().getParentDirectory().createDirectory();
    root.writeTo(getLibraryFile());
}

void PlaylistComponent::loadLibrary()
{
    auto file = getLibraryFile();
    if (!file.existsAsFile()) return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return;

    tracks.clear();
    for (auto* elem : xml->getChildIterator())
    {
        juce::File f{ elem->getStringAttribute("filePath") };
        if (!f.existsAsFile()) continue;

        TrackInfo info;
        info.title = elem->getStringAttribute("title");
        info.filePath = elem->getStringAttribute("filePath");
        info.duration = elem->getStringAttribute("duration");
        info.durationSecs = elem->getDoubleAttribute("durationSecs");
        info.fileType = elem->getStringAttribute("fileType");
        info.albumArt = extractAlbumArt(f);
        tracks.push_back(std::move(info));
    }
}

//==============================================================================
// Private helpers

juce::String PlaylistComponent::formatDuration(double seconds)
{
    if (seconds <= 0.0) return "--:--";
    int total = static_cast<int>(seconds);
    return juce::String::formatted("%d:%02d", total / 60, total % 60);
}

double PlaylistComponent::getFileDuration(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return 0.0;
    double duration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    delete reader;
    return duration;
}

juce::Image PlaylistComponent::extractAlbumArt(const juce::File& file)
{
    // Search the track's folder for common cover image filenames
    static const char* coverNames[] = {
        "cover.jpg", "cover.jpeg", "cover.png",
        "folder.jpg", "folder.jpeg",
        "artwork.jpg", "artwork.jpeg", "artwork.png",
        "AlbumArt.jpg", "AlbumArtSmall.jpg", nullptr
    };

    juce::File dir = file.getParentDirectory();
    for (int i = 0; coverNames[i] != nullptr; ++i)
    {
        juce::File candidate = dir.getChildFile(coverNames[i]);
        if (candidate.existsAsFile())
        {
            juce::Image img = juce::ImageFileFormat::loadFrom(candidate);
            if (img.isValid())
                return img;
        }
    }
    return {};
}

void PlaylistComponent::applySearch()
{
    juce::String query = searchBox.getText().toLowerCase().trim();
    filteredIndices.clear();

    for (int i = 0; i < (int)tracks.size(); ++i)
        if (query.isEmpty() || tracks[i].title.toLowerCase().contains(query))
            filteredIndices.push_back(i);

    listBox.updateContent();
    listBox.repaint();
}

const TrackInfo& PlaylistComponent::getFilteredTrack(int filteredRow) const
{
    return tracks[filteredIndices[filteredRow]];
}

int PlaylistComponent::getMasterIndex(int filteredRow) const
{
    return filteredIndices[filteredRow];
}

void PlaylistComponent::removeTrack(int masterIndex)
{
    if (masterIndex < 0 || masterIndex >= (int)tracks.size()) return;
    tracks.erase(tracks.begin() + masterIndex);
    applySearch();
    saveLibrary();
}

juce::File PlaylistComponent::getLibraryFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtoDecks")
        .getChildFile("library.xml");
}