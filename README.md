# Centaur - Package manager for ChironOS

Super simple package manager for ChironOS, but you could probably use it for other distros too.

## Install

Make sure to install the required dependencies:
- git
- cmake

```bash
# Clone the repository
git clone https://github.com/Chiron8/centaur
cd centaur

# Create build dir
mkdir build
cd build

# Install
cmake ..
make
sudo cmake --install . --prefix /usr
```

