// António Peixe - 34164
// Sistemas Operativos I
//
// Trabalho prático - Escalonador de Processos
//
//
typedef struct fragment {
  int inicio;
  int fim;
  int tamanho;
  int delete_flag;
  struct fragment *next;
} fragment;

typedef struct memSpace {
  fragment *head;
  fragment *tail;
  int size;
} memSpace;


memSpace *memSpace_novo(int memsize);
int memSpace_vazio(memSpace *ms);
void memSpace_print(memSpace *ms);
void memSpace_insert(memSpace *ms, int ini, int f, int t);
fragment *memSpace_getBest(memSpace *ms, int process_size);
fragment *memSpace_getWorst(memSpace *ms, int process_size);

void fragment_recalc_tamanho(fragment *frag);