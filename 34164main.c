// António Peixe - 34164
// Sistemas Operativos I
//
// Trabalho prático - Escalonador de Processos
//
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "estados_pcb.h"	// estrutura dos estados do escalonador e dos pcb
#include "espacos_mem.h"	// estrutura que gere o espaco livre em memoria

// variáveis globais
#define QUANTUM 4
#define READY_MAX_SIZE 4
#define MEM_SIZE 300
#define MEM_MANAGEMENT 1	// 1 = Best Fit --- 0 = Worst Fit
#define OUTPUT 1 			// 1 = Normal --- 0 = Debug
#define DEBUG_QUANT 10		// numero de ciclos a executar em modo debug

// assinatura de funcoes existentes
// funcao para ler o ficheiro para um bufer
void read_input(estado *buffer);

// funcao para imprimir a memoria bem como os 5 estados do escalonador
void print_output(int *mem, int n_pcb, estado *new, estado *ready, estado *run, 
	estado *block, estado *end);

// funcoes para manipulacao da memoria "fisica"
void memory_clean(int *mem);
void memory_clean_portion(int *mem, int ini, int end);
void memory_insert(int *mem, int *inst, int quant, int ini);
void memory_print(int *mem, int n_pcb, estado *new, estado *rea, estado *run, estado *bl);

// funcoes de transferencia de processoes de um estado para outro
void trans_buffer_new(estado *buf, estado *new, int *mem, memSpace *spc, int contPCB);
void trans_new_ready(estado *new, estado *rea);
void trans_ready_run(estado *rea, estado *run);
void trans_run_ready(estado *run, estado *rea);
void trans_run_blocked(estado *run, estado *bl);
void trans_run_end(estado *run, estado *e, int *mem, memSpace *spc);
void trans_blocked_ready(estado *bl, estado *rea);

// funcao para executar a instrucao - retorna a instrucao executada
int executar_inst(int *mem, pcb *process);

// funcao para executar o fork de um processo
void fork_process(int *mem, pcb *process, memSpace *spc, estado *new, int contPCB);


// MAIN
int main ()
{
	//inicializacao de variaveis
	int contador_tempo = 0;
	int contador_quantum = 0;
	int contador_pcb = 0;
	int toPrint = 0;
	estado *buffer, *new, *ready, *run, *block, *end;
	int *memoria;
	memSpace *espacos_memoria;

	// criar buffer de processos e ler do ficheiro
	buffer = estado_novo();
	read_input(buffer);

	// criar a memoria e a estrutura que controla o espaco livre
	memoria = malloc(MEM_SIZE * sizeof(int));
	memory_clean(memoria);
	espacos_memoria = memSpace_novo(MEM_SIZE);

	// criar todos os estados do processador
	new = estado_novo();
	ready = estado_novo();
	run = estado_novo();
	block = estado_novo();
	end = estado_novo();

	// corre enquanto houver algum processo por terminar
	while (!(estado_vazio(buffer) && estado_vazio(new) && estado_vazio(ready)
		&& estado_vazio(run) && estado_vazio(block)))
	{
		
		// se buffer tiver um processo e for tempo transferir para new
		if (!estado_vazio(buffer) && estado_front_entrada(buffer) <= contador_tempo) {
			trans_buffer_new(buffer,new, memoria, espacos_memoria, ++contador_pcb);
			toPrint = 1;
		}

		// enquanto houver processos em block que devem sair, transferir para new
		while (!estado_front_blockCount(block) && !estado_front_entrada(block)) {
			trans_blocked_ready(block, ready);
		}

		// enquanto houver processos em new que devem sair, transferir para ready
		while (!estado_vazio(new) && !estado_front_entrada(new) && estado_size(ready) <= READY_MAX_SIZE) {
			trans_new_ready(new, ready);
		}

		// se run estiver vazio e houver um processo em ready que pode sair, transferir
		if (estado_vazio(run) && !estado_vazio(ready) && !estado_front_entrada(ready)) {
			trans_ready_run(ready, run);
		}

		// decrementar os contadores dos processos bloqueados
		estado_block_dec(block);

		// se run nao estiver vazio correr instrucao
		if (!estado_vazio(run)) {

			int i = executar_inst(memoria, run->head->process);

			switch (i) {
				case 7:			// passar pra blocked
					trans_run_blocked(run, block);
					contador_quantum = -1;	
					break;
				case 6:			// fazer fork do processo que esta em run
					fork_process(memoria, run->head->process, espacos_memoria, new, ++contador_pcb);
					toPrint = 1;
					break;
				case 9:			// terminar o processo
					trans_run_end(run, end, memoria, espacos_memoria);
					toPrint = 1;
					contador_quantum = -1;
					break;
			}
			contador_quantum++;
			if (contador_quantum == QUANTUM) {
				contador_quantum = 0;
				trans_run_ready(run, ready);
			}
		}

		if (toPrint && OUTPUT) {
			print_output(memoria, contador_pcb, new, ready, run, block, end);
		}

		if (!OUTPUT && contador_tempo < DEBUG_QUANT) {
			print_output(memoria, contador_pcb, new, ready, run, block, end);
		}

		// limpa o indicar de processo ja transferido no ciclo
		estado_clean_entrada(new);
		estado_clean_entrada(ready);
		estado_clean_entrada(run);
		estado_clean_entrada(block);
		toPrint = 0;

		contador_tempo += 1;
	}
	return 0;
}

