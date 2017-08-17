/* Arduino SdFs Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdFs Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFs Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
\mainpage Arduino %SdFs Library
<CENTER>Copyright &copy; 2012..2017 by William Greiman
</CENTER>

\section Intro Introduction
The Arduino %SdFs Library supports FAT16, FAT32, and exFAT file systems 
on Standard SD, SDHC, and SDXC cards.

The file system classes in %SdFs are SdFat, SdExFat, and SdFs. SdFat supports
FAT16 and FAT32. SdExFat supports exFAT, SdFs supports FAT16, FAT32, and exFAT.

The corresponding file classes are File, ExFile, and FsFile.

Please read documentation under the above classses tab for more information.

A number of example are provided in the %SdFs/examples folder.  These were
developed to test %SdFs and illustrate its use.

\section exfat exFAT Features

exFAT has many features not available in FAT16/FAT32.

Files larger than 4GiB, 64-bit file size and file position.

Free space allocation performance improved by using a free space bitmap.

Removal of the physical "." and ".." directory entries that appear in
FAT16/FAT32 subdirectories.

Better support for large flash pages with boundary alignment offsets
for the FAT table and data region.

exFAT files have two separate 64-bit length fields.  The DataLength
field indicate how much space is allocate to the file. The ValidDataLength
field indicates how much actual data has been written to the file.

An exFAT file can be contiguous with pre-allocate clusters and bypass the
use of the FAT table.  In this case the contiguous flag is set in the
directory entry.  This allows an entire file to be written as one large
multi-block write.

\section SDPath Paths and Working Directories

Relative paths in %SdFs are resolved in a manner similar to Windows.

Each instance of SdFat, SdExFat, and SdFs has a current directory.  
This directory is called the volume working directory, vwd.  
Initially this directory is the root directory for the volume.

The volume working directory is changed by calling the chdir(path).

The call sd.chdir("/2014") will change the volume working directory
for sd to "/2014", assuming "/2014" exists.

Relative paths for member functions are resolved by starting at
the volume working directory.

For example, the call sd.mkdir("April") will create the directory
"/2014/April" assuming the volume working directory is "/2014".

There is current working directory, cwd, that is used to resolve paths
for file.open() calls.

For a single SD card, the current working directory is always the volume
working directory for that card.

For multiple SD cards the current working directory is set to the volume
working directory of a card by calling the chvol() member function.
The chvol() call is like the Windows \<drive letter>: command.

The call sd2.chvol() will set the current working directory to the volume
working directory for sd2.

If the volume working directory for sd2 is "/music" the call

file.open("BigBand.wav", O_READ);

will open "/music/BigBand.wav" on sd2.

\section Install Installation

You must manually install %SdFs by copying the %SdFs folder from the download
package to the Arduino libraries folder in your sketch folder.

It will be necessary to unzip and rename the folder if you download a zip
file from GitHub.

See the Manual installation section of this guide.

http://arduino.cc/en/Guide/Libraries

\section SDconfig SdFs Configuration

Several configuration options may be changed by editing the SdFsConfig.h
file in the %SdFs folder.

Here are a few of the key options.

If the symbol ENABLE_DEDICATED_SPI is nonzero, multi-block SD I/O will
be used for better performance.  The SPI bus may not be shared with
other devices in this mode.

Set USE_STANDARD_SPI_LIBRARY to use the standard Arduino SPI library. 

To enable SD card CRC checking in SPI mode set USE_SD_CRC nonzero.


\section Hardware Hardware Configuration

The hardware interface to the SD card should not use a resistor based level
shifter. Resistor based level shifters results in signal rise times that are
 too slow for many newer SD cards.


\section HowTo How to format SD Cards as FAT Volumes

The best way to restore an SD card's format on a PC or Mac is to use
SDFormatter which can be downloaded from:

http://www.sdcard.org/downloads

A formatter program, SdFormatter.ino, is included in the
%SdFs/examples/SdFormatter directory.  This program attempts to
emulate SD Association's SDFormatter.

SDFormatter aligns flash erase boundaries with file
system structures which reduces write latency and file system overhead.

The PC/Mac SDFormatter does not have an option for FAT type so it may format
very small cards as FAT12.  Use the %SdFs formatter to force FAT16
formatting of small cards.

Do not format the SD card with an OS utility, OS utilities do not format SD
cards in conformance with the SD standard.

You should use a freshly formatted SD card for best performance.  FAT
file systems become slower if many files have been created and deleted.
This is because the directory entry for a deleted file is marked as deleted,
but is not deleted.  When a new file is created, these entries must be scanned
before creating the file.  Also files can become
fragmented which causes reads and writes to be slower.

\section ExampleFiles Examples

A number of examples are provided in the SdFs/examples folder.

To access these examples from the Arduino development environment
go to:  %File -> Examples -> %SdFs -> \<program Name\>

Compile, upload to your Arduino and click on Serial Monitor to run
the example.

Here is a list:

AvrAdcLogger - Fast AVR ADC logger using Timer/ADC interrupts.

BackwardCompatibility - Demonstrate SD.h compatibility with %SdFs.h.

bench - A read/write benchmark.

DirectoryFunctions - Use of chdir(), ls(), mkdir(), and rmdir().

%ExFatFormatter - Produces optimal exFAT format for smaller SD cards.

ExFatLogger - A data-logger optimized for exFAT features.

ExFatUnicodeTest - Test program for Unicode file names.

OpenNext - Open all files in the root dir and print their filename.

ReadCsvFil - Function to read a CSV text file one field at a time.

RtcTimestampTest - Demonstration of timestamps with RTClib.

SdErrorCodes - Produce a list of error codes.

SdFormatter - This program will format an SD, SDHC, or SDXC card.

SdInfo - Initialize an SD card and analyze its structure for trouble shooting.

TeensyRtcTimestamp - %File timestamps for Teensy3.

TeensySdioDemo - Demo of SDIO and SPI modes for the Teensy 3.5/3.6 built-in SD.
 */
