# gator:log Data Logger

[![Community Discord](https://img.shields.io/discord/448979533891371018.svg)](https://aka.ms/makecodecommunity)

The gator:log, which is an open source data logger based on the Serial OpenLog, can be used to write data to an SD card using a serial connection

![SparkFun gator:log](https://raw.githubusercontent.com/sparkfun/pxt-gator-log/master/icon.png)  

## ~ hint

To use this package, go to https://makecode.microbit.org, click ``Add package`` and search for **gator-log**. The package is located [here](https://makecode.microbit.org/pkg/sparkfun/pxt-gator-log)

## ~
## Basic usage

```blocks
//Resets the gator:log so we can make sure the SD card is open and ready to write to
gatorLog.begin()
```

Use ``||initialize gator:log||`` to get the log ready to write to.

```blocks
//Creates a directory with the name "Folder1". This does not change to the directory
gatorLog.mkDirectory("Folder1")
```

Use ``||create folder with name"Folder1"||`` to create the folder.

```blocks
//Changes to a directory with the name "Folder1". This does not create the directory
gatorLog.chDirectory("Folder1")
```

Use ``||change to "Folder1" folder||`` to change to the folder.

```blocks
//Creates a file named "myFile.txt", opens the file to be written to
gatorLog.openFile("myFile.txt")
```

Use ``||open file named "myFile.txt"||`` to create and open a file.

```blocks
//Writes the text "Hello there" to the currently open file
gatorLog.writeText("Hello there")
```

Use ``||write "Hello there" to current file||`` to write the string "Hello there" to the current file.

```blocks
//Writes the text "Hello there" to the currently open file with a new line.
gatorLog.writeLine("Hello there")
```

Use ``||write line "Hello there" to current file||`` to write the string "Hello there" to the current file with a new line.

```blocks
//Writes the text "This is text" to the currently open file at position 10.
gatorLog.writeLineOffset("This is text", 10)
```

Use ``||write line "This is text" at position 10||`` to write the string "This is text" to the current file at position 10.

## Example: Basic Functionality Test

```blocks
gatorLog.begin()
gatorLog.mkDirectory("Folder1")
gatorLog.chDirectory("Folder1")
gatorLog.openFile("myFile.txt")
gatorLog.writeLine("Hello there")

```

## Supported targets

* for PXT/microbit

## License

MIT

```package
gatorLog=github:sparkfun/pxt-gator-log
```