CFLAGS = -g -ansi -std=gnu++2a -Wc++11-compat -Wc++14-compat -Wc++17-compat -Wall -Wextra -Weffc++ -Walloca -Warray-bounds -Wcast-align -Wcast-qual -Wchar-subscripts -Wctor-dtor-privacy -Wdangling-else -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2 -Winline -Wlarger-than=8192 -Wmissing-declarations -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wnarrowing -Wno-old-style-cast -Wvarargs -Walloca -Wdangling-else -fcheck-new -fsized-deallocation -fstack-check -fstrict-overflow -fno-omit-frame-pointer

all: asm cpu dis

clean: clean_asm clean_cpu clean_dis

asm: asm.o commands.o
	g++ asm.o commands.o -o asm_

cpu: cpu.o commands.o stack.o hash.o
	g++ cpu.o commands.o stack.o hash.o -o cpu_

dis: dis.o commands.o
	g++ dis.o commands.o -o dis_

clean_cpu:
	rm -rf *.o cpu_

clean_asm:
	rm -rf *.o asm_

clean_dis:
	rm -rf *.o dis_

cpu.o: cpu/cpu.cpp
	g++ $(CFLAGS) -c cpu/cpu.cpp

commands.o: commands.cpp
	g++ $(CFLAGS) -c commands.cpp

stack.o: stack/stack.cpp
	g++ $(CFLAGS) -c stack/stack.cpp

hash.o: stack/hash.cpp
	g++ $(CFLAGS) -c stack/hash.cpp

asm.o: asm/asm.cpp
	g++ $(CFLAGS) -c asm/asm.cpp

dis.o: dis/dis.cpp
	g++ $(CFLAGS) -c dis/dis.cpp