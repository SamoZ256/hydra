# Hydra

Hydra is an experimental Nintendo Switch emulator for macOS.

## Status

The emulator is still in very early stages. A few homebrew apps work perfectly, and some official games get in-game with various degrees of playability.

![Showcase](img/showcase.png)

Only the NRO, NSO and NCA formats are supported. You can extract an NSP file into NCA with [this tool](https://github.com/SamoZ256/switch-extract-macos).

In order to run official games, you will need to download a set of patches to prevent crashes. You can get the patches together with a guide on how to install them [here](https://github.com/SamoZ256/hydra-patches).

## Usage

### Dependencies

You can install Hydra dependencies with a package manager of your choice, like `brew`.

```sh
brew install fmt
```

### Building

First, clone the repository and update submodules.

```sh
git clone https://github.com/SamoZ256/hydra.git
cd hydra
git submodule update --init --recursive
```

Now configure CMake and build with Ninja.

```sh
cmake . -B build -G Ninja -DMACOS_BUNDLE=ON
ninja -C build
```

If you want to use the SwiftUI frontend instead of SDL3, you can use the `-DFRONTEND=SwiftUI` option.

### Running

If you built a macOS bundle, you will find a macOS app at `build/bin/Hydra.app`. Otherwise, you can run the emulator with the following command:

```sh
build/bin/hydra
```

For SDL3, you can drag and drop a ROM into the window or provide path to the ROM as an argument when launching the emulator.

### Configuring

You can find a config file at `/Users/USER/Library/Application Support/Hydra/config.toml` after launching the emulator at least once.
