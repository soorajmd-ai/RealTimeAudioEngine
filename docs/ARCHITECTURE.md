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