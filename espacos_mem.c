// António Peixe - 34164
// Sistemas Operativos I
//
// Trabalho prático - Escalonador de Processos
//
//
#include <stdio.h>
#include <stdlib.h>
#include "espacos_mem.h"

// cria e retorna um novo memSpace
memSpace *memSpace_novo(int memsize)
{
  memSpace *ms = (memSpace *) malloc(sizeof(memSpace));
  
  ms->head = ms->tail = NULL;
  ms->size = 0;
  memSpace_insert(ms, 0, memsize-1, memsize);
  return ms;
}

// verifica se o memSpace esta vazio
int memSpace_vazio(memSpace *ms)
{
  return ms->head==NULL;
}

// imprime os espacos de memoria
void memSpace_print(memSpace *ms)
{
  fragment *a = ms->head;
  while (a) {
    printf("%d %d | ", a->inicio, a->fim);
    a = a->next;
  }
  printf("\n");
}

// apaga todos os fragmentos com flag a 1
void memSpace_deleteFlagged(memSpace *ms)
{
  fragment *previous = NULL;
  fragment *actual = ms->head;

  while(actual) {
    // caso o actual seja a cabeca da lista
    if(actual == ms->head) {
      // caso seja para apagar
      if (actual->delete_flag) {
        ms->head = actual->next;
        free(actual);
        ms->size--;
        actual = ms->head;
      }
      else {
        previous = actual;
        actual = actual->next;
      }
    }
    // se nao for a cabeca
    else {
      // se for para apagar
      if (actual->delete_flag) {
        previous->next = actual->next;
        free(actual);
        ms->size--;
        actual = previous->next;
      }
      else {
        previous = actual;
        actual = actual->next;
      }
    }
  }

  // colocar a cauda da lista no sitio correcto
  if (previous) {
    ms->tail = previous;
  }
  else {    // caso nao haja previous significa que o memSpace ficou vazio
    ms->tail = NULL;
  }
}

// verifica se ha fragmentos contiguos para serem compactados
void memSpace_check(memSpace *ms)
{
  fragment *actual = ms->head;
  fragment *toComp = actual->next;
  
  // enquanto nao estivermos no fim executa
  while (actual->next) {
    // caso o actual seja para apagar avanca o actual
    if (actual->delete_flag) {
      actual = actual->next;
      toComp = actual->next;
    }
    else {
      // caso o toComp nao seja para apagar
      if (!toComp->delete_flag){
        // se o fim do toComp for contiguo com o inicio do actual
        if (actual->inicio == toComp->fim + 1) {
          // modificar o actual e set flag do toComp para ser apagado
          actual->inicio = toComp->inicio;
          actual->tamanho = actual->tamanho + toComp->tamanho;
          toComp->delete_flag = 1;
          // toComp novamente apos o actual
          toComp = actual->next;
        }
        // se o inicio do toComp for contiguo com o fim do actual
        else if (actual->fim == toComp->inicio - 1) {
          // modificar o actual e set flag do toComp para ser apagado
          actual->fim = toComp->fim;
          actual->tamanho = actual->tamanho + toComp->tamanho;
          toComp->delete_flag = 1;
          // toComp novamente apos o actual
          toComp = actual->next;
        }
        else {
          // se nao for contiguo avancar toComp (caso possivel)
          if (toComp->next) {toComp = toComp->next;}
          else {    // caso contrario avancar o actual
            actual = actual->next;
            toComp = actual->next;
          }
        }
      }
      else { 
        // sendo o toComp para eliminar, avacar (caso possivel)
        if (toComp->next) {toComp = toComp->next;}
        else {    // caso contrario avancar o actual
          actual = actual->next;
          toComp = actual->next;
        }
      }
    }
  }

  // funcao para remover todos os fragmentos com a flag activa
  memSpace_deleteFlagged(ms);

}

// insere um novo fragmento no memSpace
void memSpace_insert(memSpace *ms, int ini, int f, int t)
{
  fragment *frag = (fragment *) malloc(sizeof(fragment));

  frag->inicio = ini;
  frag->fim = f;
  frag->tamanho = t;
  frag->delete_flag = 0;
  frag->next = NULL;
  ms->size++;
  
  if (memSpace_vazio(ms)) {
    ms->head = ms->tail = frag;
  }
  else {
    ms->tail->next = frag;
    ms->tail = frag;
  }

  memSpace_check(ms);
}

// retorna o menor fragmento onde cabe o processo
fragment *memSpace_getBest(memSpace *ms, int process_size)
{
  fragment *out = NULL;
  fragment *actual = ms->head;

  if (!memSpace_vazio(ms)) {

    while (actual) {
      if (actual->tamanho > process_size) {
        if (out) {
          if (actual->tamanho < out->tamanho) {out = actual;}
        }
        else {
          out = actual;
        }
      }
      actual = actual->next;
    }
  }

  return out;

}

// retorna o maior fragmento onde cabe o processo
fragment *memSpace_getWorst(memSpace *ms, int process_size)
{
  fragment *out = NULL;
  fragment *actual = ms->head;

  if (!memSpace_vazio(ms)) {

    while (actual) {
      if (actual->tamanho > process_size) {
        if (out) {
          if (actual->tamanho > out->tamanho) {out = actual;}
        }
        else {
          out = actual;
        }
      }
      actual = actual->next;
    }
  }

  return out;
  
}

// calcula o tamanho do fragmento, usado se se modificar limites
void fragment_recalc_tamanho(fragment *frag)
{
  frag->tamanho = frag->fim - frag->inicio + 1;
}