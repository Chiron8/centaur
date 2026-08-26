# Centaur - Package Manager for ChironOS

![centaur demo gif](https://github.com/Chiron8/centaur/blob/gif/centaur.gif)

## What is Centaur?

Centaur is a compile based package manager designed for UNIX based operating systems and is the default for ChironOS. Instead of downloading a pre-built binary file, centaur downloads the source code and compiles it on-device.

Centaur can install, uninstall, update, sync scripts and automatically remove unused dependencies.

## How it works

Centaur uses a special script file called a `.centaur` file. A `.centaur file` is just a sequence of bash commands to install a package. Inspired by portage's ebuilds, these files turn centaur into a compile-based package manager. I've designed the syntax of these files to be as easy to create and use as possible and what better way of doing this than parsing bash (which so many people know already).

To install a package, the parser first checks for any dependencies that need to be installed first by parsing the meta section of the `.centaur` file. It this parses the install block and runs the commands to install the package. After installation, the package is added to the database alongside it's dependencies and if it was pulled in by a parent.

To uninstall, centaur checks for any dependency conflicts before parsing the uninstall section of the `.centaur` file.

For more information about how the other functions work, [Read the wiki](https://github.com/Chiron8/centaur/wiki) or look at the code yourself!

## How to install
To use centaur you need the following dependencies:

- git
- cmake
- make
- wget (most packages use this to download their tarballs)

To install:
```bash
# Clone the repo
git clone https://github.com/Chiron8/centaur
cd centaur

# Create build directory
mkdir build
cd build

# Install
cmake ..
make
sudo cmake --install .
```

## Usage: [Read the wiki](https://github.com/Chiron8/centaur/wiki/3.-Usage)
## Checklist
- [x] Install function
- [x] Uninstall function
- [x] Sync function
- [x] Update function
- [x] List function
- [x] License function
- [ ] Support installing specific versions
- [x] Make output look pretty
- [ ] Port all LFS required packages to .centaur files
- [ ] Port BLFS packages to .centaur files

> [!WARNING]
> A package manager with no bugs is a boring package manager

## Credits:

Christophe Devine for his SHA256 implementation (used in src/SHA256.c and src/SHA256.h)

