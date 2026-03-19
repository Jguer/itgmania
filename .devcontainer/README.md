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

2. Open the project in VS Code:
   ```bash
   code .
   ```

3. When prompted to "Reopen in Container", click "Reopen in Container". Alternatively, you can press F1, type "Remote-Containers: Reopen in Container" and press Enter.

4. VS Code will build the container and open the project inside it. This might take a few minutes the first time.

## Building ITGmania inside the container

Once inside the container, you can build ITGmania using the standard CMake commands:

```bash
# Configure the build
cmake -S . -B Build -DCMAKE_BUILD_TYPE=Debug -DWITH_MINIMAID=OFF

# Build the project
cmake --build Build -j $(nproc)

# Run ITGmania
./itgmania
```

## Customization

You can customize the dev container by modifying the following files:

- `.devcontainer/devcontainer.json`: Configure VS Code settings, extensions, and container settings
- `.devcontainer/Dockerfile`: Modify the container image, add dependencies, or customize the environment

## Troubleshooting

- If the build fails due to missing dependencies, you can add them to the Dockerfile and rebuild the container.
- For issues with the dev container itself, try rebuilding the container by pressing F1 and selecting "Remote-Containers: Rebuild Container". 