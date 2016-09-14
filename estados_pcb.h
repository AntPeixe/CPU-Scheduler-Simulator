// António Peixe - 34164
// Sistemas Operativos I
//
// Trabalho prático - Escalonador de Processos
//
//
typedef struct pcb {
	int id, pc;
	int mem_init, mem_end;   // delimitadores da memoria do pcb
} pcb;

typedef struct node {
  pcb *process;
  int entrada;        // contem tempo em que processo deve entrar no estado NEW
                      //              OU
                      // flag que determina se node ja modou de estado durante o ciclo
  int *inst;          // array com intrucoes (so usado no BUFFER)
  int n_inst;         // numero de instrucoes no array inst (so usado no BUFFER)
  int block_counter;  // contador para processos no estado BLOCK  
  struct node *next;
} node;

typedef struct estado {
  node *head;
  node *tail;
  int size;
} estado;


estado *estado_novo();
int estado_vazio(estado *e);
int estado_size(estado *e);
void estado_insert_newNode(estado *e, pcb *p, int ent, int *i, int ni, int bc);
void estado_insert_node(estado *e, node *n);
node *estado_pop(estado *e);
int estado_front_entrada(estado *e);
void estado_clean_entrada(estado *e);
int estado_front_blockCount(estado *e);
void estado_block_dec(estado *e);
void estado_print(estado *e);


pcb *pcb_novo(int i, int mem_s, int mem_e);
void pcb_setPC(pcb *p, int i);
void pcb_setMemInit(pcb *p, int m);
void pcb_setMemEnd(pcb *p, int m);
void pcb_print(pcb *p);
