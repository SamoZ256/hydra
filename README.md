# Hydra

Hydra is an experimental Nintendo Switch emulator for macOS.

Use the `dev` branch for the most up-to-date version.

## Building

First, you need to find your codesigning identity.

```sh
security find-identity -p codesigning
```

Now configure CMake and build with Make.

```sh
mkdir build
cd build
cmake .. -DCODESIGN_IDENTITY="Apple Development: SOMETHING (XXXXXXXXXX)"
make -j4
```
