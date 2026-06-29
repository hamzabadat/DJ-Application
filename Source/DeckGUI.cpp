/*
  ==============================================================================
    DeckGUI.cpp
    DJ deck UI component implementation
  ==============================================================================
*/

#include "DeckGUI.h"

// Colour palette used consistently across all deck components
static const juce::Colour BG_DECK{ 0xff0d0d16 };
static const juce::Colour BG_PANEL{ 0xff131320 };
static const juce::Colour BG_CONTROL{ 0xff1c1c2e };
static const juce::Colour ACCENT_PINK{ 0xfffc3c6e };
static const juce::Colour ACCENT_BLUE{ 0xff3d9bff };
static const juce::Colour ACCENT_TEAL{ 0xff30d5c8 };
static const juce::Colour TEXT_WHITE{ 0xffffffff };
static const juce::Colour TEXT_DIM{ 0xff8e8e9a };
static const juce::Colour DIVIDER{ 0xff222230 };

//==============================================================================
// RotaryKnob

RotaryKnob::RotaryKnob(const juce::String& labelText,
    double minVal, double maxVal, double defaultVal)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    slider.setRange(minVal, maxVal, 0.1);
    slider.setValue(defaultVal);
    slider.setColour(juce::Slider::rotarySliderFillColourId, ACCENT_TEAL);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a2a40));
    slider.setColour(juce::Slider::thumbColourId, ACCENT_PINK);
    slider.setColour(juce::Slider::textBoxTextColourId, TEXT_WHITE);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, BG_CONTROL);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, TEXT_DIM);
    addAndMakeVisible(label);
}

void RotaryKnob::resized()
{
    auto area = getLocalBounds();
    label.setBounds(area.removeFromBottom(18));
    slider.setBounds(area);
}

//==============================================================================
// TimeDisplay

TimeDisplay::TimeDisplay() {}

void TimeDisplay::setTime(double positionSecs, double lengthSecs)
{
    position = positionSecs;
    length = lengthSecs;
    repaint();
}

juce::String TimeDisplay::formatTime(double secs, bool showMillis)
{
    if (secs < 0.0) secs = 0.0;
    int    totalMs = static_cast<int>(secs * 1000.0);
    int    ms = totalMs % 1000 / 10;   // 2-digit centiseconds
    int    totalSec = totalMs / 1000;
    int    s = totalSec % 60;
    int    m = totalSec / 60;

    if (showMillis)
        return juce::String::formatted("%d:%02d.%02d", m, s, ms);
    else
        return juce::String::formatted("%d:%02d", m, s);
}