/*--------------------------------------------------------*/
/*--------------------------------------------------------*/

// funcao read_input recebe um argumento do tipo estado que sera o buffer de
// processos do sistema (pré estado NEW)
void read_input(estado *buffer)
{
	FILE *fil;
	char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fil = fopen("input.xpto", "r");
    if (!fil) {
		printf("Não foi possível abrir o ficheiro");
		exit(0);
	}


	int i;
	char *instrucoes;
	while ((read = getline(&line, &len, fil)) != -1) {

		// procurar o 1o espaco para saber qual o tempo de chegada do processo
		// i sera ultima posicao antes do primeiro espaco
		i = 0;
		while(1) {
			if (line[i] == 32) {
				break;
			}
			i++;
		}

		// guarda o tempo de chegada
		char tempo[i];
		strncpy(tempo, line, i);

		// instrucoes -> string apenas com instrucoes
		// calcular quantas instrucoes tera o processo
		instrucoes = &line[i+1];
		int n_inst;
		if((int)strlen(instrucoes)%3) {
			n_inst = (int)strlen(instrucoes)/3 + 1;
		}
		else {
			n_inst = (int)strlen(instrucoes)/3;
		}

		// alocar um array de inteiros para as instrucoes de acordo c/ o calculado
		int *array_inst = malloc(n_inst * sizeof(int));
		
		// introduzir instrucoes no array
		int j;			// posicao na string
		int k = 0;		// posicao no array
		for(j = 0; j < (int)strlen(instrucoes); j = j+3) {
			array_inst[k] = (instrucoes[j]-48)*10 + (instrucoes[j+1]-48);
			k++;

		}

		// introduzir as informacoes do processo no node do buffer
		estado_insert_newNode(buffer, NULL, atoi(tempo), array_inst, k, -1);
	}

	fclose(fil);
	free(line);
}

/*--------------------------------------------------------*/
/*--------------------------------------------------------*/

// funcao para imprimir a memoria bem como os 5 estados do escalonador
void print_output(int *mem, int n_pcb, estado *new, estado *ready, estado *run, 
	estado *block, estado *end)
{
	printf("MEMÓRIA\n");
	memory_print(mem, n_pcb, new, ready, run, block);
	printf("\nESTADO NEW\n");
	estado_print(new);
	printf("ESTADO READY\n");
	estado_print(ready);
	printf("ESTADO RUN\n");
	estado_print(run);
	printf("ESTADO BLOCK\n");
	estado_print(block);
	printf("ESTADO FINAL\n");
	estado_print(end);
	printf("\n");
}

/*--------------------------------------------------------*/
/*--------------------------------------------------------*/

// "limpa" totalmente a memoria
void memory_clean(int *mem)
{
	int i = 0;
	for (; i < MEM_SIZE; i++) {
		*(mem+i) = -9;
	}
}

