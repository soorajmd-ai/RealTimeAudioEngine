# Architecture

## Overview

RealTimeAudioEngine is a modular audio-processing application that separates the
graphical host from the real-time audio engine. The solution contains two
primary projects:

- **CoreDSP** is the audio backend. It provides audio I/O, DSP processing,
  threading, and the public `AudioEngine` API.
- **GTT_Host** is the graphical application. It provides the node-based
  workspace and communicates with the backend through a versioned C-compatible DSP backend API.

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

- Builds as `GTT_Host.exe` and dynamically loads the DSP backend DLL at runtime.
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

## Runtime DSP Plugin Architecture

### Host-to-Backend Boundary

`GTT_Host` communicates with `CoreDSP` through the versioned interface
declared in `include/api/DSPBackendAPI.h`.

The interface uses C-compatible structures and function pointers. The host
does not need to link directly against the implementation of `AudioEngine`.

The API exposes operations for:

- Creating and destroying backend instances.
- Initializing, starting, and stopping audio processing.
- Setting and reading live DSP parameters.
- Updating graph connections.
- Copying visualizer samples.
- Retrieving backend error messages.

The API version and structure size are checked when a DLL is loaded. Required
function pointers are also validated before the host accepts the backend.

### Main Components

| Component | Responsibility |
| --- | --- |
| `GTT_Host/DSPBackendLoader` | Loads and unloads the DLL, validates the API, manages the backend instance, and forwards host operations. |
| `include/api/DSPBackendAPI.h` | Defines the versioned C-compatible plugin boundary. |
| `src/api/DSPBackendAPI.cpp` | Implements the exported API and adapts calls to `AudioEngine`. |
| `CoreDSP` | Contains the audio engine, stream management, DSP modules, and processing graph. |
| `src/main.cpp` | Manages the GUI, graph connections, parameter controls, backend status, and reload workflow. |

### DLL Files and Runtime Reload

The current Windows development workflow uses these files in the host executable
directory:

- `CoreDSP.dll`: the normal Visual Studio build output.
- `CoreDSP_candidate.dll`: a temporary copy used for API validation.
- `CoreDSP_runtime.dll`: the deployed DLL loaded by the host.
- `CoreDSP_previous.dll`: a backup of the previously deployed runtime DLL.

The reload workflow is:

1. Copy the latest build output to the candidate DLL.
2. Load the candidate and validate its API without initializing its audio
   stream.
3. Release the validation instance and module.
4. Back up the current runtime DLL.
5. Stop and unload the active backend.
6. Copy the candidate into the runtime DLL location.
7. Load and initialize the replacement backend.
8. Restore the GUI's live parameter values and graph connections.
9. Start the replacement audio engine.
10. If a switch step fails, attempt to restore and restart the previous backend.

The GUI window and graph editor remain open during this process. The candidate
validation step checks API compatibility and instance creation; it does not
guarantee that audio initialization or startup will succeed.

Rollback is a recovery attempt, not an absolute guarantee. For example, the
previous backend may also fail to initialize if the audio device is unavailable.

### Audio Lifecycle Considerations

`AudioEngine::Initialize()` prepares the audio engine and stream.
`AudioEngine::Run()` starts the audio stream and processing thread.
`AudioEngine::Shutdown()` stops the processing thread and closes the stream.

Shutdown is not merely a pause operation. Restarting a stopped engine requires
initialization again. The reload implementation therefore stops the old engine
before initializing the replacement, avoiding an assumption that both engines
can open the same audio device simultaneously.

### Runtime Reload Verification

The normal reload path has been tested by rebuilding `CoreDSP.dll` while the
host remains open and then reloading the backend from the GUI.

A separate test changed the Gain implementation temporarily, rebuilt the DLL,
and verified that the runtime DLL was updated when reload was requested. The
original Gain implementation was restored afterward.

Failure-path tests for candidate rejection, replacement startup failure, and
successful rollback should be performed separately before treating recovery as
fully verified.