void TimeDisplay::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    float w = b.getWidth();
    float h = b.getHeight();

    // Background
    g.setColour(juce::Colour(0xff0a0a18));
    g.fillRoundedRectangle(b, 6.0f);
    g.setColour(juce::Colour(0xff1e1e30));
    g.drawRoundedRectangle(b.reduced(0.5f), 6.0f, 1.0f);

    if (length <= 0.0)
    {
        g.setColour(juce::Colour(0xff3a3a4a));
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
        g.drawText("--:-- / --:--", b.reduced(4.0f, 0.0f), juce::Justification::centred);
        return;
    }

    double remaining = length - position;

    // Layout: left half = elapsed, right half = remaining
    // Leave 5px padding top/bottom, 4px progress bar at bottom
    float padX = 4.0f;
    float padTop = 4.0f;
    float barH = 4.0f;
    float barGap = 3.0f;
    float innerH = h - padTop - barH - barGap;

    float halfW = w * 0.5f;

    // --- Label row (top, small) ---
    float labelH = juce::jmin(12.0f, innerH * 0.28f);
    float labelY = b.getY() + padTop;

    g.setColour(juce::Colour(0xff5a5a6a));
    g.setFont(juce::Font(juce::jmax(8.0f, labelH * 0.85f), juce::Font::bold));
    g.drawText("ELAPSED",
        (int)(b.getX() + padX), (int)labelY,
        (int)(halfW - padX * 2), (int)labelH,
        juce::Justification::centredLeft);
    g.drawText("REMAINING",
        (int)(b.getX() + halfW + padX), (int)labelY,
        (int)(halfW - padX * 2), (int)labelH,
        juce::Justification::centredLeft);

    // --- Time value row (below labels) ---
    float valueY = labelY + labelH + 1.0f;
    float valueH = innerH - labelH - 1.0f;
    float fontSize = juce::jmax(10.0f, valueH * 0.72f);

    // Elapsed — teal
    g.setColour(juce::Colour(0xff30d5c8));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), fontSize, juce::Font::bold));
    g.drawText(formatTime(position, false),
        (int)(b.getX() + padX), (int)valueY,
        (int)(halfW - padX * 2), (int)valueH,
        juce::Justification::centredLeft);

    // Remaining — dimmed
    g.setColour(juce::Colour(0xff8e8e9a));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), fontSize, juce::Font::plain));
    g.drawText("-" + formatTime(remaining, false),
        (int)(b.getX() + halfW + padX), (int)valueY,
        (int)(halfW - padX * 2), (int)valueH,
        juce::Justification::centredLeft);

    // Thin divider between elapsed and remaining
    g.setColour(juce::Colour(0xff2a2a3a));
    g.fillRect(b.getX() + halfW - 0.5f, b.getY() + padTop, 1.0f, innerH);

    // Progress bar at very bottom (existing posSlider style, no fill colour change)
    float barY = b.getBottom() - barH - 1.0f;
    float barX = b.getX() + padX;
    float barW = w - padX * 2.0f;
    float ratio = juce::jlimit(0.0f, 1.0f, (float)(position / length));

    g.setColour(juce::Colour(0xff1e1e30));
    g.fillRoundedRectangle(barX, barY, barW, barH, 2.0f);
    g.setColour(juce::Colour(0xff30d5c8));
    g.fillRoundedRectangle(barX, barY, barW * ratio, barH, 2.0f);
}

//==============================================================================
// VUMeter

VUMeter::VUMeter() {}

void VUMeter::setLevel(float newLevel)
{
    level.store(juce::jlimit(0.0f, 1.0f, newLevel));
}

void VUMeter::paint(juce::Graphics& g)
{
    // Smooth towards target
    float target = level.load();
    displayLevel += (target - displayLevel) * 0.25f;
    if (displayLevel < 0.001f) displayLevel = 0.0f;

    // Peak hold
    if (displayLevel >= peakLevel)
    {
        peakLevel = displayLevel;
        peakHoldTimer = 40;  // hold for ~2s at 20fps
    }
    else if (peakHoldTimer > 0)
    {
        --peakHoldTimer;
    }
    else
    {
        peakLevel -= 0.01f;
        if (peakLevel < 0.0f) peakLevel = 0.0f;
    }

    auto b = getLocalBounds().toFloat();
    float w = b.getWidth();
    float h = b.getHeight();

    // Background track
    g.setColour(juce::Colour(0xff0a0a14));
    g.fillRoundedRectangle(b, 3.0f);

    // Number of segments
    int numSegs = 20;
    float segH = (h - 2.0f) / numSegs;
    float segGap = 1.5f;
    int   litSegs = juce::roundToInt(displayLevel * numSegs);

    for (int i = 0; i < numSegs; ++i)
    {
        float y = b.getBottom() - 1.0f - (i + 1) * segH + segGap * 0.5f;
        float fraction = (float)i / (numSegs - 1);

        juce::Colour segCol;
        if (fraction > 0.85f) segCol = juce::Colour(0xffff453a); // red top
        else if (fraction > 0.65f) segCol = juce::Colour(0xffffcc00); // yellow mid
        else                       segCol = ACCENT_TEAL;               // teal bottom

        if (i < litSegs)
            g.setColour(segCol);
        else
            g.setColour(segCol.withAlpha(0.12f));  // unlit — very dim

        g.fillRoundedRectangle(1.0f, y, w - 2.0f, segH - segGap, 1.5f);
    }

    // Peak marker
    if (peakLevel > 0.01f)
    {
        float peakY = b.getBottom() - 1.0f - peakLevel * h;
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.fillRect(1.0f, peakY, w - 2.0f, 2.0f);
    }
}