// "limpa" um zona da memoria
void memory_clean_portion(int *mem, int ini, int end)
{
	while (ini <= end) {
		*(mem+ini) = -9;
		ini++;
	}
}

// insere em memoria a instrucoes
void memory_insert(int *mem, int *inst, int quant, int ini)
{
	int i = 0;
	for (i; i<quant; i++) {
		*(mem+10+ini+i) = *(inst+i);
	}
}

// metodo para imprimir a memoria
void memory_print(int *mem, int n_pcb, estado *new, estado *rea, estado *run, estado *bl)
{
	// criar array com posicoes de inicio de processos da memoria
	int array_ini_process[n_pcb];
	int i = 0;
	for (i; i<n_pcb;i++) {
		array_ini_process[i] = -1;
	}

	int cont = 0;
	node *n;
	// inserir no array o inicio da memoria dos processos no estado new
	n = new->head;
	while (n) {
		if (n->process) {
			array_ini_process[cont] = n->process->mem_init;
			cont++;
		}
		n = n->next;
	}

	// inserir no array o inicio da memoria dos processos no estado ready
	n = rea->head;
	while (n) {
		if (n->process) {
			array_ini_process[cont] = n->process->mem_init;
			cont++;
		}
		n = n->next;
	}

	// inserir no array o inicio da memoria dos processos no estado run
	n = run->head;
	while (n) {
		if (n->process) {
			array_ini_process[cont] = n->process->mem_init;
			cont++;
		}
		n = n->next;
	}

	// inserir no array o inicio da memoria dos processos no estado block
	n = bl->head;
	while (n) {
		if (n->process) {
			array_ini_process[cont] = n->process->mem_init;
			cont++;
		}
		n = n->next;
	}

	// correr a memoria para imprimir
	cont = -1;
	i = 0;
	for (i; i<MEM_SIZE; i++) {

		// verificar se a posicao actual e um inicio de processo
		int j = 0;
		for (j; j<n_pcb; j++) {
			if (array_ini_process[j] == i) {
				cont = 0;
				break;
			}
		}

		// cont = 0 -> inicio de um processo -> imprimir marcador
		if (!cont) {
			printf("|{ ");
		}

		if (*(mem+i) == -9) {
			printf("_ ");
		}
		else {
			printf("%d ",*(mem+i));
		}

		// cont = 9 -> fim de um processo -> imprimir marcador
		if (cont == 9) {
			printf("} ");
		}

		if (cont != -1) {
			cont++;
		}
	}
	printf("\n");
}

/*--------------------------------------------------------*/
/*--------------------------------------------------------*/

// transfere um processo do buffer para o estado new e aloca-o na memoria
void trans_buffer_new(estado *buf, estado *new, int *mem, memSpace *spc, int contPCB)
{
	// retira o processo do buffer
	node *n = estado_pop(buf);

	// escolhe a zona da memoria para colocar o processo
	fragment *frag_memoria;
	if (MEM_MANAGEMENT) {
		frag_memoria = memSpace_getBest(spc, n->n_inst + 10);
	}
	else {
		frag_memoria = memSpace_getWorst(spc, n->n_inst + 10);
	}

	// limites de memoria para o processo
	int memInit = frag_memoria->inicio;
	int memEnd = memInit + n->n_inst + 9;

	// altera o fragmento usado
	frag_memoria->inicio = memEnd+1;
	fragment_recalc_tamanho(frag_memoria);

	// insere o array das instrucoes na memoria
	memory_insert(mem, n->inst, n->n_inst, memInit);
	
	// criar o pcb do processo
	n->process = pcb_novo(contPCB, memInit, memEnd);

	n->entrada = 1;
	free(n->inst);
	n->inst = NULL;
	n->n_inst = -1;
	n->next = NULL;

	// inserir o processo no estado new
	estado_insert_node(new, n);

}

// transfere um processo do estado new para o estado ready
void trans_new_ready(estado *new, estado *rea)
{
	node *n = estado_pop(new);
	n->next = NULL;
	n->entrada = 1;

	estado_insert_node(rea, n);
}

