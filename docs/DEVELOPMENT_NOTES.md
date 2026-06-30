# Development Notes

## Lesson 1 - Working Directory

Problem:
Configuration file could not be loaded even though it existed.

Cause:
The application's working directory was the project folder
(AudioEngine/) instead of the solution root.

Solution:
Set the Visual Studio Debugging → Working Directory to:

$(SolutionDir)

Result:
Relative paths such as

config/audio_config.json

work correctly.