//==============================================================================
// VinylSpinner

VinylSpinner::VinylSpinner() {}

void VinylSpinner::tick(float degreesPerTick)
{
    if (!isPlaying) return;
    angle += degreesPerTick;
    if (angle >= 360.0f) angle -= 360.0f;
    repaint();
}

void VinylSpinner::setPlaying(bool playing)
{
    isPlaying = playing;
    if (!playing) repaint();
}

void VinylSpinner::setAlbumArt(const juce::Image& art)
{
    albumArt = art;
    repaint();
}

void VinylSpinner::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float r = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f;

    // Outer vinyl body — very dark grey with subtle shine
    g.setColour(juce::Colour(0xff252535));
    g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);

    // Grooves — concentric rings drawn with rotation applied
    {
        juce::Graphics::ScopedSaveState ss(g);
        g.addTransform(juce::AffineTransform::rotation(
            juce::degreesToRadians(angle), cx, cy));

        for (int i = 1; i <= 8; ++i)
        {
            float gr = r * (0.45f + i * 0.055f);
            g.setColour(juce::Colour(0xff3a3a50).withAlpha(0.9f));
            g.drawEllipse(cx - gr, cy - gr, gr * 2.0f, gr * 2.0f, 0.7f);
        }

        // Sheen arc
        juce::Path sheen;
        sheen.addArc(cx - r * 0.9f, cy - r * 0.9f, r * 1.8f, r * 1.8f,
            -0.8f, 0.4f, true);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.strokePath(sheen, juce::PathStrokeType(r * 0.3f));
    }

    // Album art or label in the centre circle
    float labelR = r * 0.35f;
    {
        juce::Graphics::ScopedSaveState ss(g);
        juce::Path circle;
        circle.addEllipse(cx - labelR, cy - labelR, labelR * 2.0f, labelR * 2.0f);
        g.reduceClipRegion(circle);

        if (albumArt.isValid())
        {
            g.drawImageWithin(albumArt,
                (int)(cx - labelR), (int)(cy - labelR),
                (int)(labelR * 2.0f), (int)(labelR * 2.0f),
                juce::RectanglePlacement::centred |
                juce::RectanglePlacement::fillDestination);
        }
        else
        {
            juce::ColourGradient grad(
                juce::Colour(0xff5a2080), cx, cy - labelR,
                juce::Colour(0xff2a1a4a), cx, cy + labelR, false);
            g.setGradientFill(grad);
            g.fillEllipse(cx - labelR, cy - labelR, labelR * 2.0f, labelR * 2.0f);
        }
    }

    // Centre spindle hole
    float spindleR = r * 0.045f;
    g.setColour(juce::Colour(0xff080810));
    g.fillEllipse(cx - spindleR, cy - spindleR, spindleR * 2.0f, spindleR * 2.0f);
    g.setColour(juce::Colour(0xff303040));
    g.drawEllipse(cx - spindleR, cy - spindleR, spindleR * 2.0f, spindleR * 2.0f, 1.0f);

    // Outer rim
    g.setColour(juce::Colour(0xff5a5a7a));
    g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f);

    // Paused overlay
    if (!isPlaying)
    {
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
    }
}

//==============================================================================
// DeckGUI

