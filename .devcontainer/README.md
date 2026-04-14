# ITGmania Development Container

This directory contains configuration files for a development container based on Fedora 43, which provides a consistent development environment for building and testing ITGmania.

## Requirements

- [Docker](https://www.docker.com/get-started)
- [Visual Studio Code](https://code.visualstudio.com/download)
- [VS Code Remote - Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## Usage

1. Clone the ITGmania repository:
   ```bash
   git clone https://github.com/itgmania/itgmania.git
   cd itgmania
   ```

2. Fetch vendored sources (required before CMake will configure). FFmpeg and other dependencies live in `extern/` as Git submodules:
   ```bash
   git submodule update --init --recursive
   ```

3. Open the project in VS Code:
   ```bash
   code .
   ```

4. When prompted to "Reopen in Container", click "Reopen in Container". Alternatively, you can press F1, type "Remote-Containers: Reopen in Container" and press Enter.

5. VS Code will build the container and open the project inside it. This might take a few minutes the first time.

## Building ITGmania inside the container

Use a **separate build directory per configuration** if you switch between Debug and Release (CMake single-configuration generators keep one `CMAKE_BUILD_TYPE` per build tree).

**Do not use the Ninja generator** for this project: the bundled FFmpeg build is wired for Makefiles-style generators only.

### Debug (typical for development)

```bash
cmake -S . -B Build -DCMAKE_BUILD_TYPE=Debug -DWITH_MINIMAID=OFF
cmake --build Build -j $(nproc)
```

On Linux the executable is written to the **repository root** as `itgmania-debug`:

```bash
./itgmania-debug
```

### Release (optimized build)

```bash
cmake -S . -B Build-Release -DCMAKE_BUILD_TYPE=Release -DWITH_MINIMAID=OFF
cmake --build Build-Release -j $(nproc)
```

On Linux the Release binary is named `itgmania` (also placed in the repository root):

```bash
./itgmania
```

### Executable names (Linux)

| Configuration   | Binary name      |
|----------------|------------------|
| Debug          | `itgmania-debug` |
| Release        | `itgmania`       |
| MinSizeRel     | `itgmania-min-size` |
| RelWithDebInfo | `itgmania-release-symbols` |

## Portable mode (`Portable.ini`)

Portable mode controls **where user-writable data** (Songs, Save, Cache, Logs, Themes, etc.) is stored.

- **Without** `Portable.ini` on Linux, those directories are created under `~/.itgmania/` while read-only game data still comes from the install or source tree.
- **With** `Portable.ini`, the game uses folders **next to the executable** for that writable content (a self-contained directory you can copy or move).

To enable it, create an **empty** file named `Portable.ini` in the **same directory as the game executable**:

- **Building from source on Linux:** the binary is emitted at the repo root, so place `Portable.ini` there next to `itgmania` or `itgmania-debug`.
- **Installed or packaged layout:** put `Portable.ini` in the same directory as the `itgmania` binary (for `cmake --install`, that is the `itgmania/` folder under your install prefix).

The file can be empty; the game only checks for its presence.

## Packaging a release

The project uses **CPack**. On Linux the generator is **TGZ** (see `CMake/CPackSetup.cmake`). Windows builds use NSIS; macOS uses DragNDrop (`.dmg`).

### Suggested Linux flow (inside the container)

1. Configure and build **Release** (use a clean or Release-only build directory).
2. Optional: set **`WITH_FULL_RELEASE=ON`** so the archive version string uses the plain semver from CMake instead of the `-BETA-git-<hash>` style.
3. Produce the archive from the build tree:

```bash
cmake -S . -B Build-Release -DCMAKE_BUILD_TYPE=Release -DWITH_MINIMAID=OFF -DWITH_FULL_RELEASE=ON
cmake --build Build-Release -j $(nproc)
cmake --build Build-Release --target package
```

Or, equivalently:

```bash
cd Build-Release && cpack -G TGZ
```

The `.tgz` appears under the build directory. Unless **`WITH_CLUB_FANTASTIC=ON`**, the filename includes `-no-songs` (bundled Club Fantastic packs are optional and off by default).

### Manual install layout (without CPack)

To inspect or roll your own archive:

```bash
cmake --install Build-Release --prefix /tmp/itgmania-install
```

On Linux this installs under `PREFIX/itgmania/` (executable, data, desktop file, etc.). Add `Portable.ini` in that folder if you want a portable distribution.

## Customization

You can customize the dev container by modifying the following files:

- `.devcontainer/devcontainer.json`: Configure VS Code settings, extensions, and container settings
- `.devcontainer/Dockerfile`: Modify the container image, add dependencies, or customize the environment

## Troubleshooting

- If CMake errors that **`extern/ffmpeg`** (or another `extern/` path) is missing or empty, run `git submodule update --init --recursive` from the repository root.
- If the build fails due to missing dependencies, you can add them to the Dockerfile and rebuild the container.
- For issues with the dev container itself, try rebuilding the container by pressing F1 and selecting "Remote-Containers: Rebuild Container".