// transfere um processo do estado ready para o estado run
void trans_ready_run(estado *rea, estado *run)
{
	node *n = estado_pop(rea);
	n->next = NULL;
	n->entrada = 1;

	estado_insert_node(run, n);
}

// transfere um processo do estado run para o estado ready
void trans_run_ready(estado *run, estado *rea)
{
	node *n = estado_pop(run);
	n->next = NULL;
	n->entrada = 1;

	estado_insert_node(rea, n);
}

// transfere um processo do estado run para o estado block
void trans_run_blocked(estado *run, estado *bl)
{
	node *n = estado_pop(run);
	n->next = NULL;
	n->block_counter = 3;
	n->entrada = 1;

	estado_insert_node(bl, n);
}

// transfere um processo do estado run para o estado end e desaloca a memoria
void trans_run_end(estado *run, estado *e, int *mem, memSpace *spc)
{
	node *n = estado_pop(run);
	n->next = NULL;

	int ini = n->process->mem_init;
	int fim = n->process->mem_end;

	// limpar a memoria onde estava o processo
	memory_clean_portion(mem, ini, fim);
	// insere o novo fragmento de memoria livre na estrutura
	memSpace_insert(spc, ini, fim, fim-ini+1);

	estado_insert_node(e, n);
}

// transfere um processo do estado block para o estado ready
void trans_blocked_ready(estado *bl, estado *rea)
{
	node *n = estado_pop(bl);
	n->block_counter = -1;
	n->next = NULL;
	n->entrada = 1;

	estado_insert_node(rea, n);
}

// executa a instrucao do processo
int executar_inst(int *mem, pcb *process)
{
	// inicio do processo na memoria
	int *initMem = mem + process->mem_init;
	// instrucao a ser executada
	int instrucao = *(initMem + process->pc + 10) / 10;
	// variavel ou valor a utilizar
	int varORval = *(initMem + process->pc + 10) % 10;

	switch(instrucao) {
		case 0:
			*(initMem + varORval) = 0;
			break;
		case 1:
			*(initMem + varORval) = *(initMem + varORval) + 1;
			break;
		case 2:
			*(initMem + varORval) = *(initMem + varORval) - 1;
			break;
		case 3:		// caso seja != 0 deve incrementar 2*pc (uma aqui e a normal do fim)
			if (*(initMem + varORval)) {
				process->pc++;
			}
			break;
		case 4:
			process->pc = process->pc - varORval - 1;	// -1 porque pc sempre incrementado no fim
			break;
		case 5:
			process->pc = process->pc + varORval - 1;	// -1 porque pc sempre incrementado no fim
			break;
		case 8:
			*initMem = *(initMem + varORval);
			break;

	}
	process->pc++;
	return instrucao;
}


// executa o fork do processo
void fork_process(int *mem, pcb *process, memSpace *spc, estado *new, int contPCB)
{
	pcb *forked;
	// tamanho = quantidade de instrucoes por correr
	int tamanho = process->mem_end - (process->mem_init + process->pc + 10) + 1;

	// escolhe a zona da memoria para colocar o processo
	fragment *frag_memoria;
	if (MEM_MANAGEMENT) {
		frag_memoria = memSpace_getBest(spc, tamanho + 10);
	}
	else {
		frag_memoria = memSpace_getWorst(spc, tamanho + 10);
	}

	// limites de memoria para o processo
	int memInit = frag_memoria->inicio;
	int memEnd = memInit + tamanho + 9;

	// altera o fragmento usado
	frag_memoria->inicio = memEnd + 1;
	fragment_recalc_tamanho(frag_memoria);

	// introduzir novo processo em new
	forked = pcb_novo(contPCB, memInit, memEnd);
	estado_insert_newNode(new, forked, 1, NULL, -1, -1);

	// copiar variaveis para o novo local
	int i = 0;
	for (i; i<10;i++) {
		*(mem + memInit + i) = *(mem + process->mem_init + i);
	}

	// copiar instrucoes restantes
	i = 0;
	for (i; i<tamanho; i++) {
		*(mem + memInit + 10 + i) = *(mem + process->mem_init + 10 + process->pc + i);
	}
}