DeckGUI::DeckGUI(DJAudioPlayer* playerToControl,
    juce::AudioFormatManager& formatManagerToUse,
    juce::AudioThumbnailCache& cacheToUse,
    const juce::String& name,
    CueLibrary& cueLibraryToUse)
    : player(playerToControl),
    deckName(name),
    cueLibrary(cueLibraryToUse),
    loadButton("Load"),
    playButton("PLAY"),
    stopButton("STOP"),
    loopButton("LOOP"),
    bassKnob("Bass", -12.0, 12.0, 0.0),
    midKnob("Mid", -12.0, 12.0, 0.0),
    trebleKnob("Treble", -12.0, 12.0, 0.0),
    waveformDisplay(formatManagerToUse, cacheToUse),
    clearAllCuesButton("Clear Cues")
{
    // Track name label
    trackNameLabel.setText("No track loaded", juce::dontSendNotification);
    trackNameLabel.setJustificationType(juce::Justification::centredLeft);
    trackNameLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    trackNameLabel.setColour(juce::Label::textColourId, TEXT_WHITE);
    addAndMakeVisible(trackNameLabel);

    // Transport buttons
    auto styleBtn = [&](juce::TextButton& btn, juce::Colour fg, juce::Colour bg)
        {
            btn.setColour(juce::TextButton::buttonColourId, bg);
            btn.setColour(juce::TextButton::buttonOnColourId, fg.withAlpha(0.25f));
            btn.setColour(juce::TextButton::textColourOffId, fg);
            btn.setColour(juce::TextButton::textColourOnId, fg);
            btn.addListener(this);
            addAndMakeVisible(btn);
        };

    styleBtn(loadButton, TEXT_DIM, BG_CONTROL);
    styleBtn(playButton, ACCENT_TEAL, BG_CONTROL);
    styleBtn(stopButton, ACCENT_PINK, BG_CONTROL);
    styleBtn(loopButton, ACCENT_BLUE, BG_CONTROL);
    styleBtn(clearAllCuesButton, TEXT_DIM, BG_CONTROL);

    // Sliders
    auto styleSlider = [&](juce::Slider& s, juce::Colour track, juce::Colour thumb)
        {
            s.setSliderStyle(juce::Slider::LinearHorizontal);
            s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
            s.setColour(juce::Slider::trackColourId, track);
            s.setColour(juce::Slider::thumbColourId, thumb);
            s.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1a2a));
            s.setColour(juce::Slider::textBoxTextColourId, TEXT_WHITE);
            s.setColour(juce::Slider::textBoxBackgroundColourId, BG_CONTROL);
            s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            s.addListener(this);
            addAndMakeVisible(s);
        };

    styleSlider(volSlider, ACCENT_PINK, juce::Colours::white);
    styleSlider(speedSlider, ACCENT_BLUE, juce::Colours::white);
    styleSlider(posSlider, ACCENT_TEAL, juce::Colours::orange);

    volSlider.setRange(0.0, 1.0, 0.01);  volSlider.setValue(0.5);
    speedSlider.setRange(0.25, 4.0, 0.01);  speedSlider.setValue(1.0);
    posSlider.setRange(0.0, 1.0, 0.001); posSlider.setValue(0.0);

    auto makeLabel = [&](juce::Label& l, const juce::String& t)
        {
            l.setText(t, juce::dontSendNotification);
            l.setFont(juce::Font(11.0f, juce::Font::bold));
            l.setColour(juce::Label::textColourId, TEXT_DIM);
            addAndMakeVisible(l);
        };
    makeLabel(volLabel, "VOL");
    makeLabel(speedLabel, "SPD");
    makeLabel(posLabel, "TIME");

    // EQ
    addAndMakeVisible(bassKnob);
    addAndMakeVisible(midKnob);
    addAndMakeVisible(trebleKnob);
    bassKnob.slider.addListener(this);
    midKnob.slider.addListener(this);
    trebleKnob.slider.addListener(this);

    // Waveform
    addAndMakeVisible(waveformDisplay);

    // Hot cues
    static const juce::Colour cueColours[DJAudioPlayer::NUM_HOT_CUES] = {
        juce::Colour(0xfffc3c6e), juce::Colour(0xff3d9bff),
        juce::Colour(0xff30d5c8), juce::Colour(0xffffcc00),
        juce::Colour(0xffbf5af2), juce::Colour(0xffff9f0a),
        juce::Colour(0xff32d74b), juce::Colour(0xffff453a)
    };
    for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
    {
        hotCueButtons[i].setButtonText(juce::String(i + 1));
        hotCueButtons[i].setColour(juce::TextButton::buttonColourId, BG_CONTROL);
        hotCueButtons[i].setColour(juce::TextButton::buttonOnColourId, cueColours[i]);
        hotCueButtons[i].setColour(juce::TextButton::textColourOffId, TEXT_DIM);
        hotCueButtons[i].addListener(this);
        addAndMakeVisible(hotCueButtons[i]);

        // Right-click listener — clears this individual cue
        cueListeners[i].onRightClick = [this, i]()
            {
                player->clearHotCue(i);
                saveCuesForCurrentTrack();
                updateHotCueButtons();
            };
        hotCueButtons[i].addMouseListener(&cueListeners[i], false);
    }

    // BPM tap tempo button
    tapButton.setButtonText("TAP");
    tapButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1c1c2e));
    tapButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffcc00));
    tapButton.addListener(this);
    addAndMakeVisible(tapButton);

    bpmLabel.setText("BPM: --", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::bold));
    bpmLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffcc00));
    addAndMakeVisible(bpmLabel);

    // VU meter, Vinyl, Time display
    addAndMakeVisible(vuMeter);
    addAndMakeVisible(vinylSpinner);
    addAndMakeVisible(timeDisplay);

    setInterceptsMouseClicks(true, true);
    startTimer(50);  // 20fps
}

