# Development Notes

## Lesson 1 - Working Directory
**Problem:** Configuration file could not be loaded even though it existed.
**Cause:** The application's working directory was the project folder instead of the solution root.
**Solution:** Set the Visual Studio Debugging → Working Directory to `$(SolutionDir)`.

## Lesson 2 - ImGui State and `imgui.ini`
**Problem:** The ImGui window for the Signal Flow Designer was completely collapsed and hidden, even when using `ImGui::SetNextWindowSize`.
**Cause:** ImGui automatically creates an `imgui.ini` file in the working directory to remember window sizes. The first run created a tiny window, which was permanently cached, overriding code-based sizing.
**Solution:** Deleted `imgui.ini` and implemented `ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport())` to force the UI to rigidly dock to the edges of the GLFW window like a professional workspace, ignoring floating window cache files.

## Lesson 3 - DAG Instantiation
**Problem:** Audio output went entirely silent (returned `0.0f`) after switching to the Directed Acyclic Graph, despite UI wires being drawn correctly.
**Cause:** Legacy code relied on `audio_config.json` to instantiate/enable DSP modules. If a module was disabled in JSON, the DAG attempted to route audio through a null module, breaking the signal chain.
**Solution:** In a visual node-based engine, wiring dictates what is active. Refactored `AudioStream` to unconditionally instantiate all available DSP modules so the user can freely bypass or include them using graphical wires.
## Lesson 4 - Runtime DSP DLL Reloading

**Goal:** Rebuild the DSP backend while keeping the graphical host open.

**Problem:** Replacing a DLL that is loaded by a running Windows process can
prevent the original build output from being overwritten.

**Solution:** Keep `CoreDSP.dll` as the Visual Studio build output and load a
separate `CoreDSP_runtime.dll` copy in the host. The reload workflow prepares
a candidate DLL, validates its exported API, backs up the deployed runtime DLL,
and then attempts the switch.

**Important:** API validation does not prove that the replacement can open the
audio device or start successfully. The reload path attempts to restore the
previous backend if switching fails, but this recovery path must be tested.

**Verification:** A normal reload has been tested after rebuilding the DSP DLL
while the host remained open. A temporary Gain implementation change was used
to verify that the runtime copy updated on reload; the original Gain
implementation was subsequently restored.

**Development caution:** Do not delete or reset `imgui.ini` as part of routine
DLL reload testing. Preserve the existing GUI state and unrelated working-tree
changes.
