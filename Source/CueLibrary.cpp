/*
  ==============================================================================
    CueLibrary.cpp
    Per-track hot cue persistence
  ==============================================================================
*/
#include "CueLibrary.h"

CueLibrary::CueLibrary()
{
    load();
}

void CueLibrary::saveCues(const juce::String& filePath,
    const std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES>& cues)
{
    if (filePath.isEmpty()) return;
    cueMap[filePath.toStdString()] = cues;
    persist();
}

bool CueLibrary::loadCues(const juce::String& filePath,
    std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES>& cues) const
{
    // Always reset first so stale cues from the previous track never bleed through
    for (auto& c : cues) c = HotCue{};

    if (filePath.isEmpty()) return false;
    auto it = cueMap.find(filePath.toStdString());
    if (it == cueMap.end()) return false;

    cues = it->second;
    return true;
}

void CueLibrary::persist() const
{
    juce::XmlElement root("CueLibrary");

    for (const auto& [path, cues] : cueMap)
    {
        auto* trackElem = root.createNewChildElement("Track");
        trackElem->setAttribute("path", juce::String(path));

        for (int i = 0; i < DJAudioPlayer::NUM_HOT_CUES; ++i)
        {
            auto* cueElem = trackElem->createNewChildElement("Cue");
            cueElem->setAttribute("index", i);
            cueElem->setAttribute("isSet", cues[i].isSet ? 1 : 0);
            cueElem->setAttribute("position", cues[i].positionSecs);
            cueElem->setAttribute("label", cues[i].label);
        }
    }

    auto file = getFile();
    file.getParentDirectory().createDirectory();
    root.writeTo(file);
}

void CueLibrary::load()
{
    auto file = getFile();
    if (!file.existsAsFile()) return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return;

    cueMap.clear();

    for (auto* trackElem : xml->getChildIterator())
    {
        juce::String path = trackElem->getStringAttribute("path");
        if (path.isEmpty()) continue;

        std::array<HotCue, DJAudioPlayer::NUM_HOT_CUES> cues{};

        for (auto* cueElem : trackElem->getChildIterator())
        {
            int idx = cueElem->getIntAttribute("index", -1);
            if (idx < 0 || idx >= DJAudioPlayer::NUM_HOT_CUES) continue;

            cues[idx].isSet = cueElem->getIntAttribute("isSet", 0) != 0;
            cues[idx].positionSecs = cueElem->getDoubleAttribute("position", 0.0);
            cues[idx].label = cueElem->getStringAttribute("label");
        }

        cueMap[path.toStdString()] = cues;
    }
}

juce::File CueLibrary::getFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtoDecks")
        .getChildFile("cues.xml");
}