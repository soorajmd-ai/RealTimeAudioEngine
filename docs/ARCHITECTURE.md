# Real-Time Audio Processing Engine

## Current Architecture

main.cpp
    │
    ├── Logger
    │
    └── ConfigManager
            │
            └── nlohmann::json

---

## Modules

### Logger

Status: ✅ Implemented

Responsibilities:

- Print informational messages
- Print warning messages
- Print error messages
- Print debug messages

Current Design:

- Static class interface
- Namespace: AudioEngine
- Console output only

Future Improvements:

- [ ] Log levels
- [ ] Timestamp support
- [ ] File logging
- [ ] Thread-safe logging
- [ ] Colored console output
- [ ] Real-time safe asynchronous logging

### ConfigManager

Status: ✅ Basic Implementation Complete

Current Features:

- Loads JSON configuration
- Stores application settings
- Provides getter functions

Next Improvements:

- Default values
- Validation
- Exception handling
- Save configuration

### Module Dependency

main.cpp
    ↓
ConfigManager
    ↓
nlohmann::json

Design Principle:

Higher-level modules should depend on interfaces, not implementation details.

## RingBuffer

Purpose:
- Temporary storage between producer and consumer threads.

Template Type:
- Generic (RingBuffer<T>)

Internal State:
- std::vector<T> buffer
- head index
- tail index
- size
- capacity

RingBuffer Design

- Generic template class
- Header-only implementation
- Fixed-capacity storage
- Pre-allocated memory
- STL-style interface

### RingBuffer Operations

Implemented:
- push()
- pop()
- size()
- capacity()
- empty()
- full()

Characteristics:
- FIFO
- Fixed capacity
- Circular indexing using modulo (%)
- No dynamic memory allocation after construction

### RingBuffer API

Current Functions

- push()
- pop()
- peek()
- clear()
- size()
- capacity()
- available()
- freeSpace()
- empty()
- full()

## ThreadManager

Current Features:
- Thread creation
- Worker thread
- Graceful shutdown
- RAII cleanup
- Thread joining

### ThreadManager Improvements

- Replaced bool with std::atomic<bool>
- Eliminated data race on thread state
- Prepared for lock-free synchronization

# Real-Time Audio Processing Engine

## System Architecture (V2.0.0 - Graphical Workspace)

The engine utilizes a decoupled, lock-free architecture separating the Graphical User Interface from the high-priority Audio Processing callback.

### Threading Model

1. **The UI Thread (Main Thread)**
   - Runs at ~60 FPS via GLFW and OpenGL3.
   - Responsible for polling user input, rendering the ImGui/ImNodes Signal Flow Designer, and tracking user-defined audio parameters.
   - Communicates parameter changes to the `AudioStream` via non-blocking, thread-safe atomic swaps or direct observer pointers.

2. **The Audio Thread (PortAudio Callback)**
   - Runs at strict real-time priority.
   - Contains NO locks (`std::mutex`), NO blocking I/O, and NO memory allocations (`new`/`malloc`) inside the callback loop.
   - Safely drains the `RingBuffer` and processes audio through the active `DSPChain`.

3. **Memory Ownership**
   - The `DSPChain` strictly owns all active DSP modules using `std::unique_ptr`.
   - `AudioStream` retains non-owning raw pointers (observers) to relay live parameter changes from the UI to the DSP modules without risking memory leaks or dangling pointers.

---

## Core Modules

### GUI System (Signal Flow Designer)
**Status:** ✅ Implemented
- **GLFW / OpenGL3:** Hardware-accelerated windowing and rendering context.
- **Dear ImGui (Docking Branch):** Immediate-mode GUI paradigm for the main application workspace, sliders, and dockable panels.
- **ImNodes:** Node-based graph editor for visually constructing and routing the DSP pipeline.

### Audio I/O (PortAudioWrapper & AudioStream)
**Status:** ✅ Implemented
- **PortAudioWrapper:** RAII-compliant manager for initializing and terminating the PortAudio API.
- **AudioStream:** Manages the active default audio stream, handles the real-time C-style callback, and bridges UI parameters to the `DSPChain`.

### DSP Pipeline (`DSPChain`)
**Status:** ✅ Implemented
**Design:** Object-oriented chain processing standard 32-bit float audio samples.
- **Gain:** Real-time linear volume scaling.
- **Delay:** Sample-accurate circular buffer for echo and feedback generation.
- **LowPassFilter / HighPassFilter:** First-order IIR filters.
- **Equalizer (3-Band):** Parametric EQ utilizing Robert Bristow-Johnson Biquad formulas (Low Shelf, Peaking, High Shelf) for frequency manipulation.

---

## Utility Modules

### ConfigManager
**Status:** ✅ Basic Implementation Complete
- Loads JSON configuration (`nlohmann::json`)
- Stores application settings and initial DSP parameters.

### Logger
**Status:** ✅ Implemented
- Static class interface inside the `AudioEngine` namespace for tracking engine initialization, module loading, and error states.

### RingBuffer
**Status:** ✅ Implemented
- **Purpose:** Temporary, lock-free storage between producer and consumer threads.
- **Characteristics:** FIFO, fixed-capacity, circular indexing modulo (%), zero dynamic memory allocation after construction.
- **Operations:** `push()`, `pop()`, `size()`, `capacity()`, `empty()`, `full()`.

### ThreadManager
**Status:** ✅ Implemented
- **Features:** Graceful background worker thread creation, RAII cleanup, and lock-free synchronization using `std::atomic<bool>`.