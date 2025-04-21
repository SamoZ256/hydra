# Hydra

Hydra is an experimental Nintendo Switch emulator for macOS.

Use the `dev` branch for the most up-to-date version.

## Usage

### Building

Note: If changing branches, you may need to update submodules with 
```sh
git submodule update --init --recursive
```

You will need to find your codesigning identity in order to build.

```sh
security find-identity -p codesigning
```

Now configure CMake and build with Ninja.

```sh
cmake . -B build -G Ninja  -DCODESIGN_IDENTITY="Apple Development: SOMETHING (XXXXXXXXXX)"
ninja -C build
```

If you want to use the SwiftUI frontend instead of SDL3, you can use the `-DFRONTEND=SwiftUI` option.

### Running

You can run the emulator with the following command:

```sh
src/frontend/hydra
```

If you are using the SDL3 frontend, you also need to provide a path to a ROM:

```sh
src/frontend/hydra /path/to/rom
```
