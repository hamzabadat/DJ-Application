# OtoDecks

A professional dual-deck DJ application built in modern C++ using the JUCE framework. OtoDecks delivers a feature-rich DJ mixing experience with real-time audio processing, advanced waveform visualization, and persistent hot cue management.

## Features

### Audio Playback & Transport
- **Dual Deck Architecture** – Independent stereo decks with synchronized mixing
- **Multi-format Support** – MP3, WAV, AIFF, FLAC, and OGG audio files
- **Speed Control** – Variable playback speed with resampling (0.01x to 4.0x)
- **Looping** – Toggle-able loop playback for extended mix sections

### 3-Band EQ
- **Parametric Equalization** – Bass (low-shelf @ 200Hz), Mid (peak @ 1kHz), Treble (high-shelf @ 8kHz)
- **IIR Filter Processing** – Per-channel stereo DSP chain for transparent frequency shaping
- **Real-time Adjustment** – Instant feedback with smooth gain transitions

### Hot Cue System
- **8 Hot Cues Per Track** – Set and trigger cue points at any position
- **Per-Track Persistence** – Cues are automatically saved and loaded for each track
- **Visual Indicators** – Color-coded waveform markers for quick visual reference
- **Right-click Clearing** – Individual or batch cue management

### Waveform Display
- **Audio Thumbnail Rendering** – Real-time waveform visualization with gradient fill
- **Playhead Indicator** – Glowing playhead showing current playback position
- **Hot Cue Markers** – Color-coded overlays for all 8 cue points per deck
- **Progress Overlay** – Visual indication of played vs. remaining track duration

### Mixing & Monitoring
- **Live VU Meter** – Animated peak-level monitoring with hold function
- **Audio Routing** – Seamless mixing of both decks via stereo mixer
- **Peak Metering** – Thread-safe atomic level updates from audio thread

### Advanced UI Components
- **Vinyl Spinner** – Rotating vinyl record graphic synchronized to playback
- **CDJ-Style Time Display** – Elapsed and remaining time with millisecond precision
- **BPM Tap Tempo** – Calculate track BPM via manual tapping
- **Album Art Display** – Center vinyl with embedded cover art when available

### Music Library
- **Drag-and-Drop** – Add tracks via file browser or drag files directly
- **Drag-to-Reorder** – Organize playlist by dragging rows
- **Search Filtering** – Quick track lookup by name
- **Persistent Library** – Tracks and metadata automatically saved to disk
- **Album Art Extraction** – Automatically detects and displays cover images

### Session Persistence
- **State Saving** – EQ settings and hot cues preserved between sessions
- **XML Storage** – Portable configuration files for reliable data retention
- **Cross-Platform** – Works on macOS, Windows, and Linux

## Technical Highlights

- **Modern C++17** – Type-safe audio processing with smart pointers and standard library utilities
- **JUCE Framework** – Industry-standard GUI and audio abstraction layer
- **Thread-Safe DSP** – Atomic operations and lock-free patterns for real-time audio
- **Clean Architecture** – Modular component design with clear separation of concerns

### Component Overview

| Component | Responsibility |
|-----------|-----------------|
| `DJAudioPlayer` | Audio transport, EQ filters, hot cue management, BPM tracking |
| `DeckGUI` | Dual-deck UI: controls, sliders, waveform, VU meter, vinyl spinner, EQ knobs |
| `PlaylistComponent` | Music library with search, drag-to-reorder, and track metadata |
| `WaveformDisplay` | Real-time waveform visualization with playhead and cue markers |
| `CueLibrary` | Per-track hot cue persistence via XML serialization |
| `MainComponent` | Top-level container and audio routing (both decks → mixer) |

## Build Instructions

### Prerequisites
- **JUCE** (7.0+) – Download from [juce.com](https://juce.com)
- **C++17 Compiler** – GCC 8+, Clang 6+, or MSVC 2017+
- **CMake** (optional) or Projucer GUI

### macOS
1. Open `OtoDecksV2.jucer` in Projucer
2. Select **Xcode** exporter and save
3. Open the generated `.xcodeproj` in Xcode
4. Build and run (⌘B)

### Windows
1. Open `OtoDecksV2.jucer` in Projucer
2. Select **Visual Studio 2019** exporter and save
3. Open the generated `.sln` in Visual Studio
4. Build and run (F5)

### Linux
1. Open `OtoDecksV2.jucer` in Projucer
2. Select **Linux Makefile** exporter and save
3. Run:
   ```bash
   cd Builds/LinuxMakefile
   make
   ./OtoDecksV2
   ```

## Usage

### Loading Tracks
- Click **Load** on a deck to browse files
- Or drag audio files directly onto a deck

### Mixing
1. **Volume** – Adjust per-deck gain sliders
2. **Speed** – Alter playback speed (tempo-sync or time-stretch)
3. **Position** – Manually seek or use the waveform to jump
4. **EQ** – Use the three knobs (Bass, Mid, Treble) to shape frequency response

### Hot Cues
- **Set** – Click the number button while track is playing or at rest
- **Trigger** – Click again to jump to that cue
- **Clear** – Right-click to remove individual cues, or use "Clear All"

### Playlist Management
- **Add** – Use "Load Files" button or drag folders
- **Search** – Type in the search box to filter tracks
- **Load** – Click "Deck 1" or "Deck 2" buttons to send track to that deck
- **Reorder** – Drag rows to reorganize

## File Structure

```
OtoDecks/
├── Source/
│   ├── Main.cpp                    # Application entry point
│   ├── MainComponent.{h,cpp}       # Top-level UI and audio routing
│   ├── DJAudioPlayer.{h,cpp}       # Audio engine with EQ and transport
│   ├── DeckGUI.{h,cpp}             # Dual-deck UI component
│   ├── PlaylistComponent.{h,cpp}   # Music library UI
│   ├── WaveformDisplay.{h,cpp}     # Waveform renderer
│   └── CueLibrary.{h,cpp}          # Hot cue persistence
├── OtoDecksV2.jucer                # JUCE project file
└── Builds/                         # Generated build files (Xcode, VS, Makefile)
```

## Configuration

Hot cue data and deck state are stored in the user's application data directory:
- **macOS** – `~/Library/Application Support/OtoDecks/`
- **Windows** – `C:\Users\[User]\AppData\Local\OtoDecks\`
- **Linux** – `~/.local/share/OtoDecks/`

Files:
- `cues.xml` – Per-track hot cue positions and labels
- `deckstate.xml` – EQ settings and BPM values

## Performance

- **Real-time Mixing** – Optimized DSP chain for smooth, low-latency performance
- **Waveform Caching** – Thumbnail generation with smart caching to minimize I/O
- **Thread-Safe** – Peak metering and UI updates use atomic operations

## Future Enhancements

- Waveform zoom and panning
- Crossfader with curve customization
- Effects rack (reverb, delay, filter sweeps)
- BPM detection and sync
- MIDI controller support
- Recording and export
- Cue point naming and color customization

## License

This project is provided as-is for educational and personal use.

## Author

Hamza Badat

---

**Built with** [JUCE](https://juce.com) – The most widely used framework for desktop audio software development.
