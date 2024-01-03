CFLAGS = -g -ansi -std=gnu++2a -Wc++11-compat -Wc++14-compat -Wc++17-compat -Wall -Wextra -Weffc++ -Walloca -Warray-bounds -Wcast-align -Wcast-qual -Wchar-subscripts -Wctor-dtor-privacy -Wdangling-else -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2 -Winline -Wlarger-than=8192 -Wmissing-declarations -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wnarrowing -Wno-old-style-cast -Wvarargs -Walloca -Wdangling-else -fcheck-new -fsized-deallocation -fstack-check -fstrict-overflow -fno-omit-frame-pointer

all: asm cpu dash

asm: asm.o commands.o
	g++ asm.o commands.o -o asm

cpu: cpu.o commands.o stack.o hash.o
	g++ cpu.o commands.o stack.o hash.o -o cpu

dasm: dasm.o commands.o
	g++ dasm.o commands.o -o dasm

clean_cpu:
	rm -rf *.o cpu

clean_asm:
	rm -rf *.o asm

clean_dasm:
	rm -rf *.o dasm

cpu.o: cpu.cpp
	g++ $(CFLAGS) cpu.cpp

commands.o: commands.cpp
	g++ $(CFLAGS) commands.cpp

stack.o: stack/stack.cpp
	g++ $(CFLAGS) stack/stack.cpp

hash.o: stack/hash.cpp
	g++ $(CFLAGS) stack/hash.cpp

asm.o: asm.cpp
	g++ $(CFLAGS) asm.cpp

dasm.o: dasm.cpp
	g++ $(CFLAGS) dasm.cpp