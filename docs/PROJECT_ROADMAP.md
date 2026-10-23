# Real-Time Audio Processing Engine

## Current Development Phase: Runtime DSP Plugin Architecture

**Git branch:** `feature/runtime-plugin-loading`

The project is evolving from a standalone real-time audio engine into a
graphical audio-processing application with a dynamically loaded DSP backend.

## Completed Foundations

### Core Audio Engine

- Implemented audio I/O through PortAudio.
- Implemented the `AudioEngine` lifecycle.
- Added configuration loading through `ConfigManager` and JSON.
- Added logging through `Logger`.
- Added background thread management.
- Added a fixed-capacity ring buffer.

### DSP Processing

- Implemented the `DSPModule` abstraction and `DSPChain`.
- Added Gain, Delay, Low-Pass, and High-Pass processing.
- Added a three-band equalizer.
- Implemented graph-based routing using a directed acyclic graph (DAG).
- Connected graphical node links to the backend graph.

### Graphical Host

- Integrated GLFW and OpenGL3.
- Integrated Dear ImGui and ImNodes.
- Added a node-based signal-flow editor.
- Added live DSP parameter controls.
- Added audio visualization using a 512-sample visualization buffer.
- Kept the host GUI separate from the DSP backend implementation.

### Runtime DSP Backend

- Added a versioned C-compatible API in `include/api/DSPBackendAPI.h`.
- Implemented the exported backend API in `src/api/DSPBackendAPI.cpp`.
- Added `DSPBackendLoader` to load and manage the backend DLL.
- Added runtime DLL deployment using a separate runtime copy.
- Added a GUI control to reload the DSP backend without closing the host.
- Added candidate DLL API validation before switching.
- Added a previous-runtime-DLL backup and a rollback attempt when switching
  fails.
- Verified successful runtime replacement after rebuilding the DSP DLL while
  the host is open.

## Current Priorities

### 1. Validate Reload Failure Recovery

- Test rejection of a missing or invalid candidate DLL.
- Test initialization and startup failure handling.
- Verify that the previous backend can be restored and restarted.
- Report recovery failures clearly in the GUI.
- Confirm that graph connections and live parameters remain intact.

### 2. Improve Runtime Reliability

- Review cleanup of temporary candidate and backup DLLs.
- Review audio-thread and UI-thread interactions.
- Verify that visualizer data is copied safely and consistently.
- Check backend lifecycle handling during application shutdown.
- Improve diagnostics for DLL loading and audio-device failures.

### 3. Workspace Persistence

- Save and restore graph nodes and connections.
- Serialize supported DSP parameter values.
- Define a versioned project-file format, preferably JSON.
- Validate project data before applying it to the active graph.

### 4. Expand DSP Capabilities

- Evaluate additional effects, including reverb.
- Consider FFT-based analysis and visualization.
- Measure CPU usage, processing latency, and memory consumption.
- Investigate SIMD optimization where profiling demonstrates a benefit.

### 5. Testing and Documentation

- Add repeatable tests for the DSP API and loader lifecycle.
- Document supported build configurations and runtime dependencies.
- Record known limitations and verified behavior.
- Keep the architecture and development notes aligned with the implementation.

## Longer-Term Direction

- A standalone graphical audio application inspired by professional audio
  tuning and signal-flow tools.
- A modular DSP backend that can be rebuilt and reloaded independently of
  the host.
- Editable and persistent signal-flow graphs.
- More DSP modules, diagnostic tools, and performance measurements.
- A maintainable C++ architecture suitable for further real-time audio work.

## Verification Status

The GUI build and normal runtime reload have been reported as successful.
Candidate validation and rollback code are present, but their failure paths
still require deliberate testing. Planned features should not be considered
implemented until they have been built and verified.
