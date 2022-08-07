# QtSpim to binary
Application which translates the text for a MIPS program from the QtSpim format to binary files, for use in my [MIPS Virtual Machine](https://github.com/Flawww/MIPS-VM).

# How to use
Create a text file and copy-paste the text from QtSpim for the loaded MIPS program. You only need to copy the sections which you actually want, and all sections go in the same text file. Each section must include it's header. The following example is the `.text` section:
```
User Text Segment [00400000]..[00440000]
[00400000] 8fa40000  lw $4, 0($29)            ; lw $a0 0($sp)
...
```
.text and .ktext sections are in the "Text" tab of QtSpim.
.data and .kdata sections are in the "Data" tab of QtSpim.

## Running
* Compile the project with any compiler which supports C++17 or newer.
* Place the QtSpim-format text file in the same directory as the executable

### Windows
* Drag & Drop the text file onto the executable

### Linux
* Run the executable with the path to the text file as the first command line argument.

# Output
The output is one binary file per section, if the specific section exists in the text file. The file name(s) will be the name of the text file, with the name of the section added as a filename extension.

The parser also fixes Spim's relative branch instructions. For some reason, Spim calculates the relative address from `PC` instead of `PC+4`, which is not what MIPS is supposed to do. 

A QtSpim-format text file with sections `.text`, `.data` and `.ktext`, that has the name `exmpl.txt` produces the following binary files:
```
exmpl.text
exmpl.data
exmpl.ktext
```

## Binary file format
The bytecode for the inputted MIPS program is stored as binary files in the files that were produced by the application.

**NOTE:** The first 4 bytes of each binary file is the *(virtual)* address of that section, ie. it tells the MIPS Virtual Machine where to map that section in its virtual memory.