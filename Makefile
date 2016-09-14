# António Peixe - 34164
# Sistemas Operativos I
#
# Trabalho prático - Escalonador de Processos
#
#
C_FILES = estados_pcb.c espacos_mem.c 
H_FILES = $(C_FILES:%.c=%.h)
O_FILES = $(C_FILES:%.c=%.o)

MAIN = 34164main.c

C_FLAGS = -g

BINARY = 34164main

all: $(O_FILES) $(MAIN)
	gcc $(C_FLAGS) -o $(BINARY) $(O_FILES) $(MAIN)

%.o: %.c %.h
	gcc $(C_FLAGS) -c $<

clean:
	rm -f $(O_FILES) *~
	rm -f $(BINARY) 34164main.o

run: all
	./$(BINARY)