FROM docker.io/fedora:41

# Install necessary packages for building ITGmania
RUN dnf -y update && dnf -y install --skip-unavailable \
    git \
    cmake \
    gcc \
    gcc-c++ \
    make \
    nasm \
    curl \
    wget \
    ninja-build \
    pkgconfig \
    # System libraries required by ITGmania
    alsa-lib-devel \
    pulseaudio-libs-devel \
    jack-audio-connection-kit-devel \
    mesa-libGL-devel \
    mesa-libGLU-devel \
    libXinerama-devel \
    libXrandr-devel \
    libXtst-devel \
    libva-devel \
    openssl-devel \
    systemd-devel \
    libusb-devel \
    gtk3-devel \
    # Development tools
    gdb \
    valgrind \
    perf \
    strace \
    vim \
    nano \
    diffutils \
    patch \
    # Extra utilities
    sudo \
    which \
    findutils \
    procps-ng \
    net-tools \
    # Install ffmpeg dependencies
    yasm \
    SDL2-devel \
    && dnf clean all

# Create a developer user with sudo access
RUN useradd -m -s /bin/bash developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/developer

# Set up working directory
WORKDIR /workspace

# Switch to the developer user
USER developer

# Configure git
RUN git config --global --add safe.directory /workspace

CMD ["/bin/bash"] 