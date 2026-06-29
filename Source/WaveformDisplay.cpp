/*
  ==============================================================================
    WaveformDisplay.cpp
    Waveform with gradient fill, playhead glow, and hot cue markers
  ==============================================================================
*/

#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
    juce::AudioThumbnailCache& cacheToUse)
    : audioThumb(1000, formatManagerToUse, cacheToUse)
{
    audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
    audioThumb.removeChangeListener(this);
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background — deep near-black
    g.setColour(juce::Colour(0xff080810));
    g.fillRoundedRectangle(bounds, 6.0f);

    if (!fileLoaded)
    {
        g.setColour(juce::Colour(0xff3a3a4a));
        g.setFont(13.0f);
        g.drawText("Drop a track or press Load", bounds, juce::Justification::centred);
        return;
    }

    // Waveform — teal gradient
    juce::ColourGradient waveGrad(
        juce::Colour(0xff30d5c8), 0, 0,
        juce::Colour(0xff3d9bff), 0, (float)getHeight(), false);
    g.setGradientFill(waveGrad);
    audioThumb.drawChannel(g, getLocalBounds(), 0,
        audioThumb.getTotalLength(), 0, 1.0f);

    // Played portion overlay (darker, to show progress)
    float playX = static_cast<float>(position * getWidth());
    g.setColour(juce::Colour(0x66000000));
    g.fillRect(0.0f, 0.0f, playX, (float)getHeight());

    // Hot cue markers
    static const juce::Colour cueColours[] = {
        juce::Colour(0xfffc3c6e), juce::Colour(0xff3d9bff),
        juce::Colour(0xff30d5c8), juce::Colour(0xffffcc00),
        juce::Colour(0xffbf5af2), juce::Colour(0xffff9f0a),
        juce::Colour(0xff32d74b), juce::Colour(0xffff453a)
    };

    for (int i = 0; i < (int)hotCuePositions.size(); ++i)
    {
        double p = hotCuePositions[i];
        if (p < 0.0) continue;

        float cx = static_cast<float>(p * getWidth());
        auto col = cueColours[i % 8];

        // Glow effect
        g.setColour(col.withAlpha(0.2f));
        g.fillRect(cx - 3.0f, 0.0f, 6.0f, (float)getHeight());

        // Solid line
        g.setColour(col);
        g.fillRect(cx - 1.0f, 0.0f, 2.0f, (float)getHeight());

        // Number tag at top
        g.setColour(col);
        g.fillRoundedRectangle(cx - 6.0f, 2.0f, 12.0f, 14.0f, 3.0f);
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText(juce::String(i + 1), (int)(cx - 6), 2, 12, 14,
            juce::Justification::centred);
    }

    // Playhead — white glowing line
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.fillRect(playX - 3.0f, 0.0f, 6.0f, (float)getHeight());
    g.setColour(juce::Colours::white);
    g.fillRect(playX - 1.0f, 0.0f, 2.0f, (float)getHeight());

    // Border
    g.setColour(juce::Colour(0xff2a2a40));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void WaveformDisplay::resized() {}

void WaveformDisplay::loadURL(juce::URL audioURL)
{
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new juce::URLInputSource(audioURL));
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    if (pos != position)
    {
        position = pos;
        repaint();
    }
}

void WaveformDisplay::setHotCuePositions(const std::vector<double>& positions)
{
    hotCuePositions = positions;
    repaint();
}