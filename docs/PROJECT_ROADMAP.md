# Real-Time Audio Processing Engine

## Current Phase: Phase 4 - Dynamic Routing (DAG)
**Version:** `2.0.0`
**Build Status:** ✅ Passing
**Git Branch:** `feature/project-configuration`

---

## Completed Milestones

### Phase 1 & 2: Core Engine & Config (V1.0.0)
- ✅ Implemented `Logger`, `ConfigManager` (JSON), and `ThreadManager`.
- ✅ Integrated PortAudio for live microphone-to-speaker pass-through.
- ✅ Built generic, lock-free `RingBuffer<float>`.

### Phase 3: DSP Processing Chain (V1.2.0)
- ✅ Added `DSPModule` base interface and `DSPChain`.
- ✅ Implemented Gain, Delay, LowPass, and HighPass filters.

### Phase 4: Graphical Workspace (V2.0.0)
- ✅ **Memory Architecture Upgrade:** Migrated DSP factory to strict `std::unique_ptr` ownership.
- ✅ **Parametric Equalizer:** Implemented a 3-Band EQ using precise Biquad filter mathematics.
- ✅ **Windowing System:** Integrated GLFW and OpenGL3.
- **Graphical User Interface:** Integrated Dear ImGui (Docking Branch) to build a professional workspace.
- **Signal Flow Designer:** Integrated ImNodes to visually map the DSP processing pipeline.
- **Live Parameter Tuning:** Bridged UI and audio threads for real-time DSP manipulation via sliders.
- **Live Audio Visualization:** Integrated real-time oscilloscopes into the node canvas via lock-free snapshot buffers.

---

## Next Milestone: The Directed Acyclic Graph (DAG)

- **Dynamic Routing:** Upgrade the underlying C++ `DSPChain` from a linear `std::vector` to a Directed Acyclic Graph.
- **Interactive Wires:** Link the ImNodes UI wires directly to the C++ DAG, allowing users to draw, reroute, and delete audio wiring dynamically.

---

## Long-Term Roadmap
- Project Save/Load (Serialize node graph to JSON)
- Reverb / FFT Modules
- Performance Optimization & SIMD Intrinsics