# AJA Modules for Nodos

![build-badge](https://github.com/nodos-dev/aja/actions/workflows/release.yml/badge.svg)

This folder contains the Nodos modules for AJA SDI I/O boards.

## Build Instructions
1. Download latest Nodos release from [nodos.dev](https://nodos.dev)
2. Clone the repository under Nodos workspace Module directory
```bash
git clone https://github.com/nodos-dev/aja.git --recurse-submodules Module/aja-modules
```
3. Generate project files from workspace root directory using CMake:
```bash
cmake -S ./Toolchain/CMake -B Build
```
4. Build the project:
```bash
cmake --build Build
```

