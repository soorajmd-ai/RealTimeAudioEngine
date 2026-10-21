# Architecture

## Overview

RealTimeAudioEngine is a modular audio-processing application that separates the
graphical host from the real-time audio engine. The solution contains two
primary projects:

- **CoreDSP** is the audio backend. It provides audio I/O, DSP processing,
  threading, and the public `AudioEngine` API.
- **GTT_Host** is the graphical application. It provides the node-based
  workspace and communicates with the backend through the `AudioEngine` API.

The design keeps user interaction and rendering off the high-priority audio
callback while allowing the host to configure and observe audio processing.

## Project Boundaries

### CoreDSP

- Builds as `CoreDSP.dll` and `CoreDSP.lib`.
- Wraps PortAudio for low-latency audio I/O.
- Owns and processes the active DSP graph.
- Exposes its public API through the `COREDSP_API` macro, defined in
  `include/api/CoreDSP_API.h`.
- Uses `AudioEngine` as the public gateway for controlling the DSP backend.

### GTT_Host

- Builds as `GTT_Host.exe` and dynamically links to `CoreDSP.dll`.
- Creates the application window and graphics context with GLFW and OpenGL3.
- Uses Dear ImGui for the workspace and controls, and ImNodes for editing the
  DSP graph.
- Passes user actions, such as parameter changes and node connections, to the
  backend through the public API.

## Runtime Architecture

### Threading Model

1. **UI thread (main thread)**
   - Runs the GLFW/OpenGL3 application and renders the ImGui/ImNodes workspace.
   - Polls user input and tracks user-defined audio parameters.
   - Reads waveform snapshots from the audio thread and displays them with
     `ImGui::PlotLines`.
   - Sends parameter changes to `AudioStream` through non-blocking,
     thread-safe observer pointers.

2. **Audio thread (PortAudio callback)**
   - Runs at real-time priority.
   - Drains the `RingBuffer`, processes audio through the active `DSPChain`, and
     publishes visualization snapshots for the UI.
   - Performs no locking (`std::mutex`), blocking I/O, or dynamic memory
     allocation in the callback.

### Audio Graph and Processing

The DSP pipeline is a directed acyclic graph (DAG) built from the connections
in the visual editor. Kahn's algorithm determines a safe processing order from
the graph's wiring. Recursive cycles are dropped to prevent feedback loops
from reaching the output.

The `DSPChain` owns active DSP modules with `std::unique_ptr`. `AudioStream`
keeps non-owning pointers to modules so UI parameter changes can be relayed
without transferring ownership.

The processing modules include:

- **Gain** for linear volume scaling.
- **Delay** for sample-accurate echo and feedback.
- **Low-pass and high-pass filters** implemented as first-order IIR filters.
- **Three-band equalizer** using Robert Bristow-Johnson biquad formulas for
  low-shelf, peaking, and high-shelf filters.

### Core Components

| Component | Responsibility |
| --- | --- |
| `AudioEngine` | Public API for controlling the backend and DSP graph. |
| `AudioStream` | Manages the audio stream, callback, parameter bridge, and 512-sample visualization buffer. |
| `PortAudioWrapper` | Initializes and terminates PortAudio using RAII. |
| `DSPChain` | Manages DSP modules and graph processing order. |
| `RingBuffer` | Fixed-capacity lock-free storage for communication between producer and consumer threads. |
| `ConfigManager` | Loads JSON configuration and initial DSP parameters. |
| `Logger` | Provides a static interface for initialization and error logging. |
| `ThreadManager` | Manages background worker threads and atomic synchronization. |

## Dependencies and Build Environment

- **PortAudio** — cross-platform real-time audio I/O.
- **GLFW** — window creation and OpenGL context management.
- **OpenGL3** — graphics rendering.
- **Dear ImGui** — immediate-mode user interface.
- **ImNodes** — node editor for Dear ImGui.
- **nlohmann/json** — JSON configuration parsing.
- **Visual Studio 2026** — primary build environment, targeting MSVC x64.

## Repository Layout

```text
RealTimeAudioEngine/
├── AudioEngine/             # Additional Visual Studio project files
├── CoreDSP/                 # DSP backend Visual Studio project
├── GTT_Host/                # Graphical host Visual Studio project
├── config/                  # Runtime configuration
├── docs/                    # Project documentation
├── include/
│   ├── api/                 # Export macros and public API declarations
│   ├── buffer/              # Ring buffer declarations
│   ├── core/                # Audio engine declarations
│   ├── dsp/                 # DSP module and chain declarations
│   ├── io/                  # Audio I/O declarations
│   └── thread/              # Thread manager declarations
├── src/
│   ├── buffer/              # Ring buffer implementation
│   ├── code/                # Audio engine implementation
│   ├── dsp/                 # DSP implementations
│   ├── io/                  # Audio I/O implementations
│   └── thread/              # Thread manager implementation
├── tests/                   # Tests
├── third_party/             # Third-party dependencies
└── RealTimeAudioEngine.slnx # Visual Studio solution
```

## Build and Run

The Visual Studio projects use solution-relative paths for include and library
settings.

1. Open `RealTimeAudioEngine.slnx` in Visual Studio.
2. Select the **Debug** or **Release** configuration and the **x64** platform.
3. Build the solution. The project dependency order builds `CoreDSP` before
   `GTT_Host`, producing the library and DLL used by the host.
4. Set `GTT_Host` as the startup project.
5. Run the host to open the graphical workspace and initialize the audio
   stream.

## Contributor Notes

- **DLL interface warning C4251:** This warning is intentionally suppressed in
  `AudioEngine.h`. Private members such as `m_threadManager` and `m_portAudio`
  are not part of the host-facing API; the host uses public methods instead.
- **Adding a DSP module:** Add its declaration under `include/dsp/` and its
  implementation under `src/dsp/`. The module should inherit from
  `DSPModule`. Individual modules do not need `COREDSP_API` unless the host
  should instantiate them directly instead of going through `AudioEngine`.
