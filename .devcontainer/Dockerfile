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
    curl-devel \
    wget \
    ninja-build \
    pkgconfig \
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
    libusb1-devel \
    libusb-compat-0.1-devel \
    gtest-devel \
    nlohmann-json-devel \
    protobuf-compiler \
    protobuf-devel \
    gtk3-devel \
    libpng-devel \
    libjpeg-devel \
    zlib-devel \
    libogg-devel \
    libvorbis-devel \
    grpc-devel \
    zlib-ng-compat-static \
    grpc-devel \
    grpc-plugins \
    libmad-devel \
    bzip2-devel \
    pcre-devel \
    gdb \
    valgrind \
    perf \
    strace \
    vim \
    nano \
    diffutils \
    patch \
    sudo \
    which \
    findutils \
    procps-ng \
    net-tools \
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