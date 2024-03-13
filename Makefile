CC = g++

CFLAGS = -g -ansi -std=gnu++2a -Wc++11-compat -Wc++14-compat -Wc++17-compat -Wall -Wextra -Weffc++ \
         -Walloca -Warray-bounds -Wcast-align -Wcast-qual -Wchar-subscripts -Wctor-dtor-privacy     \
		 -Wdangling-else -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2 \
		 -Winline -Wlarger-than=8192 -Wmissing-declarations -Wnon-virtual-dtor -Woverloaded-virtual   \
		 -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=2          \
		 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused          \
		 -Wvariadic-macros -Wno-missing-field-initializers -Wnarrowing -Wno-old-style-cast -Wvarargs     \
		 -Walloca -Wdangling-else -fcheck-new -fsized-deallocation -fstack-check -fstrict-overflow        \
		 -fno-omit-frame-pointer


IFLAGS = -I

ASM_PROJ_NAME = asm_
CPU_PROJ_NAME = cpu_
DIS_PROJ_NAME = dis_

ASM_SRC_DIR = asm
CPU_SRC_DIR = cpu
DIS_SRC_DIR = dis
LIB_SRC_DIR = proc_lib
STK_SRC_DIR = proc_lib/stack

ASM_OBJ_DIR = asm/obj
CPU_OBJ_DIR = cpu/obj
DIS_OBJ_DIR = dis/obj
LIB_OBJ_DIR = proc_lib/obj

ASM_SRC = $(wildcard $(ASM_SRC_DIR)/*.cpp)
CPU_SRC = $(wildcard $(CPU_SRC_DIR)/*.cpp)
DIS_SRC = $(wildcard $(DIS_SRC_DIR)/*.cpp)
LIB_SRC = $(wildcard $(LIB_SRC_DIR)/*.cpp)
STK_SRC = $(wildcard $(STK_SRC_DIR)/*.cpp)

ASM_OBJ = $(patsubst $(ASM_SRC_DIR)/%.cpp, $(ASM_OBJ_DIR)/%.o, $(ASM_SRC))
CPU_OBJ = $(patsubst $(CPU_SRC_DIR)/%.cpp, $(CPU_OBJ_DIR)/%.o, $(CPU_SRC))
DIS_OBJ = $(patsubst $(DIS_SRC_DIR)/%.cpp, $(DIS_OBJ_DIR)/%.o, $(DIS_SRC))
LIB_OBJ = $(patsubst $(LIB_SRC_DIR)/%.cpp, $(LIB_OBJ_DIR)/%.o, $(LIB_SRC))
STK_OBJ = $(patsubst $(STK_SRC_DIR)/%.cpp, $(LIB_OBJ_DIR)/%.o, $(STK_SRC))

clean: clean_asm clean_cpu clean_dis

asm: $(ASM_OBJ) $(ASM_OBJ_DIR)/main.o $(LIB_OBJ)
	$(CC) $^ -o $(ASM_PROJ_NAME)

cpu: $(CPU_OBJ) $(CPU_OBJ_DIR)/main.o $(LIB_OBJ) $(STK_OBJ)
	$(CC) $^ -o $(CPU_PROJ_NAME)

dis: $(DIS_OBJ) $(DIS_OBJ_DIR)/main.o $(LIB_OBJ)
	$(CC) $^ -o $(DIS_PROJ_NAME)


$(LIB_OBJ_DIR)/%.o : $(LIB_SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(IFLAGS) $(CFLAGS) -c $^ -o $@

$(LIB_OBJ_DIR)/%.o : $(STK_SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(IFLAGS) $(CFLAGS) -c $^ -o $@

$(ASM_OBJ_DIR)/%.o : $(ASM_SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(IFLAGS) $(CFLAGS) -c $^ -o $@

$(CPU_OBJ_DIR)/%.o : $(CPU_SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(IFLAGS) $(CFLAGS) -c $^ -o $@

$(DIS_OBJ_DIR)/%.o : $(DIS_SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(IFLAGS) $(CFLAGS) -c $^ -o $@

clean_asm:
	rm -rf $(ASM_OBJ_DIR) $(LIB_OBJ_DIR)
	rm $(ASM_PROJ_NAME)

clean_cpu:
	rm -rf $(CPU_OBJ_DIR) $(LIB_OBJ_DIR)
	rm $(CPU_PROJ_NAME)

clean_dis:
	rm -rf $(DIS_OBJ_DIR) $(LIB_OBJ_DIR)
	rm $(DIS_PROJ_NAME)



