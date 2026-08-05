# Real-Time Audio Processing Engine

## Current Phase

Phase 1 - Project Foundation

---

## Completed

- [x] Git repository created
- [x] GitHub repository connected
- [x] Professional folder structure
- [x] Visual Studio Solution created
- [x] Initial C++ project created
- [x] First successful build
- [x] Git ignore configured
- [x] Logger module implemented
- [x] AudioEngine namespace added

---

## Next Milestone

- Config Manager
- Project Architecture
- JSON Configuration
- PortAudio Integration

---

## Long-Term Roadmap

- ✅ Logger
- Config Manager
- Ring Buffer
- Audio Engine
- Thread Manager
- PortAudio Integration
- DSP Base Class
- Gain Effect
- Low Pass Filter
- Equalizer
- Reverb
- Runtime Parameter Control
- Testing
- Performance Optimization

---

## Current Version

**Version:** `0.1.0`

### Features

- Logger module
- Static logging interface
- AudioEngine namespace

---

## Current Status

**Build Status:** ✅ Passing

**Git Branch:** `feature/project-configuration`

**Last Completed Module:** Logger

## Current Phase

Phase 2 - Configuration System ✅ Complete

## Completed

- JSON configuration loading
- ConfigManager module
- External configuration file
- Working directory configuration

## Phase 3 - Ring Buffer

### RingBuffer Progress

Completed:
- Generic template implementation
- FIFO operations
- Fixed-capacity storage
- Circular indexing
- Initial functional testing

Next:
- Verify wrap-around behavior
- Add peek()
- Add available space calculation
- Transition to lock-free implementation

## Version 0.2.0

### Completed

- Generic RingBuffer<T>
- push()
- pop()
- clear()
- size()
- capacity()
- empty()
- full()
- Wrap-around functionality verified

## Version 0.3.0

### Completed

- ThreadManager
- Basic thread lifecycle
- RAII cleanup

## Version 0.4.0

### Completed

- AudioEngine core class
- Centralized application lifecycle
- Module integration
  - ConfigManager
  - ThreadManager

## Version 0.5.0

### Completed

- PortAudio integrated
- Third-party library built from source using CMake
- AudioEngine now initializes and terminates PortAudio
- Successfully linked external C library with Modern C++ project

## Version 0.6.0

### Completed

- Integrated PortAudio
- Enumerated available audio devices
- AudioEngine can detect microphones and speakers
- Successfully integrated a third-party C audio library

## Version 0.7.0

### Completed

- Added AudioStream abstraction
- AudioEngine now controls AudioStream lifecycle
- Implemented Open/Start/Stop/Close interfaces
- Prepared architecture for real-time audio streaming

## Version 0.8.0

### Completed

- Opened a real PortAudio stream
- Implemented audio callback
- Real-time microphone-to-speaker pass-through
- Verified live audio processing

## Version 0.9.0

### Completed

- Opened a real PortAudio stream
- Implemented a real-time audio callback
- Achieved live microphone-to-speaker pass-through
- Integrated RingBuffer<float> into the real-time audio pipeline
- Verified live audio samples are flowing through the custom RingBuffer
- Established the foundation for the DSP processing pipeline

### Current Audio Pipeline

Microphone
↓
PortAudio Callback
↓
RingBuffer<float>
↓
Speaker

### Next Milestone

- Implement Gain DSP module
- Process audio samples inside the callback
- Build a modular DSP processing chain

## Version 1.0.0

### Completed

- Added DSPModule base interface
- Implemented DSPChain for modular audio processing
- Added Gain DSP module
- Added LowPassFilter DSP module
- Integrated multiple DSP modules into the real-time audio callback
- Verified real-time DSP processing on live microphone audio

### Next Milestone

- Refactor AudioStream to own the DSP pipeline
- Remove global audio-processing objects
- Use PortAudio userData for callback context

## Version 1.1.0

### Completed

- Refactored `AudioStream` to own the DSP pipeline
- Removed global DSP processing objects
- Migrated audio callback processing into `AudioStream`
- Added configurable DSP parameters through `audio_config.json`
- Added Gain configuration via `ConfigManager`
- Added Low-Pass Filter configuration via `ConfigManager`
- Connected `AudioEngine` with `AudioStream` configuration
- Verified runtime DSP configuration using JSON
- Verified real-time microphone processing with configurable Gain and Low-Pass Filter

### Next Milestone

- Introduce `AudioBuffer` abstraction for block-based audio processing
- Refactor DSP modules to process audio buffers instead of single samples
- Support runtime DSP parameter updates without restarting

## Version 1.2.0

### Completed

- Added Delay DSP module
- Implemented circular buffer–based delay processing
- Added configurable delay length
- Added feedback for repeated echoes
- Integrated Delay into the DSP chain
- Verified real-time echo effect using live microphone input

### Next Milestone

- Implement High-Pass Filter
- Introduce additional DSP modules
- Build a configurable Equalizer