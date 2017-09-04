### Warning: This is an early beta version of the SdFs library.

The SdFs library is in early development and features may change. 
It will clearly have bugs. I am posting this version to get comments.  

The SdFs library supports FAT16/FAT32 and exFAT SD cards. It is mostly
backward compatible with SdFat on FAT16/FAT32 cards.

exFAT has many features not available in FAT16/FAT32.

I have been using Arduino 1.8.3 for development. I have done minimal
testing with an Arduino Uno, Due, Zero, Teensy 3.6 and eBay
STM32F103C boards. 

Please try the examples.  Start with SdInfo, bench, and ExFatLogger.

To use SdFs, unzip the download file and place the SdFs folder
into the libraries sub-folder in your main sketch folder.

For more information see the Manual installation section of this guide:

http://arduino.cc/en/Guide/Libraries 

A number of configuration options can be set by editing SdFatConfig.h
define macros.  See the html documentation for details

Please read the html documentation for this library.  Start with the 
Main Page.  Next go to the Classes tab and read the documentation for
the classes SdFat, SdExFat, SdFs, File, ExFile, FsFile and other 
classes as you encounter them in examples.
 
Please continue by reading the html documentation in the SdFs/doc folder.

