# Real-Time Audio Processing Engine

## System Architecture (V2.0.0 - Graphical Workspace)

The engine utilizes a decoupled, lock-free architecture separating the Graphical User Interface from the high-priority Audio Processing callback.

### Threading Model

1. **The UI Thread (Main Thread)**
   - Runs at ~60 FPS via GLFW and OpenGL3.
   - Responsible for polling user input, rendering the ImGui/ImNodes Signal Flow Designer, and tracking user-defined audio parameters.
   - Pulls real-time waveform snapshots from the audio thread and utilizes `ImGui::PlotLines` to display live oscilloscopes.
   - Communicates parameter changes to the `AudioStream` via non-blocking, thread-safe direct observer pointers.

2. **The Audio Thread (PortAudio Callback)**
   - Runs at strict real-time priority.
   - Contains NO locks (`std::mutex`), NO blocking I/O, and NO memory allocations (`new`/`malloc`) inside the callback loop.
   - Safely drains the `RingBuffer`, processes audio through the active `DSPChain`, and writes lock-free snapshots to the Visualization Buffer for the UI.

3. **Memory Ownership**
   - The `DSPChain` strictly owns all active DSP modules using `std::unique_ptr`.
   - `AudioStream` retains non-owning raw pointers (observers) to relay live parameter changes from the UI to the DSP modules without risking memory leaks.

---

## Core Modules

### GUI System (Signal Flow Designer)
- **GLFW / OpenGL3:** Hardware-accelerated windowing and rendering context.
- **Dear ImGui:** Immediate-mode GUI paradigm for the main application workspace, sliders, and dockable panels.
- **ImNodes:** Node-based graph editor for visually constructing the DSP pipeline.

### Audio I/O (PortAudioWrapper & AudioStream)
- **PortAudioWrapper:** RAII-compliant manager for initializing and terminating the PortAudio API.
- **AudioStream:** Manages the active default audio stream, handles the real-time callback, bridges UI parameters to the `DSPChain`, and maintains the 512-sample lock-free Visualization Buffer.

### DSP Pipeline (`DSPChain`)
**Design:** Object-oriented chain processing standard 32-bit float audio samples.
- **Gain:** Real-time linear volume scaling.
- **Delay:** Sample-accurate circular buffer for echo and feedback generation.
- **LowPassFilter / HighPassFilter:** First-order IIR filters.
- **Equalizer (3-Band):** Parametric EQ utilizing Robert Bristow-Johnson Biquad formulas (Low Shelf, Peaking, High Shelf).

---

## Utility Modules

- **ConfigManager:** Loads JSON configuration (`nlohmann::json`) and stores initial DSP parameters.
- **Logger:** Static class interface for tracking engine initialization and error states.
- **RingBuffer:** Generic, fixed-capacity, circular lock-free storage between producer and consumer threads.
- **ThreadManager:** Graceful background worker thread creation and lock-free synchronization using `std::atomic<bool>`.