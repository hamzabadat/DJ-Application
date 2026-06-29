/*
  ==============================================================================
    DJAudioPlayer.cpp
    Audio player with transport, 3-band EQ, hot cues, speed control and BPM
  ==============================================================================
*/

#include "DJAudioPlayer.h"

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse)
{
}

DJAudioPlayer::~DJAudioPlayer()
{
    transportSource.setSource(nullptr);
}

//==============================================================================
// AudioSource

void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    updateEQFilters();
}

void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Handle looping
    if (looping && !transportSource.isPlaying() &&
        transportSource.getCurrentPosition() >= transportSource.getLengthInSeconds() - 0.05)
    {
        transportSource.setPosition(0.0);
        transportSource.start();
    }

    resampleSource.getNextAudioBlock(bufferToFill);

    // Apply 3-band EQ via IIR filters
    auto* buffer = bufferToFill.buffer;
    int startSample = bufferToFill.startSample;
    int numSamples = bufferToFill.numSamples;


    if (buffer->getNumChannels() >= 1)
    {
        float* dataL = buffer->getWritePointer(0, startSample);
        bassFilterL.processSamples(dataL, numSamples);
        midFilterL.processSamples(dataL, numSamples);
        trebleFilterL.processSamples(dataL, numSamples);
    }
    if (buffer->getNumChannels() >= 2)
    {
        float* dataR = buffer->getWritePointer(1, startSample);
        bassFilterR.processSamples(dataR, numSamples);
        midFilterR.processSamples(dataR, numSamples);
        trebleFilterR.processSamples(dataR, numSamples);
    }

    // Measure peak level for VU meter
    float peak = 0.0f;
    for (int ch = 0; ch < buffer->getNumChannels(); ++ch)
    {
        const float* data = buffer->getReadPointer(ch, startSample);
        for (int s = 0; s < numSamples; ++s)
            peak = std::max(peak, std::abs(data[s]));
    }
    peakLevel.store(juce::jlimit(0.0f, 1.0f, peak));
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

//==============================================================================
// Transport

void DJAudioPlayer::loadURL(juce::URL audioURL)
{
    currentFileName = audioURL.getFileName();
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
            new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());
        updateEQFilters();
    }
}

void DJAudioPlayer::start()
{
    transportSource.start();
}

void DJAudioPlayer::stop()
{
    transportSource.stop();
}

bool DJAudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

void DJAudioPlayer::setLooping(bool shouldLoop)
{
    looping = shouldLoop;
    if (readerSource != nullptr)
        readerSource->setLooping(shouldLoop);
}

bool DJAudioPlayer::isLooping() const
{
    return looping;
}

//==============================================================================
// Gain / Speed / Position

void DJAudioPlayer::setGain(double gain)
{
    gain = juce::jlimit(0.0, 1.0, gain);
    transportSource.setGain(static_cast<float>(gain));
}

void DJAudioPlayer::setSpeed(double ratio)
{
    ratio = juce::jlimit(0.01, 4.0, ratio);
    resampleSource.setResamplingRatio(ratio);
}

void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    pos = juce::jlimit(0.0, 1.0, pos);
    double posInSecs = transportSource.getLengthInSeconds() * pos;
    setPosition(posInSecs);
}

double DJAudioPlayer::getCurrentPositionSecs() const
{
    return transportSource.getCurrentPosition();
}

double DJAudioPlayer::getPositionRelative() const
{
    double length = transportSource.getLengthInSeconds();
    if (length <= 0.0) return 0.0;
    return transportSource.getCurrentPosition() / length;
}

double DJAudioPlayer::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

juce::String DJAudioPlayer::getCurrentFileName() const
{
    return currentFileName;
}

//==============================================================================
// EQ

void DJAudioPlayer::setBassGain(float gainDb)
{
    bassGainDb = gainDb;
    updateEQFilters();
}

void DJAudioPlayer::setMidGain(float gainDb)
{
    midGainDb = gainDb;
    updateEQFilters();
}

void DJAudioPlayer::setTrebleGain(float gainDb)
{
    trebleGainDb = gainDb;
    updateEQFilters();
}

void DJAudioPlayer::updateEQFilters()
{
    if (currentSampleRate <= 0.0) return;

    // Bass: low-shelf at 200 Hz
    auto bassCoeffs = juce::IIRCoefficients::makeLowShelf(
        currentSampleRate, 200.0, 0.7, juce::Decibels::decibelsToGain(bassGainDb));
    bassFilterL.setCoefficients(bassCoeffs);
    bassFilterR.setCoefficients(bassCoeffs);

    // Mid: peak/bell at 1000 Hz, Q=1.0
    auto midCoeffs = juce::IIRCoefficients::makePeakFilter(
        currentSampleRate, 1000.0, 1.0, juce::Decibels::decibelsToGain(midGainDb));
    midFilterL.setCoefficients(midCoeffs);
    midFilterR.setCoefficients(midCoeffs);

    // Treble: high-shelf at 8000 Hz
    auto trebleCoeffs = juce::IIRCoefficients::makeHighShelf(
        currentSampleRate, 8000.0, 0.7, juce::Decibels::decibelsToGain(trebleGainDb));
    trebleFilterL.setCoefficients(trebleCoeffs);
    trebleFilterR.setCoefficients(trebleCoeffs);
}

//==============================================================================
// Hot Cues

void DJAudioPlayer::setHotCue(int index)
{
    if (index < 0 || index >= NUM_HOT_CUES) return;
    hotCues[index].isSet = true;
    hotCues[index].positionSecs = transportSource.getCurrentPosition();
    hotCues[index].label = juce::String(index + 1);
}

void DJAudioPlayer::triggerHotCue(int index)
{
    if (index < 0 || index >= NUM_HOT_CUES) return;
    if (!hotCues[index].isSet) return;
    transportSource.setPosition(hotCues[index].positionSecs);
}

void DJAudioPlayer::clearHotCue(int index)
{
    if (index < 0 || index >= NUM_HOT_CUES) return;
    hotCues[index] = HotCue{};
}

void DJAudioPlayer::clearAllHotCues()
{
    for (auto& cue : hotCues)
        cue = HotCue{};
}

HotCue DJAudioPlayer::getHotCue(int index) const
{
    if (index < 0 || index >= NUM_HOT_CUES) return HotCue{};
    return hotCues[index];
}

void DJAudioPlayer::setHotCueData(int index, const HotCue& cue)
{
    if (index < 0 || index >= NUM_HOT_CUES) return;
    hotCues[index] = cue;
}

//==============================================================================
// Peak Level

float DJAudioPlayer::getPeakLevel() const
{
    return peakLevel.load();
}

//==============================================================================
// BPM

void DJAudioPlayer::setBPM(double value)
{
    bpm = value;
}

double DJAudioPlayer::getBPM() const
{
    return bpm;
}