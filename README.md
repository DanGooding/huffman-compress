# Huffman Compress
A little file compression program, that uses huffman coding.

High frequency bytes in the input are represented by short bit sequences, whilst longer codes are used for rarer bytes.
This type of compression is good for text, but bad at images (it may make a jpeg larger!)

## Usage
    $ ./bin/huffman -c <original_file> <compressed_dest>
    $ ./bin/huffman -d <compressed_file> <decompressed_dest>

## Performance

Reduces the first 10^8 bytes of English wikipedia to 64% of its original size, 
which is [not great](http://mattmahoney.net/dc/text.html). 
This takes 1.1s to compress and 2.3s to decompress on my machine.

