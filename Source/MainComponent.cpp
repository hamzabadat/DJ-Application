/*
  ==============================================================================
    MainComponent.cpp
    Top-level application component
  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(1200, 860);

    // Open stereo output; (2,2) avoids driver-level assertion on some Windows devices
    setAudioChannels(2, 2);

    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    addAndMakeVisible(playlistComponent);

    formatManager.registerBasicFormats();

    // Add mixer inputs once here — NOT in prepareToPlay which can be called multiple times
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);

    // Wire playlist load callbacks to deck GUIs
    playlistComponent.setDeck1LoadCallback([this](const juce::URL& url,
        const juce::String& name,
        const juce::Image& art,
        const juce::String& path)
        {
            deckGUI1.loadTrack(url, name, art, path);
        });

    playlistComponent.setDeck2LoadCallback([this](const juce::URL& url,
        const juce::String& name,
        const juce::Image& art,
        const juce::String& path)
        {
            deckGUI2.loadTrack(url, name, art, path);
        });

    // Load persisted deck states (EQ, hot cues)
    loadDeckStates();
}

MainComponent::~MainComponent()
{
    saveDeckStates();
    shutdownAudio();
}

//==============================================================================

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);

    // Push peak levels to VU meters (safe to call from audio thread via atomic)
    deckGUI1.pushAudioLevel(player1.getPeakLevel());
    deckGUI2.pushAudioLevel(player2.getPeakLevel());
}

void MainComponent::releaseResources()
{
    mixerSource.removeAllInputs();
    mixerSource.releaseResources();
    player1.releaseResources();
    player2.releaseResources();
}

//==============================================================================

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff080810));

    // Thin divider between decks and playlist
    g.setColour(juce::Colour(0xfffc3c6e).withAlpha(0.3f));
    int divY = getHeight() * 2 / 3;
    g.fillRect(0, divY, getWidth(), 2);

    // Vertical divider between decks
    g.setColour(juce::Colour(0xff1a1a28));
    g.fillRect(getWidth() / 2, 0, 1, divY);
}

void MainComponent::resized()
{
    int deckHeight = getHeight() * 2 / 3;
    int playlistHeight = getHeight() - deckHeight;
    int deckWidth = getWidth() / 2;

    deckGUI1.setBounds(0, 0, deckWidth, deckHeight);
    deckGUI2.setBounds(deckWidth, 0, getWidth() - deckWidth, deckHeight);
    playlistComponent.setBounds(0, deckHeight, getWidth(), playlistHeight);
}

//==============================================================================
// Persistence

void MainComponent::saveDeckStates()
{
    juce::ValueTree root("OtoDecksState");
    root.addChild(deckGUI1.saveState(), -1, nullptr);
    root.addChild(deckGUI2.saveState(), -1, nullptr);

    auto xml = root.toXmlString();
    auto file = getDeckStateFile();
    file.getParentDirectory().createDirectory();
    file.replaceWithText(xml);
}

void MainComponent::loadDeckStates()
{
    auto file = getDeckStateFile();
    if (!file.existsAsFile()) return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return;

    juce::ValueTree root = juce::ValueTree::fromXml(*xml);
    if (!root.isValid()) return;

    if (root.getNumChildren() >= 1)
        deckGUI1.loadState(root.getChild(0));
    if (root.getNumChildren() >= 2)
        deckGUI2.loadState(root.getChild(1));
}

juce::File MainComponent::getDeckStateFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtoDecks")
        .getChildFile("deckstate.xml");
}