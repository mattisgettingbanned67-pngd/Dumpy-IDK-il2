# IL2CppOffsetDumper

A stronger CMake-based IL2CPP dumper that builds to a Windows `.exe` and supports drag-and-drop CLI usage.

## What it dumps
- Images/assemblies (grouped output by image)
- Classes (namespaces + type tokens)
- Fields (field tokens)
- Methods (method tokens, method indices, parameter counts, and best-effort RVAs)

## Improvements
- Better image-to-type mapping (`typeStart` + `typeCount` range, not only exact start index).
- x64/x86 PE detection from `GameAssembly.dll` (no hardcoded 64-bit assumption).
- Better heuristic for method-pointer table scanning across `.rdata` and `.data` with scoring against metadata method count.
- Output path support with `--out`.

## Build (Windows)
```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable will be generated at:
`build\Release\IL2CppOffsetDumper.exe`

## Usage
### Drag-and-drop mode
Drag `global-metadata.dat` and `GameAssembly.dll` onto `IL2CppOffsetDumper.exe`.

### Interactive mode
Run the exe directly and paste each path when prompted.

### Direct CLI mode
```bat
IL2CppOffsetDumper.exe "C:\path\global-metadata.dat" "C:\path\GameAssembly.dll"
```

### Set custom output path
```bat
IL2CppOffsetDumper.exe "C:\path\global-metadata.dat" "C:\path\GameAssembly.dll" --out "C:\path\dump.cs"
```

Default output file: `dump.cs` in the current working directory.
