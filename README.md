This is an implementation of a file compactor using the Huffman algorythm using the C language.
This was an assignment for the data structures subject.

## Usage
First compile the source using make. You can change to your preferred compiler edditing the makefile.
```make```
This will generate the "huffman" binary in the current folder

To compress a binary file:
```./huffman -c [file_name]```
This will generate the compacted file "[file_name].hff"

To decompress a compressed file created using this program:
```./huffman -d [compacted_file_name]```
This will generate the original uncompressed file, with 4 less characters in the name (intended to remove the ".hff" extension)