DeckGUI::~DeckGUI() { stopTimer(); }

//==============================================================================

void DeckGUI::paint(juce::Graphics& g)
{
    g.fillAll(BG_DECK);

    // Header gradient
    juce::ColourGradient hdr(juce::Colour(0xff1a0a24), 0, 0,
        BG_PANEL, 0, 32, false);
    g.setGradientFill(hdr);
    g.fillRect(0, 0, getWidth(), 32);

    // Deck name
    g.setColour(deckName == "Deck 1" ? ACCENT_PINK : ACCENT_BLUE);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(deckName, 0, 0, getWidth(), 32, juce::Justification::centred);

    // Header underline
    g.setColour((deckName == "Deck 1" ? ACCENT_PINK : ACCENT_BLUE).withAlpha(0.5f));
    g.fillRect(0, 31, getWidth(), 1);
}

void DeckGUI::resized()
{
    // Helper: safely remove a strip, never going negative
    auto take = [](juce::Rectangle<int>& r, int h) -> juce::Rectangle<int>
        {
            h = juce::jmin(h, r.getHeight());
            return r.removeFromTop(juce::jmax(0, h));
        };

    auto area = getLocalBounds();
    const int m = 4;

    // Guard: if window is tiny just hide everything gracefully
    if (area.getWidth() < 100 || area.getHeight() < 100)
        return;

    take(area, 32);  // header

    // Transport row
    {
        auto row = take(area, 36).reduced(m, m);
        int bw = juce::jmax(1, row.getWidth() / 4);
        loadButton.setBounds(row.removeFromLeft(bw).reduced(2, 0));
        playButton.setBounds(row.removeFromLeft(bw).reduced(2, 0));
        stopButton.setBounds(row.removeFromLeft(bw).reduced(2, 0));
        loopButton.setBounds(row.reduced(2, 0));
    }
    take(area, m);

    // Vinyl | Time display | VU meter
    {
        int vinylSize = juce::jmin(110, area.getHeight() / 4);
        int vuWidth = 14;
        auto row = take(area, vinylSize);

        vuMeter.setBounds(row.removeFromRight(vuWidth).reduced(0, 2));
        vinylSpinner.setBounds(row.removeFromLeft(vinylSize));
        row.reduce(m, 0);

        if (row.getHeight() > 0)
        {
            trackNameLabel.setBounds(row.removeFromTop(juce::jmin(20, row.getHeight())));
            take(row, m);
            if (row.getHeight() > 0)
                timeDisplay.setBounds(row);
        }
    }
    take(area, m);

    // Volume
    {
        auto row = take(area, 22).reduced(m, 2);
        volLabel.setBounds(row.removeFromLeft(38));
        volSlider.setBounds(row);
    }
    take(area, m);

    // Speed
    {
        auto row = take(area, 22).reduced(m, 2);
        speedLabel.setBounds(row.removeFromLeft(38));
        speedSlider.setBounds(row);
    }
    take(area, m + 2);

    // EQ knobs
    {
        auto row = take(area, 92).reduced(m, 0);
        int kw = juce::jmax(1, row.getWidth() / 3);
        bassKnob.setBounds(row.removeFromLeft(kw).reduced(3, 0));
        midKnob.setBounds(row.removeFromLeft(kw).reduced(3, 0));
        trebleKnob.setBounds(row.reduced(3, 0));
    }
    take(area, m);

    // Waveform
    waveformDisplay.setBounds(take(area, 66).reduced(m, 0));
    take(area, 2);

    // Position slider
    {
        auto row = take(area, 22).reduced(m, 2);
        posLabel.setBounds(row.removeFromLeft(38));
        posSlider.setBounds(row);
    }
    take(area, m);

    // Hot cue buttons — 2 rows of 4
    if (area.getHeight() >= 54)
    {
        auto cueArea = take(area, 54).reduced(m, 0);
        int cw = juce::jmax(1, cueArea.getWidth() / 4);
        auto r1 = cueArea.removeFromTop(26);
        auto r2 = cueArea;
        for (int i = 0; i < 4; ++i) hotCueButtons[i].setBounds(r1.removeFromLeft(cw).reduced(2, 1));
        for (int i = 4; i < 8; ++i) hotCueButtons[i].setBounds(r2.removeFromLeft(cw).reduced(2, 1));
    }
    take(area, m);

    // BPM row: [TAP button] [BPM label] [Clear Cues button]
    if (area.getHeight() >= 26)
    {
        auto bpmRow = take(area, 26).reduced(m, 0);
        int tapW = 52;
        int clearW = 90;
        tapButton.setBounds(bpmRow.removeFromLeft(tapW).reduced(1, 0));
        clearAllCuesButton.setBounds(bpmRow.removeFromRight(clearW).reduced(1, 0));
        bpmLabel.setBounds(bpmRow.reduced(2, 0));
    }
}

