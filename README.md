# FAT32 Disk Image Extraction Tool

This tool provides a command-line interface (CLI) for extracting data from a 
disk image, included corrupted drives. It allows the extraction of files from the disk image, which
can be particularly useful if you are unable to open the disk image with standard tools or if you need to extract specific files from an embedded FAT32 file system.

Implemented as per the original white paper Microsoft did for the FAT32 specification http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc

License: MIT

## Get Started

1. Compile the executable by running `make` in your terminal at the root dir.
2. From the root dir, run `bin/fat32 PATH_TO_YOUR_DISKIMAGE`.

    For example, you could run `bin/fat32 sample-diskimage/diskimage` to test the program on the sample disk image: 


## Usage

When you execute the program, your shell will enter a loop where you can input various commands. The list of available commands is provided below:

Using these commands, you can navigate the corrupted disk image and extract any necessary files.

### Commands

- **INFO**: Display detailed information about the FAT32 file system.
- **DIR**: List the contents of the current directory.
- **CD**: Change the current directory to a specified folder.
- **GET**: Download and extract a file from the disk image to the current working directory.
