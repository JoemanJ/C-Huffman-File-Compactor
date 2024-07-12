all: huffman

huffman: obj/huffman.o obj/main.o obj/bitmap.o
	gcc -g -o huffman obj/huffman.o obj/main.o obj/bitmap.o

obj/huffman.o: huffman.c huffman.h obj/
	gcc -c -g -o obj/huffman.o huffman.c

obj/main.o: main.c obj/
	gcc -c -g -o obj/main.o main.c

obj/bitmap.o: bitmap.c obj/
	gcc -c -g -o obj/bitmap.o bitmap.c

obj/: 
	mkdir obj

test_c: huffman entrada\ teste.txt
	./huffman -c entrada\ teste.txt

debug_c: huffman entrada\ teste.txt
	gdb --args ./huffman -c entrada\ teste.txt

test_d: huffman entrada\ teste.txt.hff
	./huffman -d entrada\ teste.txt.hff

debug_d: huffman entrada\ teste.txt.hff
	gdb --args ./huffman -d entrada\ teste.txt.hff

clean:
	rm -rf ./obj huffman