//==============================================================================

void DeckGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fChooser.launchAsync(juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.existsAsFile())
                    loadTrack(juce::URL{ f }, f.getFileNameWithoutExtension());
            });
        return;
    }

    if (button == &playButton) { player->start(); vinylSpinner.setPlaying(true);  return; }
    if (button == &stopButton) { player->stop();  vinylSpinner.setPlaying(false); return; }

    if (button == &loopButton)
    {
        bool nowLooping = !player->isLooping();
        player->setLooping(nowLooping);
        loopButton.setColour(juce::TextButton::buttonColourId,
            nowLooping ? ACCENT_BLUE.withAlpha(0.35f) : BG_CONTROL);
        return;
    }

    if (button == &clearAllCuesButton)
    {
        player->clearAllHotCues();
        saveCuesForCurrentTrack();
        updateHotCueButtons();
        return;
    }

    // TAP TEMPO
    if (button == &tapButton)
    {
        juce::int64 nowMs = juce::Time::currentTimeMillis();
        juce::int64 gap = nowMs - lastTapMs;

        // Reset if more than 3 seconds since last tap
        if (lastTapMs == 0 || gap > 3000)
        {
            tapCount = 0;
            tapBpmAccum = 0.0;
        }

        if (lastTapMs > 0 && gap > 0 && gap < 3000)
        {
            double intervalBpm = 60000.0 / (double)gap;
            if (intervalBpm > 20.0 && intervalBpm < 300.0)
            {
                tapBpmAccum += intervalBpm;
                ++tapCount;
                double avgBpm = tapBpmAccum / tapCount;
                player->setBPM(avgBpm);
                bpmLabel.setText("BPM: " + juce::String(avgBpm, 1),
                    juce::dontSendNotification);
            }
        }

        lastTapMs = nowMs;

        // Flash the tap button yellow briefly
        tapButton.setColour(juce::TextButton::buttonColourId,
            juce::Colour(0xffffcc00).withAlpha(0.4f));
        juce::Timer::callAfterDelay(120, [this]
            {
                tapButton.setColour(juce::TextButton::buttonColourId,
                    juce::Colour(0xff1c1c2e));
            });
        return;
    }

    for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
    {
        if (button == &hotCueButtons[i])
        {
            if (player->getHotCue(i).isSet)
                player->triggerHotCue(i);
            else
            {
                player->setHotCue(i);
                saveCuesForCurrentTrack();
                updateHotCueButtons();
            }
            return;
        }
    }
}

