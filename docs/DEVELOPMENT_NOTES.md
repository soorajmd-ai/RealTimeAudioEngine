# Development Notes

## Lesson 1 - Working Directory
**Problem:** Configuration file could not be loaded even though it existed.
**Cause:** The application's working directory was the project folder instead of the solution root.
**Solution:** Set the Visual Studio Debugging → Working Directory to `$(SolutionDir)`.

## Lesson 2 - ImGui State and `imgui.ini`
**Problem:** The ImGui window for the Signal Flow Designer was completely collapsed and hidden, even when using `ImGui::SetNextWindowSize`.
**Cause:** ImGui automatically creates an `imgui.ini` file in the working directory to remember window sizes. The first run created a tiny window, which was permanently cached, overriding code-based sizing.
**Solution:** Deleted `imgui.ini` and implemented `ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport())` to force the UI to rigidly dock to the edges of the GLFW window like a professional workspace, ignoring floating window cache files.