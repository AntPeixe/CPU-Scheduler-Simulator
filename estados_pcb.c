// António Peixe - 34164
// Sistemas Operativos I
//
// Trabalho prático - Escalonador de Processos
//
//
#include <stdio.h>
#include <stdlib.h>
#include "estados_pcb.h"

// cria e retorna um novo estado
estado *estado_novo()
{
  estado *e = (estado *) malloc(sizeof(estado));
  
  e->head = e->tail = NULL;
  e->size = 0;
  return e;
}

// verifica se o estado esta vazio
int estado_vazio(estado *e)
{
  return e->head==NULL;
}

// retorna o tamanho do estado (quantidade de nodes)
int estado_size(estado *e)
{
  return e->size;
}

// cria um node e insere-o no estado
void estado_insert_newNode(estado *e, pcb *p, int ent, int *i, int ni, int bc)
{
  node *n = (node *) malloc(sizeof(node));
  
  n->process = p;
  n->entrada = ent;
  n->inst = i;
  n->n_inst = ni;
  n->block_counter = bc;
  n->next = NULL;
  e->size++;
  
  if (estado_vazio(e)) {
    e->head = e->tail = n;
  }
  else {
    e->tail->next = n;
    e->tail = n;
  }
}

// insere um node no estado
void estado_insert_node(estado *e, node *n)
{
  if (estado_vazio(e)) {
    e->head = e->tail = n;
  }
  else {
    e->tail->next = n;
    e->tail = n;
  }
  e->size++;
}

// retira e retorna o node da frente do estado caso nao esteja vazio
node *estado_pop(estado *e)
{
  if (!estado_vazio(e)) {
    node *head = e->head;
    
    e->head = head->next;
    
    e->size--;

    return head;
  }
  else {
    return NULL;
  }
}

// retorna o valor da variavel entrada do node da frente do estado
int estado_front_entrada(estado *e)
{
  if (!estado_vazio(e)) {
    return e->head->entrada;
  }
  else {
    return -1;
  }
}

// faz set para 0 da variavel entrada de todos os nodes do estado
void estado_clean_entrada(estado *e)
{
  if (!estado_vazio(e)) {
    node *n = e->head;
    while (n) {
      n->entrada = 0;
      n = n->next;
    }
  }
}

// retorna o valor da variavel blockCount do node da frente do estado
int estado_front_blockCount(estado *e)
{
  if (!estado_vazio(e)) {
    return e->head->block_counter;
  }
  else {
    return -1;
  }
}

// decrementa o valor da variavel blockCount de todos os nodes do estado
void estado_block_dec(estado *e)
{
  if (!estado_vazio(e)) {
    node *n = e->head;
    n->block_counter--;
    while (n->next) {
      n = n->next;
      n->block_counter--;
    }
  }
}

// print do node
void node_print(node *n)
{
  if (n->process){
    pcb_print(n->process);
  }

  int i;
  for(i = 0; i < n->n_inst; i++){
    printf("%d ", n->inst[i]);
  }
  printf("\n");
}

// print do estado
void estado_print(estado *e)
{

  if (estado_vazio(e)) {
    printf("Estado Vazio!!!\n\n");
    return;
  }

  node *n = e->head;  
  for (; n; n = n->next) {
    node_print(n);
  }

}


/*-----------------------------------------*/
/*-----------------------------------------*/
/*-----------------------------------------*/

// cria e retorna um novo pcb
pcb *pcb_novo(int i, int mem_s, int mem_e)
{
  pcb *p = (pcb *) malloc(sizeof(pcb));
  
  p->id = i;
  p->pc = 0;
  p->mem_init = mem_s;
  p->mem_end = mem_e;
  return p;
}

// faz set do pc do pcb
void pcb_setPC(pcb *p, int i)
{
  p->pc = i;
}
// faz set do memInit do pcb
void pcb_setMemInit(pcb *p, int m)
{
  p->mem_init = m;
}
// faz set do memEnd do pcb
void pcb_setMemEnd(pcb *p, int m)
{
  p->mem_end = m;
}
// print do pcb
void pcb_print(pcb *p)
{
  printf("process id: %d\n", p->id);
  printf("process pc: %d\n", p->pc);
  printf("process mem start: %d\n", p->mem_init);
  printf("process mem end: %d\n", p->mem_end);
}