void DeckGUI::sliderValueChanged(juce::Slider* s)
{
    if (s == &volSlider)         player->setGain(s->getValue());
    else if (s == &speedSlider)       player->setSpeed(s->getValue());
    else if (s == &posSlider)         player->setPositionRelative(s->getValue());
    else if (s == &bassKnob.slider)   player->setBassGain(static_cast<float>(s->getValue()));
    else if (s == &midKnob.slider)    player->setMidGain(static_cast<float>(s->getValue()));
    else if (s == &trebleKnob.slider) player->setTrebleGain(static_cast<float>(s->getValue()));
}

bool DeckGUI::isInterestedInFileDrag(const juce::StringArray&) { return true; }

void DeckGUI::filesDropped(const juce::StringArray& files, int, int)
{
    if (!files.isEmpty())
    {
        juce::File f{ files[0] };
        if (f.existsAsFile())
            loadTrack(juce::URL{ f }, f.getFileNameWithoutExtension());
    }
}

void DeckGUI::timerCallback()
{
    bool playing = player->isPlaying();
    vinylSpinner.setPlaying(playing);
    if (playing)
        vinylSpinner.tick(1.8f);   // ~33rpm feel at 20fps

    vuMeter.repaint();

    if (player->getLengthInSeconds() > 0.0)
    {
        posSlider.removeListener(this);
        posSlider.setValue(player->getPositionRelative(), juce::dontSendNotification);
        posSlider.addListener(this);
    }

    waveformDisplay.setPositionRelative(player->getPositionRelative());

    // Update time display
    timeDisplay.setTime(player->getCurrentPositionSecs(), player->getLengthInSeconds());

    std::vector<double> cuePositions;
    double len = player->getLengthInSeconds();
    for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
    {
        auto cue = player->getHotCue(i);
        cuePositions.push_back((cue.isSet && len > 0.0) ? cue.positionSecs / len : -1.0);
    }
    waveformDisplay.setHotCuePositions(cuePositions);
}

void DeckGUI::pushAudioLevel(float peak)
{
    vuMeter.setLevel(peak);
}

void DeckGUI::loadTrack(const juce::URL& url, const juce::String& trackName,
    const juce::Image& art, const juce::String& filePath)
{
    player->loadURL(url);
    waveformDisplay.loadURL(url);
    currentTrackName = trackName;
    currentFilePath = filePath.isNotEmpty() ? filePath
        : url.getLocalFile().getFullPathName();

    trackNameLabel.setText(trackName, juce::dontSendNotification);
    vinylSpinner.setAlbumArt(art);
    vinylSpinner.setPlaying(false);

    // Restore this track's hot cues from the cue library
    player->clearAllHotCues();
    std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES> savedCues{};
    if (cueLibrary.loadCues(currentFilePath, savedCues))
        for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
            player->setHotCueData(i, savedCues[i]);

    updateHotCueButtons();

    // Reset BPM tap state for new track
    tapCount = 0;
    tapBpmAccum = 0.0;
    lastTapMs = 0;
    double savedBpm = player->getBPM();
    bpmLabel.setText(savedBpm > 0.0
        ? "BPM: " + juce::String(savedBpm, 1)
        : "BPM: --",
        juce::dontSendNotification);
}

