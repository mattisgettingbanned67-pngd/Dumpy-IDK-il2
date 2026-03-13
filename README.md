# IL2CppOffsetDumper

A CMake-based command-line IL2CPP metadata dumper designed to compile into a Windows `.exe`.

## Features
- Drag-and-drop friendly input handling (works via command-line args when files are dropped onto the `.exe`).
- Reads `global-metadata.dat` and `GameAssembly.dll`.
- Dumps classes, methods, tokens, method indices, and best-effort RVAs into `dump.cs`.
- Heuristic method-pointer-table scanning in `.rdata` to recover RVAs when possible.

## Build (Windows)
```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable will be at:
`build\Release\IL2CppOffsetDumper.exe`

## Usage
### Drag-and-drop mode
Drag `global-metadata.dat` and `GameAssembly.dll` onto `IL2CppOffsetDumper.exe`.

### Interactive mode
Run the exe directly and paste each path when prompted.

### Direct CLI mode
```bat
IL2CppOffsetDumper.exe "C:\path\to\global-metadata.dat" "C:\path\to\GameAssembly.dll"
```

Output file: `dump.cs` in the working directory.