void DeckGUI::saveCuesForCurrentTrack()
{
    if (currentFilePath.isEmpty()) return;
    std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES> cues{};
    for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
        cues[i] = player->getHotCue(i);
    cueLibrary.saveCues(currentFilePath, cues);
}

void DeckGUI::updateHotCueButtons()
{
    static const juce::Colour cueColours[DJAudioPlayer::NUM_HOT_CUES] = {
        juce::Colour(0xfffc3c6e), juce::Colour(0xff3d9bff),
        juce::Colour(0xff30d5c8), juce::Colour(0xffffcc00),
        juce::Colour(0xffbf5af2), juce::Colour(0xffff9f0a),
        juce::Colour(0xff32d74b), juce::Colour(0xffff453a)
    };
    for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
    {
        bool set = player->getHotCue(i).isSet;
        hotCueButtons[i].setColour(juce::TextButton::buttonColourId,
            set ? cueColours[i].withAlpha(0.9f) : BG_CONTROL);
        hotCueButtons[i].setColour(juce::TextButton::textColourOffId,
            set ? juce::Colours::black : TEXT_DIM);
    }
    repaint();
}

//==============================================================================
// Persistence

juce::ValueTree DeckGUI::saveState() const
{
    juce::ValueTree state("DeckState");
    state.setProperty("deckName", deckName, nullptr);
    state.setProperty("bassGain", player->getBassGain(), nullptr);
    state.setProperty("midGain", player->getMidGain(), nullptr);
    state.setProperty("trebleGain", player->getTrebleGain(), nullptr);
    state.setProperty("volume", volSlider.getValue(), nullptr);
    state.setProperty("speed", speedSlider.getValue(), nullptr);
    state.setProperty("looping", player->isLooping(), nullptr);

    juce::ValueTree cuesNode("HotCues");
    for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
    {
        auto cue = player->getHotCue(i);
        juce::ValueTree c("Cue");
        c.setProperty("index", i, nullptr);
        c.setProperty("isSet", cue.isSet, nullptr);
        c.setProperty("position", cue.positionSecs, nullptr);
        c.setProperty("label", cue.label, nullptr);
        cuesNode.addChild(c, -1, nullptr);
    }
    state.addChild(cuesNode, -1, nullptr);
    return state;
}

void DeckGUI::loadState(const juce::ValueTree& state)
{
    if (!state.isValid()) return;

    float  bass = state.getProperty("bassGain", 0.0f);
    float  mid = state.getProperty("midGain", 0.0f);
    float  treble = state.getProperty("trebleGain", 0.0f);
    double vol = state.getProperty("volume", 0.5);
    double speed = state.getProperty("speed", 1.0);
    bool   loop = state.getProperty("looping", false);

    player->setBassGain(bass);     bassKnob.slider.setValue(bass, juce::dontSendNotification);
    player->setMidGain(mid);       midKnob.slider.setValue(mid, juce::dontSendNotification);
    player->setTrebleGain(treble); trebleKnob.slider.setValue(treble, juce::dontSendNotification);
    player->setGain(vol);          volSlider.setValue(vol, juce::dontSendNotification);
    player->setSpeed(speed);       speedSlider.setValue(speed, juce::dontSendNotification);
    player->setLooping(loop);
    loopButton.setColour(juce::TextButton::buttonColourId,
        loop ? ACCENT_BLUE.withAlpha(0.35f) : BG_CONTROL);

    auto cuesNode = state.getChildWithName("HotCues");
    if (cuesNode.isValid())
    {
        for (int i = 0; i < cuesNode.getNumChildren(); ++i)
        {
            auto cn = cuesNode.getChild(i);
            int  idx = cn.getProperty("index", -1);
            if (idx >= 0 && idx < DJAudioPlayer::NUM_HOT_CUES)
            {
                HotCue cue;
                cue.isSet = cn.getProperty("isSet", false);
                cue.positionSecs = cn.getProperty("position", 0.0);
                cue.label = cn.getProperty("label", "").toString();
                player->setHotCueData(idx, cue);
            }
        }
        updateHotCueButtons();
    }
}