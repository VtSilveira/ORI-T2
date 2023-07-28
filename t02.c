/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Organização de Recuperação da Informação
 * Prof. Tiago A. Almeida e Prof. Jurandy Almeida
 *
 * Trabalho 02 - Árvores-B
 *
 * RA: 802318
 * Aluno: Vitor Silveira
 * ========================================================================== */
 
/* Bibliotecas */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 
typedef enum {
  false,
  true
} bool;
 
/* Tamanho dos campos dos registros */
/* Campos de tamanho fixo */
#define TAM_DATE 9
#define TAM_DATETIME 13
#define TAM_INT_NUMBER 5
#define TAM_FLOAT_NUMBER 14
#define TAM_ID_CURSO 9
#define TAM_ID_USUARIO 12
#define TAM_TELEFONE 12
#define QTD_MAX_CATEGORIAS 3
 
/* Campos de tamanho variável (tamanho máximo) */
#define TAM_MAX_NOME 45
#define TAM_MAX_TITULO 52
#define TAM_MAX_INSTITUICAO 52
#define TAM_MAX_MINISTRANTE 51
#define TAM_MAX_EMAIL 45
#define TAM_MAX_CATEGORIA 21
 
#define MAX_REGISTROS 1000
#define TAM_REGISTRO_USUARIO (TAM_ID_USUARIO + TAM_MAX_NOME + TAM_MAX_EMAIL + TAM_FLOAT_NUMBER + TAM_TELEFONE)
#define TAM_REGISTRO_CURSO (TAM_ID_CURSO + TAM_MAX_TITULO + TAM_MAX_INSTITUICAO + TAM_MAX_MINISTRANTE + TAM_DATE + TAM_INT_NUMBER + TAM_FLOAT_NUMBER + QTD_MAX_CATEGORIAS * TAM_MAX_CATEGORIA + 1)
#define TAM_REGISTRO_INSCRICAO (TAM_ID_CURSO + TAM_ID_USUARIO + TAM_DATETIME + 1 + TAM_DATETIME - 4)
#define TAM_ARQUIVO_USUARIOS (TAM_REGISTRO_USUARIO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CURSOS (TAM_REGISTRO_CURSO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_INSCRICOES (TAM_REGISTRO_INSCRICAO * MAX_REGISTROS + 1)
 
#define TAM_RRN_REGISTRO 4
#define TAM_CHAVE_USUARIOS_IDX (TAM_ID_USUARIO + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_CURSOS_IDX (TAM_ID_CURSO + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_INSCRICOES_IDX (TAM_ID_USUARIO + TAM_ID_CURSO + TAM_RRN_REGISTRO - 2)
#define TAM_CHAVE_TITULO_IDX (TAM_MAX_TITULO + TAM_ID_CURSO - 2)
#define TAM_CHAVE_DATA_CURSO_USUARIO_IDX (TAM_DATETIME + TAM_ID_CURSO + TAM_ID_USUARIO - 3)
#define TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX (TAM_MAX_CATEGORIA - 1)
#define TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX (TAM_ID_CURSO - 1)
 
#define TAM_ARQUIVO_USUARIOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CURSOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_INSCRICOES_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_TITULO_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_DATA_CURSO_USUARIO_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CATEGORIAS_IDX (1000 * MAX_REGISTROS + 1)
 
/* Mensagens padrões */
#define SUCESSO "OK\n"
#define RRN_NOS "Nos percorridos:"
#define RRN_REGS_PRIMARIOS "Registros primários percorridos:"
#define RRN_REGS_SECUNDARIOS "Registros secundários percorridos:"
#define INDICE_CRIADO "Indice %s criado com sucesso!\n"
#define AVISO_NENHUM_REGISTRO_ENCONTRADO "AVISO: Nenhum registro encontrado\n"
#define ERRO_OPCAO_INVALIDA "ERRO: Opcao invalida\n"
#define ERRO_MEMORIA_INSUFICIENTE "ERRO: Memoria insuficiente\n"
#define ERRO_PK_REPETIDA "ERRO: Ja existe um registro com a chave %s\n"
#define ERRO_REGISTRO_NAO_ENCONTRADO "ERRO: Registro nao encontrado\n"
#define ERRO_SALDO_NAO_SUFICIENTE "ERRO: Saldo insuficiente\n"
#define ERRO_CATEGORIA_REPETIDA "ERRO: O curso %s ja possui a categoria %s\n"
#define ERRO_VALOR_INVALIDO "ERRO: Valor invalido\n"
#define ERRO_ARQUIVO_VAZIO "ERRO: Arquivo vazio\n"
#define ERRO_NAO_IMPLEMENTADO "ERRO: Funcao %s nao implementada\n"
 
/* Registro de Usuario */
typedef struct
{
  char id_usuario[TAM_ID_USUARIO];
  char nome[TAM_MAX_NOME];
  char email[TAM_MAX_EMAIL];
  char telefone[TAM_TELEFONE];
  double saldo;
} Usuario;
 
/* Registro de Curso */
typedef struct
{
  char id_curso[TAM_ID_CURSO];
  char titulo[TAM_MAX_TITULO];
  char instituicao[TAM_MAX_INSTITUICAO];
  char ministrante[TAM_MAX_MINISTRANTE];
  char lancamento[TAM_DATE];
  int carga;
  double valor;
  char categorias[QTD_MAX_CATEGORIAS][TAM_MAX_CATEGORIA];
} Curso;
 
/* Registro de Inscricao */
typedef struct
{
  char id_curso[TAM_ID_CURSO];
  char id_usuario[TAM_ID_USUARIO];
  char data_inscricao[TAM_DATETIME];
  char status;
  char data_atualizacao[TAM_DATETIME];
} Inscricao;
 
/*----- Registros dos índices -----*/
 
/* Struct para índice de lista invertida */
typedef struct
{
  char *chave;
  int proximo_indice;
} inverted_list_node;
 
/* Struct para um nó de Árvore-B */
typedef struct
{
  int this_rrn;
  int qtd_chaves;
  char **chaves;  // ponteiro para o começo do campo de chaves no arquivo de índice respectivo
  bool folha;
  int *filhos;  // vetor de int para o RRN dos nós filhos (DEVE SER DESALOCADO APÓS O USO!!!)
} btree_node;
 
/* Variáveis globais */
/* Arquivos de dados */
char ARQUIVO_USUARIOS[TAM_ARQUIVO_USUARIOS];
char ARQUIVO_CURSOS[TAM_ARQUIVO_CURSOS];
char ARQUIVO_INSCRICOES[TAM_ARQUIVO_INSCRICOES];
 
/* Ordem das Árvores-B */
int btree_order = 3;  // valor padrão
 
/* Índices */
/* Struct para os parâmetros de uma lista invertida */
typedef struct
{
  // Ponteiro para o arquivo de índice secundário
  char *arquivo_secundario;
 
  // Ponteiro para o arquivo de índice primário
  char *arquivo_primario;
 
  // Quantidade de registros de índice secundário
  unsigned qtd_registros_secundario;
 
  // Quantidade de registros de índice primário
  unsigned qtd_registros_primario;
 
  // Tamanho de uma chave secundária nesse índice
  unsigned tam_chave_secundaria;
 
  // Tamanho de uma chave primária nesse índice
  unsigned tam_chave_primaria;
 
  // Função utilizada para comparar as chaves do índice secundário.
  // Igual às funções de comparação do bsearch e qsort.
  int (*compar)(const void *key, const void *elem);
} inverted_list;
 
/* Struct para os parâmetros de uma Árvore-B */
typedef struct
{
  // RRN da raiz
  int rrn_raiz;
 
  // Ponteiro para o arquivo de índice
  char *arquivo;
 
  // Quantidade de nós no arquivo de índice
  unsigned qtd_nos;
 
  // Tamanho de uma chave nesse índice
  unsigned tam_chave;
 
  // Função utilizada para comparar as chaves do índice.
  // Igual às funções de comparação do bsearch e qsort.
  int (*compar)(const void *key, const void *elem);
} btree;
 
typedef struct
{
  char chave_promovida[TAM_CHAVE_TITULO_IDX + 1];  // TAM_CHAVE_TITULO_IDX é a maior chave possível
  int filho_direito;
} promovido_aux;
 
/* Arquivos de índices */
char ARQUIVO_USUARIOS_IDX[TAM_ARQUIVO_USUARIOS_IDX];
char ARQUIVO_CURSOS_IDX[TAM_ARQUIVO_CURSOS_IDX];
char ARQUIVO_INSCRICOES_IDX[TAM_ARQUIVO_INSCRICOES_IDX];
char ARQUIVO_TITULO_IDX[TAM_ARQUIVO_TITULO_IDX];
char ARQUIVO_DATA_CURSO_USUARIO_IDX[TAM_ARQUIVO_DATA_CURSO_USUARIO_IDX];
char ARQUIVO_CATEGORIAS_SECUNDARIO_IDX[TAM_ARQUIVO_CATEGORIAS_IDX];
char ARQUIVO_CATEGORIAS_PRIMARIO_IDX[TAM_ARQUIVO_CATEGORIAS_IDX];
 
/* Comparam a chave (key) com cada elemento do índice (elem).
 * Funções auxiliares para o buscar e inserir chaves em Árvores-B.
 * Note que, desta vez, as funções comparam chaves no formato de strings, não structs.
 * key é a chave do tipo string que está sendo buscada ou inserida. elem é uma chave do tipo string da struct btree_node.
 *
 * Dica: busque sobre as funções strncmp e strnlen, muito provavelmente vai querer utilizá-las no código.
 * */
int order_usuarios_idx(const void *key, const void *elem);
int order_cursos_idx(const void *key, const void *elem);
int order_inscricoes_idx(const void *key, const void *elem);
int order_titulo_idx(const void *key, const void *elem);
int order_data_curso_usuario_idx(const void *key, const void *elem);
int order_categorias_idx(const void *key, const void *elem);
 
btree usuarios_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_USUARIOS_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_USUARIOS_IDX,
    .compar = order_usuarios_idx,
};
 
btree cursos_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_CURSOS_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_CURSOS_IDX,
    .compar = order_cursos_idx,
};
 
btree inscricoes_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_INSCRICOES_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_INSCRICOES_IDX,
    .compar = order_inscricoes_idx,
};
 
btree titulo_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_TITULO_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_TITULO_IDX,
    .compar = order_titulo_idx,
};
 
btree data_curso_usuario_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_DATA_CURSO_USUARIO_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_DATA_CURSO_USUARIO_IDX,
    .compar = order_data_curso_usuario_idx,
};
 
inverted_list categorias_idx = {
    .arquivo_secundario = ARQUIVO_CATEGORIAS_SECUNDARIO_IDX,
    .arquivo_primario = ARQUIVO_CATEGORIAS_PRIMARIO_IDX,
    .qtd_registros_secundario = 0,
    .qtd_registros_primario = 0,
    .tam_chave_secundaria = TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX,
    .tam_chave_primaria = TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX,
    .compar = order_categorias_idx,
};
 
/* Contadores */
unsigned qtd_registros_usuarios = 0;
unsigned qtd_registros_cursos = 0;
unsigned qtd_registros_inscricoes = 0;
 
/* Funções de geração determinística de números pseudo-aleatórios */
uint64_t prng_seed;
 
void prng_srand(uint64_t value) {
  prng_seed = value;
}
 
uint64_t prng_rand() {
  // https://en.wikipedia.org/wiki/Xorshift#xorshift*
  uint64_t x = prng_seed;  // O estado deve ser iniciado com um valor diferente de 0
  x ^= x >> 12;            // a
  x ^= x << 25;            // b
  x ^= x >> 27;            // c
  prng_seed = x;
  return x * UINT64_C(0x2545F4914F6CDD1D);
}
 
/**
 * Gera um <a href="https://en.wikipedia.org/wiki/Universally_unique_identifier">UUID Version-4 Variant-1</a>
 * (<i>string</i> aleatória) de 36 caracteres utilizando o gerador de números pseudo-aleatórios
 * <a href="https://en.wikipedia.org/wiki/Xorshift#xorshift*">xorshift*</a>. O UUID é
 * escrito na <i>string</i> fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char chave_aleatoria[37];<br />
 * new_uuid(chave_aleatoria);<br />
 * printf("chave aleatória: %s&#92;n", chave_aleatoria);<br />
 * </code>
 *
 * @param buffer String de tamanho 37 no qual será escrito
 * o UUID. É terminado pelo caractere <code>\0</code>.
 */
void new_uuid(char buffer[37]) {
  uint64_t r1 = prng_rand();
  uint64_t r2 = prng_rand();
 
  sprintf(buffer, "%08x-%04x-%04lx-%04lx-%012lx", (uint32_t)(r1 >> 32), (uint16_t)(r1 >> 16), 0x4000 | (r1 & 0x0fff), 0x8000 | (r2 & 0x3fff), r2 >> 16);
}
 
/* Funções de manipulação de data */
time_t epoch;
 
#define YEAR0 1900
#define EPOCH_YR 1970
#define SECS_DAY (24L * 60L * 60L)
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)
 
#define TIME_MAX 2147483647L
 
long _dstbias = 0;   // Offset for Daylight Saving Time
long _timezone = 0;  // Difference in seconds between GMT and local time
 
const int _ytab[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
 
struct tm *gmtime_r(const time_t *timer, struct tm *tmbuf) {
  // based on http://www.jbox.dk/sanos/source/lib/time.c.html
  time_t time = *timer;
  unsigned long dayclock, dayno;
  int year = EPOCH_YR;
 
  dayclock = (unsigned long)time % SECS_DAY;
  dayno = (unsigned long)time / SECS_DAY;
 
  tmbuf->tm_sec = dayclock % 60;
  tmbuf->tm_min = (dayclock % 3600) / 60;
  tmbuf->tm_hour = dayclock / 3600;
  tmbuf->tm_wday = (dayno + 4) % 7;  // Day 0 was a thursday
  while (dayno >= (unsigned long)YEARSIZE(year)) {
    dayno -= YEARSIZE(year);
    year++;
  }
  tmbuf->tm_year = year - YEAR0;
  tmbuf->tm_yday = dayno;
  tmbuf->tm_mon = 0;
  while (dayno >= (unsigned long)_ytab[LEAPYEAR(year)][tmbuf->tm_mon]) {
    dayno -= _ytab[LEAPYEAR(year)][tmbuf->tm_mon];
    tmbuf->tm_mon++;
  }
  tmbuf->tm_mday = dayno + 1;
  tmbuf->tm_isdst = 0;
  return tmbuf;
}
 
time_t mktime(struct tm *tmbuf) {
  // based on http://www.jbox.dk/sanos/source/lib/time.c.html
  long day, year;
  int tm_year;
  int yday, month;
  /*unsigned*/ long seconds;
  int overflow;
  long dst;
 
  tmbuf->tm_min += tmbuf->tm_sec / 60;
  tmbuf->tm_sec %= 60;
  if (tmbuf->tm_sec < 0) {
    tmbuf->tm_sec += 60;
    tmbuf->tm_min--;
  }
  tmbuf->tm_hour += tmbuf->tm_min / 60;
  tmbuf->tm_min = tmbuf->tm_min % 60;
  if (tmbuf->tm_min < 0) {
    tmbuf->tm_min += 60;
    tmbuf->tm_hour--;
  }
  day = tmbuf->tm_hour / 24;
  tmbuf->tm_hour = tmbuf->tm_hour % 24;
  if (tmbuf->tm_hour < 0) {
    tmbuf->tm_hour += 24;
    day--;
  }
  tmbuf->tm_year += tmbuf->tm_mon / 12;
  tmbuf->tm_mon %= 12;
  if (tmbuf->tm_mon < 0) {
    tmbuf->tm_mon += 12;
    tmbuf->tm_year--;
  }
  day += (tmbuf->tm_mday - 1);
  while (day < 0) {
    if (--tmbuf->tm_mon < 0) {
      tmbuf->tm_year--;
      tmbuf->tm_mon = 11;
    }
    day += _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
  }
  while (day >= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon]) {
    day -= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
    if (++(tmbuf->tm_mon) == 12) {
      tmbuf->tm_mon = 0;
      tmbuf->tm_year++;
    }
  }
  tmbuf->tm_mday = day + 1;
  year = EPOCH_YR;
  if (tmbuf->tm_year < year - YEAR0)
    return (time_t)-1;
  seconds = 0;
  day = 0;  // Means days since day 0 now
  overflow = 0;
 
  // Assume that when day becomes negative, there will certainly
  // be overflow on seconds.
  // The check for overflow needs not to be done for leapyears
  // divisible by 400.
  // The code only works when year (1970) is not a leapyear.
  tm_year = tmbuf->tm_year + YEAR0;
 
  if (TIME_MAX / 365 < tm_year - year)
    overflow++;
  day = (tm_year - year) * 365;
  if (TIME_MAX - day < (tm_year - year) / 4 + 1)
    overflow++;
  day += (tm_year - year) / 4 + ((tm_year % 4) && tm_year % 4 < year % 4);
  day -= (tm_year - year) / 100 + ((tm_year % 100) && tm_year % 100 < year % 100);
  day += (tm_year - year) / 400 + ((tm_year % 400) && tm_year % 400 < year % 400);
 
  yday = month = 0;
  while (month < tmbuf->tm_mon) {
    yday += _ytab[LEAPYEAR(tm_year)][month];
    month++;
  }
  yday += (tmbuf->tm_mday - 1);
  if (day + yday < 0)
    overflow++;
  day += yday;
 
  tmbuf->tm_yday = yday;
  tmbuf->tm_wday = (day + 4) % 7;  // Day 0 was thursday (4)
 
  seconds = ((tmbuf->tm_hour * 60L) + tmbuf->tm_min) * 60L + tmbuf->tm_sec;
 
  if ((TIME_MAX - seconds) / SECS_DAY < day)
    overflow++;
  seconds += day * SECS_DAY;
 
  // Now adjust according to timezone and daylight saving time
  if (((_timezone > 0) && (TIME_MAX - _timezone < seconds)) ||
      ((_timezone < 0) && (seconds < -_timezone))) {
    overflow++;
  }
  seconds += _timezone;
 
  if (tmbuf->tm_isdst) {
    dst = _dstbias;
  } else {
    dst = 0;
  }
 
  if (dst > seconds)
    overflow++;  // dst is always non-negative
  seconds -= dst;
 
  if (overflow)
    return (time_t)-1;
 
  if ((time_t)seconds != seconds)
    return (time_t)-1;
  return (time_t)seconds;
}
 
bool set_time(char *date) {
  // http://www.cplusplus.com/reference/ctime/mktime/
  struct tm tm_;
 
  if (strlen(date) == TAM_DATETIME - 1 && sscanf(date, "%4d%2d%2d%2d%2d", &tm_.tm_year, &tm_.tm_mon, &tm_.tm_mday, &tm_.tm_hour, &tm_.tm_min) == 5) {
    tm_.tm_year -= 1900;
    tm_.tm_mon -= 1;
    tm_.tm_sec = 0;
    tm_.tm_isdst = -1;
    epoch = mktime(&tm_);
    return true;
  }
  return false;
}
 
void tick_time() {
  epoch += prng_rand() % 864000;  // 10 dias
}
 
/**
 * Escreve a <i>data</i> atual no formato <code>AAAAMMDD</code> em uma <i>string</i>
 * fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char timestamp[TAM_DATE];<br />
 * current_date(timestamp);<br />
 * printf("data atual: %s&#92;n", timestamp);<br />
 * </code>
 *
 * @param buffer String de tamanho <code>TAM_DATE</code> no qual será escrita
 * a <i>timestamp</i>. É terminado pelo caractere <code>\0</code>.
 */
void current_date(char buffer[TAM_DATE]) {
  // http://www.cplusplus.com/reference/ctime/strftime/
  // http://www.cplusplus.com/reference/ctime/gmtime/
  // AAAA MM DD
  // %Y   %m %d
  struct tm tm_;
  if (gmtime_r(&epoch, &tm_) != NULL)
    strftime(buffer, TAM_DATE, "%Y%m%d", &tm_);
}
 
/**
 * Escreve a <i>data</i> e a <i>hora</i> atual no formato <code>AAAAMMDDHHMM</code> em uma <i>string</i>
 * fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char timestamp[TAM_DATETIME];<br />
 * current_datetime(timestamp);<br />
 * printf("data e hora atual: %s&#92;n", timestamp);<br />
 * </code>
 *
 * @param buffer String de tamanho <code>TAM_DATETIME</code> no qual será escrita
 * a <i>timestamp</i>. É terminado pelo caractere <code>\0</code>.
 */
void current_datetime(char buffer[TAM_DATETIME]) {
  // http://www.cplusplus.com/reference/ctime/strftime/
  // http://www.cplusplus.com/reference/ctime/gmtime/
  // AAAA MM DD HH MM
  // %Y   %m %d %H %M
  struct tm tm_;
  if (gmtime_r(&epoch, &tm_) != NULL)
    strftime(buffer, TAM_DATETIME, "%Y%m%d%H%M", &tm_);
}
 
/* Remove comentários (--) e caracteres whitespace do começo e fim de uma string */
void clear_input(char *str) {
  char *ptr = str;
  int len = 0;
 
  for (; ptr[len]; ++len) {
    if (strncmp(&ptr[len], "--", 2) == 0) {
      ptr[len] = '\0';
      break;
    }
  }
 
  while (len - 1 > 0 && isspace(ptr[len - 1]))
    ptr[--len] = '\0';
 
  while (*ptr && isspace(*ptr))
    ++ptr, --len;
 
  memmove(str, ptr, len + 1);
}
 
/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */
 
/* Cria o índice respectivo */
void criar_usuarios_idx();
void criar_cursos_idx();
void criar_inscricoes_idx();
void criar_titulo_idx();
void criar_data_curso_usuario_idx();
void criar_categorias_idx();
 
/* Exibe um registro com base no RRN */
bool exibir_usuario(int rrn);
bool exibir_curso(int rrn);
bool exibir_inscricao(int rrn);
 
/* Exibe um registro com base na chave de um btree_node */
bool exibir_btree_usuario(char *chave);
bool exibir_btree_curso(char *chave);
bool exibir_btree_inscricao(char *chave);
bool exibir_btree_titulo(char *chave);
bool exibir_btree_data_curso_usuario(char *chave);
 
/* Recupera do arquivo o registro com o RRN informado
 * e retorna os dados nas structs Usuario, Curso e Inscricao */
Usuario recuperar_registro_usuario(int rrn);
Curso recuperar_registro_curso(int rrn);
Inscricao recuperar_registro_inscricao(int rrn);
 
/* Escreve em seu respectivo arquivo na posição informada (RRN) */
void escrever_registro_usuario(Usuario u, int rrn);
void escrever_registro_curso(Curso j, int rrn);
void escrever_registro_inscricao(Inscricao c, int rrn);
 
/* Funções principais */
void cadastrar_usuario_menu(char *id_usuario, char *nome, char *email, char *telefone);
void cadastrar_telefone_menu(char *id_usuario, char *telefone);
void remover_usuario_menu(char *id_usuario);
void cadastrar_curso_menu(char *titulo, char *instituicao, char *ministrante, char *lancamento, int carga, double valor);
void adicionar_saldo_menu(char *id_usuario, double valor);
void inscrever_menu(char *id_curso, char *id_usuario);
void cadastrar_categoria_menu(char *titulo, char *categoria);
void atualizar_status_inscricoes_menu(char *id_usuario, char *titulo, char status);
 
/* Busca */
void buscar_usuario_id_menu(char *id_usuario);
void buscar_curso_id_menu(char *id_curso);
void buscar_curso_titulo_menu(char *titulo);
 
/* Listagem */
void listar_usuarios_id_menu();
void listar_cursos_categorias_menu(char *categoria);
void listar_inscricoes_periodo_menu(char *data_inicio, char *data_fim);
 
/* Liberar espaço */
void liberar_espaco_menu();
 
/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu();
void imprimir_arquivo_cursos_menu();
void imprimir_arquivo_inscricoes_menu();
 
/* Imprimir índices primários */
void imprimir_usuarios_idx_menu();
void imprimir_cursos_idx_menu();
void imprimir_inscricoes_idx_menu();
 
/* Imprimir índices secundários */
void imprimir_titulo_idx_menu();
void imprimir_data_curso_usuario_idx_menu();
void imprimir_categorias_secundario_idx_menu();
void imprimir_categorias_primario_idx_menu();
 
/* Funções de manipulação de Lista Invertida */
/**
 * Responsável por inserir duas chaves (chave_secundaria e chave_primaria) em uma Lista Invertida (t).<br />
 * Atualiza os parâmetros dos índices primário e secundário conforme necessário.<br />
 * As chaves a serem inseridas devem estar no formato correto e com tamanho t->tam_chave_primario e t->tam_chave_secundario.<br />
 * O funcionamento deve ser genérico para qualquer Lista Invertida, adaptando-se para os diferentes parâmetros presentes em seus structs.<br />
 *
 * @param chave_secundaria Chave a ser buscada (caso exista) ou inserida (caso não exista) no registro secundário da Lista Invertida.
 * @param chave_primaria Chave a ser inserida no registro primário da Lista Invertida.
 * @param t Ponteiro para a Lista Invertida na qual serão inseridas as chaves.
 */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t);
 
/**
 * Responsável por buscar uma chave no índice secundário de uma Lista invertida (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice inicial das chaves no registro primário.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = inverted_list_secondary_search(NULL, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse no caminho feito para encontrar a chave.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, true, categoria, &categorias_idx);<br />
 * </code>
 *
 * @param result Ponteiro para ser escrito o índice inicial (primeira ocorrência) das chaves do registro primário. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave_secundaria Chave a ser buscada.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t);
 
/**
 * Responsável por percorrer o índice primário de uma Lista invertida (T). O valor de retorno indica a quantidade de chaves encontradas.
 * O ponteiro para o vetor de strings result pode ser fornecido opcionalmente, e será populado com a lista de todas as chaves encontradas.
 * O ponteiro para o inteiro indice_final também pode ser fornecido opcionalmente, e deve conter o índice do último campo da lista encadeada
 * da chave primaria fornecida (isso é útil na inserção de um novo registro).<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. As chaves encontradas deverão ser retornadas e tanto o caminho quanto o indice_final não devem ser informados.<br />
 * ...<br />
 * char chaves[TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX][MAX_REGISTROS];<br />
 * int qtd = inverted_list_primary_search(chaves, false, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse nas chaves encontradas, apenas no indice_final, e o caminho não deve ser informado.<br />
 * ...<br />
 * int indice_final;
 * int qtd = inverted_list_primary_search(NULL, false, indice, &indice_final, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse nas chaves encontradas e no caminho feito.<br />
 * ...<br />
 * char chaves[TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX][MAX_REGISTROS];<br />
 * int qtd = inverted_list_primary_search(chaves, true, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * </code>
 *
 * @param result Ponteiro para serem escritas as chaves encontradas. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param indice Índice do primeiro registro da lista encadeada a ser procurado.
 * @param indice_final Ponteiro para ser escrito o índice do último registro encontrado (cujo campo indice é -1). É ignorado caso NULL.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica a quantidade de chaves encontradas.
 */
int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t);
 
/**
 * Responsável por buscar uma chave (k) dentre os registros secundários de uma Lista Invertida de forma eficiente.<br />
 * O valor de retorno deve indicar se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice no registro secundário da chave encontrada.<br />
 *
 * @param result Ponteiro para ser escrito o índice nos registros secundários da chave encontrada. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Lista Invertida.
 * @param t Ponteiro para o índice da Lista Invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_binary_search(int *result, bool exibir_caminho, char *chave, inverted_list *t);
 
/* Funções de manipulação de Árvores-B */
/**
 * Responsável por inserir uma chave (k) em uma Árvore-B (T). Atualiza os parâmetros da Árvore-B conforme necessário.<br />
 * A chave a ser inserida deve estar no formato correto e com tamanho t->tam_chave.<br />
 * O funcionamento deve ser genérico para qualquer Árvore-B, considerando que os únicos parâmetros que se alteram entre
 * as árvores é o t->tam_chave.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * ...<br />
 * char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];<br />
 * sprintf(usuario_str, "%s%04d", id_usuario, rrn_usuario);<br />
 * btree_insert(usuario_str, &usuarios_idx);<br />
 * ...<br />
 * </code>
 *
 * @param chave Chave a ser inserida na Árvore-B.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 */
void btree_insert(char *chave, btree *t);
 
/**
 * Função auxiliar de inserção de uma chave (k) em uma Árvore-B (T). Atualiza os parâmetros da Árvore-B conforme necessário.<br />
 * Esta é uma função recursiva. Ela recebe o RRN do nó a ser trabalhado sobre.<br />
 *
 * @param chave Chave a ser inserida na Árvore-B.
 * @param rrn RRN do nó no qual deverá ser processado.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 * @return Retorna uma struct do tipo promovido_aux que contém a chave promovida e o RRN do filho direito.
 */
promovido_aux btree_insert_aux(char *chave, int rrn, btree *t);
 
/**
 * Função auxiliar para dividir um nó de uma Árvore-B (T). Atualiza os parâmetros conforme necessário.<br />
 *
 * @param promo Uma struct do tipo promovido_aux que contém a chave a ser inserida e o RRN do seu filho direito.
 * @param node Ponteiro para nó que deve ser dividido.
 * @param i O índice da posição onde a chave a ser inserida deve estar.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 * @return Retorna uma struct do tipo promovido_aux que contém a chave promovida e o RRN do filho direito.
 */
promovido_aux btree_divide(promovido_aux promo, btree_node *node, int i, btree *t);
 
/**
 * Responsável por remover uma chave (k) de uma Árvore-B (T). Atualiza os parâmetros da Árvore-B conforme necessário.<br />
 * A chave a ser removida deve estar no formato correto e com tamanho t->tam_chave.<br />
 * O funcionamento deve ser genérico para qualquer Árvore-B, considerando que os únicos parâmetros que se alteram entre
 * as árvores é o t->tam_chave.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * ...<br />
 * char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];<br />
 * sprintf(usuario_str, "%s%04d", id_usuario, rrn_usuario);<br />
 * btree_delete(usuario_str, &usuarios_idx);<br />
 * ...<br />
 * </code>
 *
 * @param chave Chave a ser removida da Árvore-B.
 * @param t Ponteiro para o índice do tipo Árvore-B do qual será removida a chave.
 */
void btree_delete(char *chave, btree *t);
 
/**
 * Função auxiliar de remoção de uma chave (k) de uma Árvore-B (T). Atualiza os parâmetros da Árvore-B conforme necessário.<br />
 * Esta é uma função recursiva. Ela recebe o RRN do nó a ser trabalhado sobre.<br />
 *
 * @param chave Chave a ser removida da Árvore-B.
 * @param rrn RRN do nó no qual deverá ser processado.
 * @param t Ponteiro para o índice do tipo Árvore-B do qual será removida a chave.
 * @return Indica se a remoção deixou o nó que foi processado com menos chaves que o mínimo necessário.
 */
bool btree_delete_aux(char *chave, int rrn, btree *t);
 
/**
 * Função auxiliar para redistribuir ou concatenar nós irmãos adjacentes à esquerda e à direita de um nó pai em uma Árvore-B (T).
 * Atualiza os parâmetros conforme necessário.<br />
 *
 * @param node Ponteiro para nó pai dos nós irmãos adjacentes que deve ser redistribuidos ou concatenados.
 * @param i O índice da posição no nó pai onde se encontra a chave separadora dos nós irmãos adjacentes.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual serão redistribuídos ou concatenados os nós irmãos adjacentes.
 * @return Indica se a redistribuição ou concatenação deixou o nó pai com menos chaves que o mínimo necessário.
 */
bool btree_borrow_or_merge(btree_node *node, int i, btree *t);
 
/**
 * Responsável por buscar uma chave (k) em uma Árvore-B (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para a string result pode ser fornecido opcionalmente, e conterá o resultado encontrado.<br />
 * Esta é uma função recursiva. O parâmetro rrn recebe a raíz da Árvore-B na primeira chamada, e nas chamadas
 * subsequentes é o RRN do filho de acordo com o algoritmo fornecido.<br />
 * Comportamento de acordo com as especificações do PDF sobre Árvores-B e suas operações.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * char result[TAM_CHAVE_USUARIOS_IDX + 1];<br />
 * bool found = btree_search(result, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = btree_search(NULL, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Busca por uma chave de tamanho variável (específico para o caso de buscas por chaves PIX).<br />
 * ...<br />
 * char titulo_str[TAM_MAX_TITULO];<br />
 * strcpy(titulo_str, titulo);<br />
 * strpadright(titulo_str, '#', TAM_MAX_TITULO - 1);<br />
 * bool found = btree_search(NULL, false, titulo_str, titulo_idx.rrn_raiz, &titulo_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 4. Há interesse no caminho feito para encontrar a chave.<br />
 * ...<br />
 * char result[TAM_CHAVE_USUARIOS_IDX + 1];<br />
 * printf(RRN_NOS);<br />
 * bool found = btree_search(result, true, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx);<br />
 * printf("\n");<br />
 * </code>
 *
 * @param result Ponteiro para ser escrita a chave encontrada. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Árvore-B.
 * @param rrn RRN do nó no qual deverá ser processado. É o RRN da raíz da Árvore-B caso seja a primeira chamada.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool btree_search(char *result, bool exibir_caminho, char *chave, int rrn, btree *t);
 
/**
 * Responsável por buscar uma chave (k) dentro do nó de uma Árvore-B (T) de forma eficiente. O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e indica o índice da chave encontrada (caso tenha sido encontrada)
 * ou o índice do filho onde esta chave deve estar (caso não tenha sido encontrada).<br />
 *
 * @param result Ponteiro para ser escrito o índice da chave encontrada ou do filho onde ela deve estar. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Árvore-B.
 * @param node Ponteiro para o nó onde a busca deve ser feita.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool btree_binary_search(int *result, bool exibir_caminho, char *chave, btree_node *node, btree *t);
 
/**
 * Função para percorrer uma Árvore-B (T) em ordem.<br />
 * Os parâmetros chave_inicio e chave_fim podem ser fornecidos opcionalmente, e contém o intervalo do percurso.
 * Caso chave_inicio e chave_fim sejam NULL, o índice inteiro é percorrido.
 * Esta é uma função recursiva. O parâmetro rrn recebe a raíz da Árvore-B na primeira chamada, e nas chamadas
 * subsequentes é o RRN do filho de acordo com o algoritmo de percursão em ordem.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * // Exemplo 1. Intervalo não especificado.
 * ...<br />
 * bool imprimiu = btree_print_in_order(NULL, NULL, exibir_btree_usuario, usuarios_idx.rrn_raiz, &usuarios_idx);
 * ...<br />
 * <br />
 * // Exemplo 2. Imprime as transações contidas no intervalo especificado.
 * ...<br />
 * bool imprimiu = btree_print_in_order(data_inicio, data_fim, exibir_btree_data_curso_usuario, data_curso_usuario_idx.rrn_raiz, &data_curso_usuario_idx);
 * ...<br />
 * </code>
 *
 * @param chave_inicio Começo do intervalo. É ignorado caso NULL.
 * @param chave_fim Fim do intervalo. É ignorado caso NULL.
 * @param exibir Função utilizada para imprimir uma chave no índice. É informada a chave para a função.
 * @param rrn RRN do nó no qual deverá ser processado.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 * @return Indica se alguma chave foi impressa.
 */
bool btree_print_in_order(char *chave_inicio, char *chave_fim, bool (*exibir)(char *chave), int rrn, btree *t);
 
/**
 * Função interna para ler um nó em uma Árvore-B (T).<br />
 *
 * @param no No a ser lido da Árvore-B.
 * @param t Árvore-B na qual será feita a leitura do nó.
 */
btree_node btree_read(int rrn, btree *t);
 
/**
 * Função interna para escrever um nó em uma Árvore-B (T).<br />
 *
 * @param no No a ser escrito na Árvore-B.
 * @param t Árvore-B na qual será feita a escrita do nó.
 */
void btree_write(btree_node no, btree *t);
 
/**
 * Função interna para alocar o espaço necessário dos campos chaves (vetor de strings) e filhos (vetor de inteiros) da struct btree_node.<br />
 *
 * @param t Árvore-B base para o qual será alocado um struct btree_node.
 */
btree_node btree_node_malloc(btree *t);
 
/**
 * Função interna para liberar o espaço alocado dos campos chaves (vetor de strings) e filhos (vetor de inteiros) da struct btree_node.<br />
 *
 * @param t Árvore-B base para o qual será liberado o espaço alocado para um struct btree_node.
 */
void btree_node_free(btree_node no);
 
/**
 * Função interna para calcular o tamanho em bytes de uma Árvore-B.<br />
 *
 * @param t Árvore-B base para o qual será calculado o tamanho.
 */
int btree_register_size(btree *t);
 
/**
 * Preenche uma string str com o caractere pad para completar o tamanho size.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 * @param pad Caractere utilizado para fazer o preenchimento à direita.
 * @param size Tamanho desejado para a string.
 */
char *strpadright(char *str, char pad, unsigned size);
 
/**
 * Converte uma string str para letras maiúsculas.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 */
char *strupr(char *str);
 
/**
 * Converte uma string str para letras minúsculas.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 */
char *strlower(char *str);
 
/* <<< COLOQUE AQUI OS DEMAIS PROTÓTIPOS DE FUNÇÕES, SE NECESSÁRIO >>> */
 
// peguei essa funcao em: https://stackoverflow.com/questions/8377412/ceil-function-how-can-we-implement-it-ourselves
int ceil(double num) {
  int inum = (int)num;
 
  if (num == (double)inum) {
    return inum;
  }
 
  return inum + 1;
}
 
btree_node btree_father_search(int *pos_chave, char *chave, int rrn, btree *t, btree_node *pai);
 
void recriar_usuarios_idx();
 
/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */
 
int main() {
  // variáveis utilizadas pelo interpretador de comandos
  char input[500];
  uint64_t seed = 2;
  char datetime[TAM_DATETIME] = "202103181430";  // UTC 18/03/2021 14:30:00
  char id_usuario[TAM_ID_USUARIO];
  char nome[TAM_MAX_NOME];
  char email[TAM_MAX_EMAIL];
  char telefone[TAM_TELEFONE];
  char id_curso[TAM_ID_CURSO];
  char titulo[TAM_MAX_TITULO];
  char instituicao[TAM_MAX_INSTITUICAO];
  char ministrante[TAM_MAX_MINISTRANTE];
  char lancamento[TAM_DATE];
  char categoria[TAM_MAX_CATEGORIA];
  int carga;
  double valor;
  char data_inicio[TAM_DATETIME];
  char data_fim[TAM_DATETIME];
  char status;
 
  scanf("SET BTREE_ORDER %d;\n", &btree_order);
 
  scanf("SET ARQUIVO_USUARIOS TO '%[^']';\n", ARQUIVO_USUARIOS);
  int temp_len = strlen(ARQUIVO_USUARIOS);
  qtd_registros_usuarios = temp_len / TAM_REGISTRO_USUARIO;
  ARQUIVO_USUARIOS[temp_len] = '\0';
 
  scanf("SET ARQUIVO_CURSOS TO '%[^']';\n", ARQUIVO_CURSOS);
  temp_len = strlen(ARQUIVO_CURSOS);
  qtd_registros_cursos = temp_len / TAM_REGISTRO_CURSO;
  ARQUIVO_CURSOS[temp_len] = '\0';
 
  scanf("SET ARQUIVO_INSCRICOES TO '%[^']';\n", ARQUIVO_INSCRICOES);
  temp_len = strlen(ARQUIVO_INSCRICOES);
  qtd_registros_inscricoes = temp_len / TAM_REGISTRO_INSCRICAO;
  ARQUIVO_INSCRICOES[temp_len] = '\0';
 
  // inicialização do gerador de números aleatórios e função de datas
  prng_srand(seed);
  putenv("TZ=UTC");
  set_time(datetime);
 
  criar_usuarios_idx();
  criar_cursos_idx();
  criar_inscricoes_idx();
  criar_titulo_idx();
  criar_data_curso_usuario_idx();
  criar_categorias_idx();
 
  while (1) {
    fgets(input, 500, stdin);
    printf("%s", input);
    clear_input(input);
 
    if (strcmp("", input) == 0)
      continue;  // não avança o tempo nem imprime o comando este seja em branco
 
    /* Funções principais */
    if (sscanf(input, "INSERT INTO usuarios VALUES ('%[^']', '%[^']', '%[^']', '%[^']');", id_usuario, nome, email, telefone) == 4)
      cadastrar_usuario_menu(id_usuario, nome, email, telefone);
    else if (sscanf(input, "INSERT INTO usuarios VALUES ('%[^']', '%[^']', '%[^']');", id_usuario, nome, email) == 3) {
      strcpy(telefone, "");
      strpadright(telefone, '*', TAM_TELEFONE - 1);
      cadastrar_usuario_menu(id_usuario, nome, email, telefone);
    } else if (sscanf(input, "UPDATE usuarios SET telefone = '%[^']' WHERE id_usuario = '%[^']';", telefone, id_usuario) == 2)
      cadastrar_telefone_menu(id_usuario, telefone);
    else if (sscanf(input, "DELETE FROM usuarios WHERE id_usuario = '%[^']';", id_usuario) == 1)
      remover_usuario_menu(id_usuario);
    else if (sscanf(input, "INSERT INTO cursos VALUES ('%[^']', '%[^']', '%[^']', '%[^']', %d, %lf);", titulo, instituicao, ministrante, lancamento, &carga, &valor) == 6)
      cadastrar_curso_menu(titulo, instituicao, ministrante, lancamento, carga, valor);
    else if (sscanf(input, "UPDATE usuarios SET saldo = saldo + %lf WHERE id_usuario = '%[^']';", &valor, id_usuario) == 2)
      adicionar_saldo_menu(id_usuario, valor);
    else if (sscanf(input, "INSERT INTO inscricoes VALUES ('%[^']', '%[^']');", id_curso, id_usuario) == 2)
      inscrever_menu(id_curso, id_usuario);
    else if (sscanf(input, "UPDATE cursos SET categorias = array_append(categorias, '%[^']') WHERE titulo = '%[^']';", categoria, titulo) == 2)
      cadastrar_categoria_menu(titulo, categoria);
    else if (sscanf(input, "UPDATE inscricoes SET status = '%c' WHERE id_curso = (SELECT id_curso FROM cursos WHERE titulo = '%[^']') AND id_usuario = '%[^']';", &status, titulo, id_usuario) == 3)
      atualizar_status_inscricoes_menu(id_usuario, titulo, status);
 
    /* Busca */
    else if (sscanf(input, "SELECT * FROM usuarios WHERE id_usuario = '%[^']';", id_usuario) == 1)
      buscar_usuario_id_menu(id_usuario);
    else if (sscanf(input, "SELECT * FROM cursos WHERE id_curso = '%[^']';", id_curso) == 1)
      buscar_curso_id_menu(id_curso);
    else if (sscanf(input, "SELECT * FROM cursos WHERE titulo = '%[^']';", titulo) == 1)
      buscar_curso_titulo_menu(titulo);
 
    /* Listagem */
    else if (strcmp("SELECT * FROM usuarios ORDER BY id_usuario ASC;", input) == 0)
      listar_usuarios_id_menu();
    else if (sscanf(input, "SELECT * FROM cursos WHERE '%[^']' = ANY (categorias) ORDER BY id_curso ASC;", categoria) == 1)
      listar_cursos_categorias_menu(categoria);
    else if (sscanf(input, "SELECT * FROM inscricoes WHERE data_inscricao BETWEEN '%[^']' AND '%[^']' ORDER BY data_inscricao ASC;", data_inicio, data_fim) == 2)
      listar_inscricoes_periodo_menu(data_inicio, data_fim);
 
    /* Liberar espaço */
    else if (strcmp("VACUUM usuarios;", input) == 0)
      liberar_espaco_menu();
 
    /* Imprimir arquivos de dados */
    else if (strcmp("\\echo file ARQUIVO_USUARIOS", input) == 0)
      imprimir_arquivo_usuarios_menu();
    else if (strcmp("\\echo file ARQUIVO_CURSOS", input) == 0)
      imprimir_arquivo_cursos_menu();
    else if (strcmp("\\echo file ARQUIVO_INSCRICOES", input) == 0)
      imprimir_arquivo_inscricoes_menu();
 
    /* Imprimir índices primários */
    else if (strcmp("\\echo index usuarios_idx", input) == 0)
      imprimir_usuarios_idx_menu();
    else if (strcmp("\\echo index cursos_idx", input) == 0)
      imprimir_cursos_idx_menu();
    else if (strcmp("\\echo index inscricoes_idx", input) == 0)
      imprimir_inscricoes_idx_menu();
 
    /* Imprimir índices secundários */
    else if (strcmp("\\echo index titulo_idx", input) == 0)
      imprimir_titulo_idx_menu();
    else if (strcmp("\\echo index data_curso_usuario_idx", input) == 0)
      imprimir_data_curso_usuario_idx_menu();
    else if (strcmp("\\echo index categorias_secundario_idx", input) == 0)
      imprimir_categorias_secundario_idx_menu();
    else if (strcmp("\\echo index categorias_primario_idx", input) == 0)
      imprimir_categorias_primario_idx_menu();
 
    /* Liberar memória eventualmente alocada e encerrar programa */
    else if (strcmp("\\q", input) == 0) {
      return 0;
    } else if (sscanf(input, "SET SRAND %lu;", &seed) == 1) {
      prng_srand(seed);
      printf(SUCESSO);
      continue;
    } else if (sscanf(input, "SET TIME '%[^']';", datetime) == 1) {
      if (set_time(datetime))
        printf(SUCESSO);
      else
        printf(ERRO_VALOR_INVALIDO);
      continue;
    } else
      printf(ERRO_OPCAO_INVALIDA);
 
    tick_time();
  }
}
 
/* ========================================================================== */
 
/* Cria o índice primário usuarios_idx */
void criar_usuarios_idx() {
  char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];
  for (unsigned i = 0; i < qtd_registros_usuarios; ++i) {
    Usuario u = recuperar_registro_usuario(i);
 
    sprintf(usuario_str, "%s%04d", u.id_usuario, i);
    btree_insert(usuario_str, &usuarios_idx);
  }
  printf(INDICE_CRIADO, "usuarios_idx");
}
 
void recriar_usuarios_idx() {
  char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];
 
  usuarios_idx.rrn_raiz = -1;
  usuarios_idx.qtd_nos = 0;
 
  for (unsigned i = 0; i < qtd_registros_usuarios; ++i) {
    Usuario u = recuperar_registro_usuario(i);
 
    sprintf(usuario_str, "%s%04d", u.id_usuario, i);
    btree_insert(usuario_str, &usuarios_idx);
  }
}
 
/* Cria o índice primário cursos_idx */
void criar_cursos_idx() {
  char curso_str[TAM_CHAVE_CURSOS_IDX + 1];
  for (unsigned i = 0; i < qtd_registros_cursos; ++i) {
    Curso u = recuperar_registro_curso(i);
 
    sprintf(curso_str, "%s%04d", u.id_curso, i);
    btree_insert(curso_str, &cursos_idx);
  }
  printf(INDICE_CRIADO, "cursos_idx");
}
 
/* Cria o índice primário inscricoes_idx */
void criar_inscricoes_idx() {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(INDICE_CRIADO, "inscricoes_idx");
}
 
/* Cria o índice secundário titulo_idx */
void criar_titulo_idx() {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(INDICE_CRIADO, "titulo_idx");
}
 
/* Cria o índice secundário data_curso_usuario_idx */
void criar_data_curso_usuario_idx() {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(INDICE_CRIADO, "data_curso_usuario_idx");
}
 
/* Cria os índices (secundário e primário) de categorias_idx */
void criar_categorias_idx() {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(INDICE_CRIADO, "categorias_idx");
}
 
/* Exibe um usuario dado seu RRN */
bool exibir_usuario(int rrn) {
  if (rrn < 0)
    return false;
 
  Usuario u = recuperar_registro_usuario(rrn);
 
  printf("%s, %s, %s, %s, %.2lf\n", u.id_usuario, u.nome, u.email, u.telefone, u.saldo);
  return true;
}
 
/* Exibe um curso dado seu RRN */
bool exibir_curso(int rrn) {
  if (rrn < 0)
    return false;
 
  Curso j = recuperar_registro_curso(rrn);
 
  printf("%s, %s, %s, %s, %s, %d, %.2lf\n", j.id_curso, j.titulo, j.instituicao, j.ministrante, j.lancamento, j.carga, j.valor);
  return true;
}
 
/* Exibe uma inscricao dado seu RRN */
bool exibir_inscricao(int rrn) {
  if (rrn < 0)
    return false;
 
  Inscricao c = recuperar_registro_inscricao(rrn);
 
  printf("%s, %s, %s, %c, %s\n", c.id_curso, c.id_usuario, c.data_inscricao, c.status, c.data_atualizacao);
 
  return true;
}
 
bool exibir_btree_usuario(char *chave) {
  int rrn = atoi(chave + TAM_ID_USUARIO - 1);
  return exibir_usuario(rrn);
}
 
bool exibir_btree_curso(char *chave) {
  int rrn = atoi(chave + TAM_ID_CURSO - 1);
  return exibir_curso(rrn);
}
 
bool exibir_btree_inscricao(char *chave) {
  int rrn = atoi(chave + TAM_ID_CURSO + TAM_ID_USUARIO - 2);
  return exibir_inscricao(rrn);
}
 
bool exibir_btree_titulo(char *chave) {
  char id_curso[TAM_ID_CURSO];
  strncpy(id_curso, chave + TAM_MAX_TITULO - 1, TAM_ID_CURSO);
  id_curso[TAM_ID_CURSO - 1] = '\0';
  buscar_curso_id_menu(id_curso);
}
 
bool exibir_btree_data_curso_usuario(char *chave) {
  char id_insc[TAM_ID_CURSO + TAM_ID_USUARIO -1];
  strncpy(id_insc, chave + TAM_DATETIME - 1, TAM_ID_CURSO + TAM_ID_USUARIO - 2);
  id_insc[TAM_ID_CURSO + TAM_ID_USUARIO - 2] = '\0';
  char key[TAM_CHAVE_INSCRICOES_IDX + 1];
  key[TAM_CHAVE_INSCRICOES_IDX] = '\0';
  btree_search(key, false, id_insc, inscricoes_idx.rrn_raiz, &inscricoes_idx);
  exibir_btree_inscricao(key);
}
 
/* Recupera do arquivo de usuários o registro com o RRN
 * informado e retorna os dados na struct Usuario */
Usuario recuperar_registro_usuario(int rrn) {
  Usuario u;
  char temp[TAM_REGISTRO_USUARIO + 1], *p;
  strncpy(temp, ARQUIVO_USUARIOS + (rrn * TAM_REGISTRO_USUARIO), TAM_REGISTRO_USUARIO);
  temp[TAM_REGISTRO_USUARIO] = '\0';
 
  p = strtok(temp, ";");
  strcpy(u.id_usuario, p);
  p = strtok(NULL, ";");
  strcpy(u.nome, p);
  p = strtok(NULL, ";");
  strcpy(u.email, p);
  p = strtok(NULL, ";");
  strcpy(u.telefone, p);
  p = strtok(NULL, ";");
  u.saldo = atof(p);
  p = strtok(NULL, ";");
 
  return u;
}
 
/* Recupera do arquivo de cursos o registro com o RRN
 * informado e retorna os dados na struct Curso */
Curso recuperar_registro_curso(int rrn) {
  Curso u;
  char temp[TAM_REGISTRO_CURSO + 1], *p;
  strncpy(temp, ARQUIVO_CURSOS + (rrn * TAM_REGISTRO_CURSO), TAM_REGISTRO_CURSO);
  temp[TAM_REGISTRO_CURSO] = '\0';
 
  p = strtok(temp, ";");
  strcpy(u.id_curso, p);
  p = strtok(NULL, ";");
  strcpy(u.titulo, p);
  p = strtok(NULL, ";");
  strcpy(u.instituicao, p);
  p = strtok(NULL, ";");
  strcpy(u.ministrante, p);
  p = strtok(NULL, ";");
  strcpy(u.lancamento, p);
  p = strtok(NULL, ";");
  u.carga = atoi(p);
  p = strtok(NULL, ";");
  u.valor = atof(p);
  p = strtok(NULL, ";");
 
  return u;
}
 
/* Recupera do arquivo de inscricoes o registro com o RRN
 * informado e retorna os dados na struct Inscricao */
Inscricao recuperar_registro_inscricao(int rrn) {
  Inscricao u;
  char temp[TAM_REGISTRO_INSCRICAO + 1], *p;
  strncpy(temp, ARQUIVO_INSCRICOES + (rrn * TAM_REGISTRO_INSCRICAO), TAM_REGISTRO_INSCRICAO);
  temp[TAM_REGISTRO_USUARIO] = '\0';
 
  strncpy(u.id_curso, temp, 8);
  u.id_curso[8] = '\0';
  strncpy(u.id_usuario, &temp[8], 11);
  u.id_usuario[11] = '\0';
  strncpy(u.data_inscricao, &temp[19], 12);
  u.data_inscricao[12] = '\0';
  u.status = temp[31];
  strncpy(u.data_atualizacao, &temp[32], 12);
  u.data_atualizacao[12] = '\0';
 
  return u;
}
 
/* Escreve no arquivo de usuários na posição informada (RRN)
 * os dados na struct Usuario */
void escrever_registro_usuario(Usuario u, int rrn) {
  char temp[TAM_REGISTRO_USUARIO + 1], p[100];
  temp[0] = '\0';
  p[0] = '\0';
 
  strcpy(temp, u.id_usuario);
  strcat(temp, ";");
  strcat(temp, u.nome);
  strcat(temp, ";");
  strcat(temp, u.email);
  strcat(temp, ";");
  strcat(temp, u.telefone);
  strcat(temp, ";");
  sprintf(p, "%013.2lf", u.saldo);
  strcat(temp, p);
  strcat(temp, ";");
 
  strpadright(temp, '#', TAM_REGISTRO_USUARIO);
 
  strncpy(ARQUIVO_USUARIOS + rrn * TAM_REGISTRO_USUARIO, temp, TAM_REGISTRO_USUARIO);
}
 
/* Escreve no arquivo de cursos na posição informada (RRN)
 * os dados na struct Curso */
void escrever_registro_curso(Curso j, int rrn) {
  char temp[TAM_REGISTRO_CURSO + 1], p[100];
  temp[0] = '\0';
  p[0] = '\0';
  strcpy(temp, j.id_curso);
  strcat(temp, ";");
  strcat(temp, j.titulo);
  strcat(temp, ";");
  strcat(temp, j.instituicao);
  strcat(temp, ";");
  strcat(temp, j.ministrante);
  strcat(temp, ";");
  strcat(temp, j.lancamento);
  strcat(temp, ";");
  sprintf(p, "%04d", j.carga);
  strcat(temp, p);
  strcat(temp, ";");
  sprintf(p, "%013.2lf", j.valor);
  strcat(temp, p);
  strcat(temp, ";");
 
  if (j.categorias[0][0] != '\0') {
    strcat(temp, j.categorias[0]);
 
    if (j.categorias[1][0] != '\0')
      strcat(temp, "|");
  }
 
  if (j.categorias[1][0] != '\0') {
    strcat(temp, j.categorias[1]);
    if (j.categorias[2][0] != '\0')
      strcat(temp, "|");
  }
 
  if (j.categorias[2][0] != '\0') {
    strcat(temp, j.categorias[2]);
    strcat(temp, "|");
  }
 
  strcat(temp, ";");
 
  for (int i = strlen(temp); i < TAM_REGISTRO_CURSO; i++)
    temp[i] = '#';
 
  strncpy(ARQUIVO_CURSOS + rrn * TAM_REGISTRO_CURSO, temp, TAM_REGISTRO_CURSO);
  ARQUIVO_CURSOS[qtd_registros_cursos * TAM_REGISTRO_CURSO] = '\0';
}
 
/* Escreve no arquivo de inscricoes na posição informada (RRN)
 * os dados na struct Inscricao */
void escrever_registro_inscricao(Inscricao c, int rrn) {
  char temp[TAM_REGISTRO_INSCRICAO + 1];
  temp[0] = '\0';
  char str[2];
  str[0] = c.status;
  str[1] = '\0';
 
  strcpy(temp, c.id_curso);
  strcat(temp, c.id_usuario);
  strcat(temp, c.data_inscricao);
  strcat(temp, str);
  strcat(temp, c.data_atualizacao);
 
  strncpy(ARQUIVO_INSCRICOES + rrn * TAM_REGISTRO_INSCRICAO, temp, TAM_REGISTRO_INSCRICAO);
  ARQUIVO_INSCRICOES[qtd_registros_inscricoes * TAM_REGISTRO_INSCRICAO] = '\0';
}
 
/* Funções principais */
void cadastrar_usuario_menu(char *id_usuario, char *nome, char *email, char *telefone) {
  // checar se já existe usuário
  if (usuarios_idx.rrn_raiz != -1 && btree_search(NULL, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx)) {
    printf(ERRO_PK_REPETIDA, id_usuario);
    return;
  }
 
  // criar o usuario
  Usuario u;
  strcpy(u.email, email);
  strcpy(u.id_usuario, id_usuario);
  strcpy(u.nome, nome);
  u.saldo = 0;
  strcpy(u.telefone, telefone);
 
  // inserir no arquivo de dados
  escrever_registro_usuario(u, qtd_registros_usuarios);
 
  // inserir no arquivo de índice
  char index_user[TAM_CHAVE_USUARIOS_IDX + 1];
  sprintf(index_user, "%s%04d", u.id_usuario, qtd_registros_usuarios);
 
  qtd_registros_usuarios++;
 
  btree_insert(index_user, &usuarios_idx);
 
  printf(SUCESSO);
  // printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_usuario_menu");
}
 
void cadastrar_telefone_menu(char *id_usuario, char *telefone) {
  char user_node_rrn[TAM_CHAVE_USUARIOS_IDX + 1];
  int u_n_rrn;
  // buscar o usuario que tenha o id_usuario fornecido
 
  if (usuarios_idx.rrn_raiz == -1 || !btree_search(user_node_rrn, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
 
  u_n_rrn = atoi(&user_node_rrn[TAM_ID_USUARIO - 1]);
  Usuario aux = recuperar_registro_usuario(u_n_rrn);
  strcpy(aux.telefone, telefone);
  escrever_registro_usuario(aux, u_n_rrn);
  printf(SUCESSO);
 
  // printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_telefone_menu");
}
 
void remover_usuario_menu(char *id_usuario) {
  char user_node_rrn[TAM_CHAVE_USUARIOS_IDX + 1];
  int u_n_rrn;
 
  if (usuarios_idx.rrn_raiz == -1 || !btree_search(user_node_rrn, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
  user_node_rrn[TAM_CHAVE_USUARIOS_IDX] = '\0';
  u_n_rrn = atoi(&user_node_rrn[TAM_ID_USUARIO - 1]);
  Usuario user = recuperar_registro_usuario(u_n_rrn);
  btree_delete(id_usuario, &usuarios_idx);
  user.id_usuario[0] = '*';
  user.id_usuario[1] = '|';
  escrever_registro_usuario(user, u_n_rrn);
  printf(SUCESSO);
  // printf(ERRO_NAO_IMPLEMENTADO, "remover_usuario_menu");
}
 
void cadastrar_curso_menu(char *titulo, char *instituicao, char *ministrante, char *lancamento, int carga, double valor) {
  char *titulo_caps = (char *)malloc(TAM_MAX_TITULO);
  strcpy(titulo_caps, titulo);
  strupr(titulo_caps);
 
  // se ja tem curso com esse titulo cadastrado, retorna msg de erro
  if (titulo_idx.rrn_raiz != -1 && btree_search(NULL, false, titulo_caps, titulo_idx.rrn_raiz, &titulo_idx)) {
    printf(ERRO_PK_REPETIDA, titulo);
    free(titulo_caps);
    return;
  }
 
  // se nao, cria um novo curso e insere nos arquivos de dados e indice
  Curso c;
 
  strcpy(c.titulo, titulo);
  strcpy(c.instituicao, instituicao);
  strcpy(c.ministrante, ministrante);
  strcpy(c.lancamento, lancamento);
  c.carga = carga;
  c.valor = valor;
  sprintf(c.id_curso, "%08d", qtd_registros_cursos);
  strcpy(c.categorias[0], "\0");
  strcpy(c.categorias[1], "\0");
  strcpy(c.categorias[2], "\0");
 
  // inserir no arquivo de índice primario cursos
  char index_course[TAM_CHAVE_CURSOS_IDX + 1];
  sprintf(index_course, "%s%04d", c.id_curso, qtd_registros_cursos);
 
  char index_title[TAM_CHAVE_TITULO_IDX + 1];
  strcpy(index_title, titulo_caps);
  strpadright(index_title, '#', TAM_MAX_TITULO - 1);
  strcat(index_title, c.id_curso);
 
  qtd_registros_cursos++;
 
  escrever_registro_curso(c, qtd_registros_cursos - 1);
 
  btree_insert(index_course, &cursos_idx);
 
  btree_insert(index_title, &titulo_idx);
 
  printf(SUCESSO);
 
  free(titulo_caps);
}
 
void adicionar_saldo_menu(char *id_usuario, double valor) {
  // se o valor for negativo, exibe msg de erro
  if (valor <= 0) {
    printf(ERRO_VALOR_INVALIDO);
    return;
  }
 
  // variáveis para recuperar o rrn do nó que o usuário se encontra para poder atualizar o valor
  char user_node_rrn[TAM_CHAVE_USUARIOS_IDX + 1];
  int u_n_rrn;
 
  // se nao existe usuario de id id_usuario. entao exibir msg de erro
  if (usuarios_idx.rrn_raiz == -1 || !btree_search(user_node_rrn, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
 
  // se existe, atualizar o valor e escrever o registro
  user_node_rrn[TAM_CHAVE_USUARIOS_IDX] = '\0';
  u_n_rrn = atoi(&user_node_rrn[TAM_ID_USUARIO - 1]);
  Usuario aux = recuperar_registro_usuario(u_n_rrn);
  aux.saldo += valor;
  escrever_registro_usuario(aux, u_n_rrn);
  printf(SUCESSO);
}
 
void inscrever_menu(char *id_curso, char *id_usuario) {
  // ver se curso e usuario existem
  char user_node_rrn[TAM_CHAVE_USUARIOS_IDX + 1];
  int u_n_rrn;
  char course_node_rrn[TAM_CHAVE_CURSOS_IDX + 1];
  int c_n_rrn;
 
  if (!btree_search(course_node_rrn, false, id_curso, cursos_idx.rrn_raiz, &cursos_idx) || !btree_search(user_node_rrn, false, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
 
  // ver se usuario ja esta inscrito no curso
  char index_insc[TAM_CHAVE_INSCRICOES_IDX + 1];
 
  sprintf(index_insc, "%s%s", id_curso, id_usuario);
 
  if (btree_search(NULL, false, index_insc, inscricoes_idx.rrn_raiz, &inscricoes_idx)) {
    printf(ERRO_PK_REPETIDA, index_insc);
    return;
  }
 
  user_node_rrn[TAM_CHAVE_USUARIOS_IDX] = '\0';
  course_node_rrn[TAM_CHAVE_CURSOS_IDX] = '\0';
  u_n_rrn = atoi(&user_node_rrn[TAM_ID_USUARIO - 1]);
  c_n_rrn = atoi(&course_node_rrn[TAM_ID_CURSO - 1]);
  Usuario user = recuperar_registro_usuario(u_n_rrn);
  Curso course = recuperar_registro_curso(c_n_rrn);
 
  double valor = user.saldo - course.valor;
 
  if (valor < 0) {
    printf(ERRO_SALDO_NAO_SUFICIENTE);
    return;
  }
 
  user.saldo -= course.valor;
 
  escrever_registro_usuario(user, u_n_rrn);
 
  char timestamp[TAM_DATETIME];
  Inscricao insc;
 
  sprintf(&index_insc[TAM_ID_CURSO + TAM_ID_USUARIO - 2], "%04d", qtd_registros_inscricoes);
 
  btree_insert(index_insc, &inscricoes_idx);
 
  current_datetime(timestamp);
  strncpy(insc.data_inscricao, timestamp, TAM_DATETIME);
 
  char index_data[TAM_CHAVE_DATA_CURSO_USUARIO_IDX + 1];
  sprintf(index_data, "%s%s%s", timestamp, id_curso, id_usuario);
  btree_insert(index_data, &data_curso_usuario_idx);
 
  qtd_registros_inscricoes++;
 
  strncpy(insc.id_curso, id_curso, TAM_ID_CURSO);
  strncpy(insc.id_usuario, id_usuario, TAM_ID_USUARIO);
 
  current_datetime(timestamp);
  insc.status = 'A';
  strncpy(insc.data_atualizacao, timestamp, TAM_DATETIME);
 
  escrever_registro_inscricao(insc, qtd_registros_inscricoes - 1);
  printf(SUCESSO);
}
 
void cadastrar_categoria_menu(char *titulo, char *categoria) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_categoria_menu");
}
 
void atualizar_status_inscricoes_menu(char *id_usuario, char *titulo, char status) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "atualizar_status_inscricoes_menu");
}
 
/*void cadastrar_usuario_menu(char *id_usuario, char *nome, char *email, char *telefone) {
    //ver se o usuario ja existe
    usuarios_index *us = (usuarios_index*) busca_binaria(id_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false, 0);
 
    if (us){
        printf(ERRO_PK_REPETIDA, id_usuario);
        return;
    }
    //criar um novo usuario
    Usuario u;
 
    strcpy(u.id_usuario, id_usuario);
    strcpy(u.nome, nome);
    strcpy(u.email, email);
    strcpy(u.telefone, telefone);
    u.saldo = 0;
 
    //colocar ele no arquivo de dados
    escrever_registro_usuario(u, qtd_registros_usuarios);
 
    usuarios_idx[qtd_registros_usuarios].rrn = qtd_registros_usuarios;
    strcpy(usuarios_idx[qtd_registros_usuarios].id_usuario, u.id_usuario);
 
    qtd_registros_usuarios++;
 
    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
 
 
    //printf("AINDA FALTA TESTAR, parece funcionando legal");
    printf(SUCESSO);
}
 
void cadastrar_telefone_menu(char* id_usuario, char* telefone) {
    //buscar o usuario que tenha o id_usuario fornecido
    usuarios_index *u = (usuarios_index*) busca_binaria(id_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false, 0);
    //com o rnn, recuperar o registro
 
    if (!u){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    Usuario user = recuperar_registro_usuario(u->rrn);
    //atualizar
    strcpy(user.telefone, telefone);
    //escrever no arquivo de dados
    escrever_registro_usuario(user, u->rrn);
 
    printf(SUCESSO);
    //printf("AINDA FALTA TESTAR");
}
void remover_usuario_menu(char *id_usuario) {
 
    usuarios_index *u = (usuarios_index*) busca_binaria(id_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false, 0);
 
    if (!u){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    Usuario user = recuperar_registro_usuario(u->rrn);
    strcpy(user.id_usuario, "*|");
    strcpy(u->id_usuario, "*|");
    escrever_registro_usuario(user, u->rrn);
 
    u->rrn = -1;
 
    printf(SUCESSO);
    //printf("remover_usuario_menu -> FALTA TESTAR");
}
void cadastrar_curso_menu(char *titulo, char *instituicao, char *ministrante, char* lancamento, int carga, double valor) {
    //criar um novo curso
    char *titulo_caps = (char*)malloc(TAM_MAX_TITULO);
    strcpy(titulo_caps, titulo);
    strupr(titulo_caps);
 
    titulos_index *t = (titulos_index*) busca_binaria(titulo_caps, titulo_idx, qtd_registros_cursos, sizeof(titulos_index), bsearch_titulo_idx, false, 0);
 
    if (t){
        printf(ERRO_PK_REPETIDA, titulo);
        return;
    }
 
    Curso c;
 
    strcpy(c.titulo, titulo);
    strcpy(c.instituicao, instituicao);
    strcpy(c.ministrante, ministrante);
    strcpy(c.lancamento, lancamento);
    c.carga = carga;
    c.valor = valor;
    strcpy(c.categorias[0], "\0");
    strcpy(c.categorias[1], "\0");
    strcpy(c.categorias[2], "\0");
 
    //criando o idcurso
    char str[9];
    sprintf(str, "%d", qtd_registros_cursos);
    int strl = strlen(str);
    char id_curso[9];
 
    for (int i = 0; i < 8-strl; i++){
        strcat(id_curso, "0");
    }
 
    strcat(id_curso, str);
    strcpy(c.id_curso, id_curso);
 
    //colocar ele no arquivo de index
    cursos_idx[qtd_registros_cursos].rrn = qtd_registros_cursos;
    strcpy(cursos_idx[qtd_registros_cursos].id_curso, id_curso);
    qsort(cursos_idx, qtd_registros_cursos + 1, sizeof(cursos_index), qsort_cursos_idx);
 
    //insere no arquivo de indice secundario titulo
    strcpy(titulo_idx[qtd_registros_cursos].titulo, titulo_caps);
    strcpy(titulo_idx[qtd_registros_cursos].id_curso, id_curso);
    qsort(titulo_idx, qtd_registros_cursos + 1, sizeof(titulos_index), qsort_titulo_idx);
 
    qtd_registros_cursos++;
 
    escrever_registro_curso(c, qtd_registros_cursos - 1);
    printf(SUCESSO);
    //printf("Testando!!\n");
    free(titulo_caps);
}
 
void adicionar_saldo_menu(char *id_usuario, double valor) {
    //buscar o usuario que tenha o id_usuario fornecido
    usuarios_index *u = (usuarios_index*) busca_binaria(id_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false, 0);
    //com o rnn, recuperar o registro
 
    if (!u){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    if (valor < 0){
        printf(ERRO_VALOR_INVALIDO);
        return;
    }
 
    if (u->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    Usuario user = recuperar_registro_usuario(u->rrn);
    //atualizar
    user.saldo = user.saldo + valor;
    //escrever no arquivo de dados
    escrever_registro_usuario(user, u->rrn);
    //printf(ERRO_NAO_IMPLEMENTADO, "adicionar_saldo_menu");
    printf(SUCESSO);
}
 
void inscrever_menu(char *id_curso, char *id_usuario) {
    cursos_index *c = (cursos_index*) busca_binaria(id_curso, cursos_idx, qtd_registros_cursos, sizeof(cursos_index), qsort_cursos_idx, false, 0);
    usuarios_index *u = (usuarios_index*) busca_binaria(id_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false, 0);
 
    if (!c || !u){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    //ver se ja existe inscricao (Para fazer essa parte, conversei em alto nível com o aluno Sérgio Néres Junior)
    inscricoes_index i;
    strcpy(i.id_curso, id_curso);
    strcpy(i.id_usuario, id_usuario);
 
    inscricoes_index *aux = (inscricoes_index*) busca_binaria(&i, inscricoes_idx, qtd_registros_inscricoes, sizeof(inscricoes_index), qsort_inscricoes_idx, false, 0);
    if (aux){ //encontrou inscricao
        printf(ERRO_PK_REPETIDA,strcat(id_curso,id_usuario));
        return;
    }
 
    //ver se o usuario tem saldo suficente
 
    Usuario user = recuperar_registro_usuario(u->rrn);
    Curso course = recuperar_registro_curso(c->rrn);
    double valor = user.saldo - course.valor;
 
    if (valor < 0){
        printf(ERRO_SALDO_NAO_SUFICIENTE);
        return;
    }
 
    user.saldo -= course.valor;
 
    escrever_registro_usuario(user, u->rrn);
 
    char timestamp[TAM_DATETIME];
    Inscricao insc;
 
    //nao encontrou inscricao, usuario e curso existem e há saldo, entao basta inscreve-lo
    //escrever no indice
    inscricoes_idx[qtd_registros_inscricoes].rrn = qtd_registros_inscricoes;
    strcpy(inscricoes_idx[qtd_registros_inscricoes].id_curso, id_curso);
    strcpy(inscricoes_idx[qtd_registros_inscricoes].id_usuario, id_usuario);
    qsort(inscricoes_idx, qtd_registros_inscricoes, sizeof(inscricoes_index), qsort_inscricoes_idx);
 
    //escrever no indice secundario
    strcpy(data_curso_usuario_idx[qtd_registros_inscricoes].id_curso, id_curso);
    strcpy(data_curso_usuario_idx[qtd_registros_inscricoes].id_usuario, id_usuario);
    current_datetime(timestamp);
    strcpy(insc.data_inscricao, timestamp);
    strcpy(data_curso_usuario_idx[qtd_registros_inscricoes].data, timestamp);
    qsort(data_curso_usuario_idx, qtd_registros_inscricoes + 1, sizeof(data_curso_usuario_index), qsort_data_curso_usuario_idx);
 
    qtd_registros_inscricoes++;
 
    strcpy(insc.id_curso, id_curso);
    strcpy(insc.id_usuario, id_usuario);
 
    current_datetime(timestamp);
    insc.status = 'A';
    strcpy(insc.data_atualizacao, timestamp);
 
    escrever_registro_inscricao(insc,qtd_registros_inscricoes-1);
    printf(SUCESSO);
}
 
void cadastrar_categoria_menu(char* titulo, char* categoria) {
    titulos_index *t = (titulos_index*) busca_binaria(strupr(titulo), titulo_idx, qtd_registros_cursos, sizeof(titulos_index), bsearch_titulo_idx, false, 0);
    if (!t){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    cursos_index *c = (cursos_index*) busca_binaria(t->id_curso, cursos_idx, qtd_registros_cursos, sizeof(cursos_index), qsort_cursos_idx, false, 0);
    Curso curso = recuperar_registro_curso(c->rrn);
    char *token1, *token2, *token3;
 
    token1 = strtok(categoria, "|");
    token2 = strtok(NULL, "|");
    token3 = strtok(NULL, "|");
 
    if(!strcmp(curso.categorias[0], token1) || !strcmp(curso.categorias[1], token1) || !strcmp(curso.categorias[2], token1)){
        printf(ERRO_CATEGORIA_REPETIDA, curso.titulo, token1);
        return;
    }
 
    int siz1 = (c->rrn * TAM_REGISTRO_CURSO) + strlen(curso.id_curso)+1 + strlen(curso.titulo)+1 + strlen(curso.instituicao)+1 + strlen(curso.ministrante)+1 + strlen(curso.lancamento)+1 + 4 + 13 + 1;
    if (ARQUIVO_CURSOS[siz1] == ';' && ARQUIVO_CURSOS[siz1 + 1] == ';'){ //nao ha nenhuma categoria
        strcpy(curso.categorias[0], token1);
 
        if (token2)
            strcpy(curso.categorias[1], token2);
 
        if (token3)
            strcpy(curso.categorias[2], token3);
 
        if (!token2 && !token3){
            strcpy(curso.categorias[1], "\0");
            strcpy(curso.categorias[2], "\0");
        }
    }else if(ARQUIVO_CURSOS[siz1 + strlen(curso.categorias[0]) + 1] == ';'){ //há 1 categoria
        strcpy(curso.categorias[1], token1);
 
        if (token2)
            strcpy(curso.categorias[1], token2);
        else
            strcpy(curso.categorias[2], "\0");
 
    }else{ //há 2 categorias
        strcpy(curso.categorias[1], token1);
    }
 
    escrever_registro_curso(curso, c->rrn);
 
    inverted_list_insert(categoria, c->id_curso, &categorias_idx);
 
    printf(SUCESSO);
}
 
void atualizar_status_inscricoes_menu(char *id_usuario, char *titulo, char status) {
    titulos_index *t = (titulos_index*) busca_binaria(strupr(titulo), titulo_idx, qtd_registros_cursos, sizeof(titulos_index), bsearch_titulo_idx, false, 0);
    inscricoes_index i;
    strcpy(i.id_curso, t->id_curso);
    strcpy(i.id_usuario, id_usuario);
    inscricoes_index *aux = (inscricoes_index*) busca_binaria(&i, inscricoes_idx, qtd_registros_inscricoes, sizeof(inscricoes_index), qsort_inscricoes_idx, false, 0);
 
    if (!aux){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
 
    Inscricao insc = recuperar_registro_inscricao(aux->rrn);
 
    char timestamp[TAM_DATETIME];
    current_datetime(timestamp);
 
    insc.status = status;
    strcpy(insc.data_atualizacao, timestamp);
 
    escrever_registro_inscricao(insc,aux->rrn);
 
    printf(SUCESSO);
}*/
 
/* Busca */
void buscar_usuario_id_menu(char *id_usuario) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  // buscar usuario por id
  char user_rrn[TAM_CHAVE_USUARIOS_IDX + 1];
  printf(RRN_NOS);
  if (!btree_search(user_rrn, true, id_usuario, usuarios_idx.rrn_raiz, &usuarios_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
 
  exibir_btree_usuario(user_rrn);
}
 
void buscar_curso_id_menu(char *id_curso) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  // buscar curso por id
  char curso_rrn[TAM_CHAVE_CURSOS_IDX + 1];
  printf(RRN_NOS);
  if (!btree_search(curso_rrn, true, id_curso, cursos_idx.rrn_raiz, &cursos_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
 
  exibir_btree_curso(curso_rrn);
}
 
void buscar_curso_titulo_menu(char *titulo) {
  char curso_rrn[TAM_CHAVE_TITULO_IDX + 1];
 
  printf(RRN_NOS);
  if (!btree_search(curso_rrn, true, strupr(titulo), titulo_idx.rrn_raiz, &titulo_idx)) {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    return;
  }
 
  exibir_btree_titulo(curso_rrn);
}
 
/* Listagem */
void listar_usuarios_id_menu() {
  if (!btree_print_in_order(NULL, NULL, exibir_btree_usuario, usuarios_idx.rrn_raiz, &usuarios_idx))
    printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
}
 
void listar_cursos_categorias_menu(char *categoria) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "listar_cursos_categorias_menu");
}
 
void listar_inscricoes_periodo_menu(char *data_inicio, char *data_fim) {
  if (!btree_print_in_order(data_inicio, data_fim, exibir_btree_data_curso_usuario, data_curso_usuario_idx.rrn_raiz, &data_curso_usuario_idx))
    printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
  else
    printf("\n");
}
 
/* Liberar espaço */
void liberar_espaco_menu() {
  char *aux = (char *)malloc(TAM_ARQUIVO_USUARIOS);
  aux[0] = '\0';
  int c = 0;
 
  for (int i = 0; i < TAM_ARQUIVO_USUARIOS; i += TAM_REGISTRO_USUARIO) {
    if (ARQUIVO_USUARIOS[i] == '*' && ARQUIVO_USUARIOS[i + 1] == '|') {
      c++;
      continue;
    } else {
      strncat(aux, &ARQUIVO_USUARIOS[i], TAM_REGISTRO_USUARIO);
    }
  }
 
  qtd_registros_usuarios -= c;
  strcpy(ARQUIVO_USUARIOS, aux);
  free(aux);
  recriar_usuarios_idx();
  printf(SUCESSO);
}
 
/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu() {
  if (qtd_registros_usuarios == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_USUARIOS);
}
 
void imprimir_arquivo_cursos_menu() {
  if (qtd_registros_cursos == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_CURSOS);
}
 
void imprimir_arquivo_inscricoes_menu() {
  if (qtd_registros_inscricoes == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_INSCRICOES);
}
 
/* Imprimir índices primários */
void imprimir_usuarios_idx_menu() {
  if (usuarios_idx.qtd_nos == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_USUARIOS_IDX);
}
 
void imprimir_cursos_idx_menu() {
  if (cursos_idx.qtd_nos == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_CURSOS_IDX);
}
 
void imprimir_inscricoes_idx_menu() {
  if (inscricoes_idx.qtd_nos == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_INSCRICOES_IDX);
}
 
/* Imprimir índices secundários */
void imprimir_titulo_idx_menu() {
  if (titulo_idx.qtd_nos == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_TITULO_IDX);
}
 
void imprimir_data_curso_usuario_idx_menu() {
  if (data_curso_usuario_idx.qtd_nos == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_DATA_CURSO_USUARIO_IDX);
}
 
void imprimir_categorias_secundario_idx_menu() {
  if (categorias_idx.qtd_registros_secundario == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_CATEGORIAS_SECUNDARIO_IDX);
}
 
void imprimir_categorias_primario_idx_menu() {
  if (categorias_idx.qtd_registros_primario == 0)
    printf(ERRO_ARQUIVO_VAZIO);
  else
    printf("%s\n", ARQUIVO_CATEGORIAS_PRIMARIO_IDX);
}
 
/* Função de comparação entre chaves do índice usuarios_idx */
int order_usuarios_idx(const void *key, const void *elem) {
  return strncmp(key, elem, TAM_ID_USUARIO - 1);
}
 
/* Função de comparação entre chaves do índice cursos_idx */
int order_cursos_idx(const void *key, const void *elem) {
  return strncmp(key, elem, TAM_ID_CURSO - 1);
}
 
/* Função de comparação entre chaves do índice inscricoes_idx */
int order_inscricoes_idx(const void *key, const void *elem) {
  return strncmp(key, elem, TAM_ID_CURSO + TAM_ID_USUARIO - 2);
}
 
/* Função de comparação entre chaves do índice titulo_idx */
int order_titulo_idx(const void *key, const void *elem) {
  return strncmp(key, elem, strlen(key));
}
 
/* Funções de comparação entre chaves do índice data_curso_usuario_idx */
int order_data_curso_usuario_idx(const void *key, const void *elem) {
  return strncmp(key, elem, TAM_DATETIME + TAM_ID_CURSO + TAM_ID_USUARIO - 3);
}
 
/* Função de comparação entre chaves do índice secundário de categorias_idx */
int order_categorias_idx(const void *key, const void *elem) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "order_categorias_idx");
  return -1;
}
 
/* Funções de manipulação de Lista Invertida */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t) {
  /*    strcpy(t->categorias_primario_idx[t->qtd_registros_primario].chave_primaria, chave_primaria);
  t->categorias_primario_idx[t->qtd_registros_primario].proximo_indice = -1;
  t->qtd_registros_primario++;
 
  int result;
  int indice_final;
 
  if (inverted_list_secondary_search(&result, false, chave_secundaria, t)){
      inverted_list_primary_search(NULL, false, result, &indice_final, t);
      t->categorias_primario_idx[indice_final].proximo_indice = t->qtd_registros_primario - 1;
  } else {
      strcpy(t->categorias_secundario_idx[t->qtd_registros_secundario].chave_secundaria, strupr(chave_secundaria));
      t->categorias_secundario_idx[t->qtd_registros_secundario].primeiro_indice = t->qtd_registros_primario - 1;
      t->qtd_registros_secundario++;
      qsort(t->categorias_secundario_idx, t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx);
  }*/
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_insert");
}
 
bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t) {
  /*    categorias_secundario_index* tem = (categorias_secundario_index*) busca_binaria(strupr(chave_secundaria), t->categorias_secundario_idx,t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx, false, 0);
 
  if (tem){
      *result = tem->primeiro_indice;
      return true;
  }else
      return false;*/
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_secondary_search");
  return false;
}
 
int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t) {
  /*    int qtd = 0;
  int i;
  for (i = indice; i != -1; i = t->categorias_primario_idx[i].proximo_indice, qtd++){
      if (exibir_caminho){
          printf(REGS_PERCORRIDOS);
          printf(" %d", i);
      }
 
      if (result)
          strcpy(result[qtd], t->categorias_primario_idx[i].chave_primaria);
 
      indice_final = &i;
  }
 
  if (exibir_caminho)
      printf("\n");
 
  return qtd;   */
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_primary_search");
  return -1;
}
 
bool inverted_list_binary_search(int *result, bool exibir_caminho, char *chave, inverted_list *t) {
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_binary_search");
  return false;
}
 
/* Funções de manipulação de Árvores-B */
void btree_insert(char *chave, btree *t) {
  // arvore esta vazia
  if (t->rrn_raiz == -1) {
    // aloca-se um novo nó
    btree_node node = btree_node_malloc(t);
 
    // copia-se a chave para o nó
    strncpy(node.chaves[0], chave, t->tam_chave);
 
    // é o único nó, então é nó folha
    node.folha = true;
 
    // primeiro nó, rrn = 0
    node.this_rrn = 0;
 
    // tem 1 chave
    node.qtd_chaves = 1;
 
    // vira a raíz
    t->rrn_raiz = node.this_rrn;
 
    // aumenta a qtd de nós da árvore
    t->qtd_nos++;
 
    // escreve na árvore em 'disco'
    btree_write(node, t);
 
    // free node
    btree_node_free(node);
  } else {  // árvore não está vazia
 
    promovido_aux promo = btree_insert_aux(chave, t->rrn_raiz, t);  // chamando a função auxiliar para tratar os casos: 1. há espaço no nó; 2.não há espaço no nó.
 
    if (*promo.chave_promovida != '\0') {  // caso haja chave a ser promovida (se nao tiver, entao a inserção ocorreu bem) (só vai haver nos casos de divisao)
      btree_node x = btree_node_malloc(t);
      x.folha = false;
      x.this_rrn = t->qtd_nos;
      x.qtd_chaves = 1;
      strncpy(x.chaves[0], promo.chave_promovida, t->tam_chave);
      x.filhos[0] = t->rrn_raiz;
      x.filhos[1] = promo.filho_direito;
 
      t->rrn_raiz = x.this_rrn;
      t->qtd_nos++;
 
      btree_write(x, t);
      btree_node_free(x);
    }
  }
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_insert");
}
 
promovido_aux btree_insert_aux(char *chave, int rrn, btree *t) {
  // le o nó passado
  btree_node x = btree_read(rrn, t);
 
  promovido_aux promo;
  int i = 0;
  promo.filho_direito = -1;
  promo.chave_promovida[0] = '\0';
 
  if (x.folha) {
    if (x.qtd_chaves < (btree_order - 1)) {  // caso haja espaço no nó, ele será inserido normalmente
      i = x.qtd_chaves - 1;
      while (i >= 0 && t->compar(chave, x.chaves[i]) < 0) {  // desloca os valores de forma que i + 1 seja o local correto para a inserção da chave
        strncpy(x.chaves[i + 1], x.chaves[i], t->tam_chave);
        x.chaves[i + 1][t->tam_chave] = '\0';
        i--;
      }
      // insere a chave
      strncpy(x.chaves[i + 1], chave, t->tam_chave);
      x.chaves[i + 1][t->tam_chave] = '\0';
      x.qtd_chaves++;
      // escreve no arquivo
      btree_write(x, t);
      // free
      btree_node_free(x);
 
      return promo;
    } else {  // se o nó não possuir espaço, faz o processo de divisão
      strncpy(promo.chave_promovida, chave, t->tam_chave);
      promo.chave_promovida[t->tam_chave] = '\0';
      return btree_divide(promo, &x, i, t);
    }
 
  } else {  // se o nó não é folha
    i = x.qtd_chaves - 1;
 
    while (i >= 0 && t->compar(chave, x.chaves[i]) < 0) {  // acha onde o nó filho onde a chave deveria estar
      i--;
    }
    i++;
 
    promo = btree_insert_aux(chave, x.filhos[i], t);  // faz a inserção recursiva até o nó ser folha
 
    if (promo.chave_promovida[0] != '\0') {  // se existe uma chave a ser promovida, então faz o processo de promoção e ajuste de ponteiros
      strncpy(chave, promo.chave_promovida, t->tam_chave);
      if (x.qtd_chaves < (btree_order - 1)) {  // se tem espaco no nó pai, insere aqui
        i = x.qtd_chaves - 1;
 
        while (i >= 0 && t->compar(chave, x.chaves[i]) < 0) {
          strncpy(x.chaves[i + 1], x.chaves[i], t->tam_chave);
          x.chaves[i + 1][t->tam_chave] = '\0';
          x.filhos[i + 2] = x.filhos[i + 1];
          i--;
        }
 
        strncpy(x.chaves[i + 1], chave, t->tam_chave);
        x.chaves[i + 1][t->tam_chave] = '\0';
        x.filhos[i + 2] = promo.filho_direito;
 
        x.qtd_chaves++;
        btree_write(x, t);
        btree_node_free(x);
 
        promo.filho_direito = -1;
        promo.chave_promovida[0] = '\0';
        return promo;
      } else {  // se não tem espaço, faz a divisão
        strncpy(promo.chave_promovida, chave, t->tam_chave);
        promo.chave_promovida[t->tam_chave] = '\0';
        return btree_divide(promo, &x, i, t);
      }
    } else {  // se não existe uma chave a ser promovida, entao a inserção foi concluida
      btree_node_free(x);
 
      promo.filho_direito = -1;
      promo.chave_promovida[0] = '\0';
      return promo;
    }
  } /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_insert_aux");
}
 
promovido_aux btree_divide(promovido_aux promo, btree_node *node, int i, btree *t) {
  int n = node->qtd_chaves - 1;  // n entendi muito bem como usar o i, entao deixei ele de fora
 
  btree_node y = btree_node_malloc(t);  // nó da direita
 
  y.qtd_chaves = ((btree_order - 1) / 2);  // quantidade de chaves é dividida igualmente
 
  y.folha = node->folha;  // se o nó original é folha, o novo também será
 
  bool flag = false;                                                                 // flag que indica se a chave que sera inserida ja foi inserida
  for (int c = y.qtd_chaves - 1; c >= 0; c--) {                                      // parte de atribuição de chaves e alocação de
    if (flag == false && (t->compar(promo.chave_promovida, node->chaves[n]) > 0)) {  // se essa condiçao for verdadeira, entao significa que a chave deve ser inserida no nó da direita
      strncpy(y.chaves[c], promo.chave_promovida, t->tam_chave);
      y.filhos[c + 1] = promo.filho_direito;
      flag = true;
    } else {  // se nao, distribui as chaves
      strncpy(y.chaves[c], node->chaves[n], t->tam_chave);
      y.chaves[c][t->tam_chave] = '\0';
      y.filhos[c + 1] = node->filhos[n + 1];
      n--;
    }
  }
 
  if (flag == false) {  // caso a chave ainda nao tenha sido inserida, significa que ela deve ser inserida no nó da esquerda
    while (n >= 0 && (t->compar(promo.chave_promovida, node->chaves[n]) < 0)) {
      strncpy(node->chaves[n + 1], node->chaves[n], t->tam_chave);
      node->chaves[n + 1][t->tam_chave] = '\0';
      node->filhos[n + 2] = node->filhos[n + 1];
      n--;
    }
    strncpy(node->chaves[n + 1], promo.chave_promovida, t->tam_chave);
    node->filhos[n + 2] = promo.filho_direito;
  }
 
  strncpy(promo.chave_promovida, node->chaves[btree_order / 2], t->tam_chave);
  y.filhos[0] = node->filhos[1 + (btree_order / 2)];
 
  if (node->filhos[1 + (btree_order / 2)] != -1)
    node->filhos[1 + (btree_order / 2)] = -1;
 
  node->qtd_chaves = btree_order / 2;
  y.this_rrn = t->qtd_nos;
  promo.filho_direito = y.this_rrn;
  t->qtd_nos++;
  btree_write(*node, t);
  btree_write(y, t);
 
  btree_node_free(*node);
  btree_node_free(y);
 
  return promo;
 
  /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_divide");
}
 
// x
void btree_delete(char *chave, btree *t) {
  // busca pela chave
  if (btree_search(NULL, false, chave, t->rrn_raiz, t)) {
    btree_delete_aux(chave, t->rrn_raiz, t);
  } else {
    printf(ERRO_REGISTRO_NAO_ENCONTRADO);
  }
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_delete");
}
// x
bool btree_delete_aux(char *chave, int rrn, btree *t) {
  // le o nó
  btree_node node = btree_read(rrn, t);
  int pos;  // pos vai conter a posicao da chave a ser removida
 
  if (btree_binary_search(&pos, false, chave, &node, t)) {
    if (node.folha) {
      // faz a remocao
      node.chaves[pos][0] = '\0';
      node.qtd_chaves--;
 
      // deslocar as chaves de node para a esquerda, se necessario
      for (int i = pos; i < node.qtd_chaves; i++) {
        strncpy(node.chaves[i], node.chaves[i + 1], t->tam_chave);
        node.chaves[i][t->tam_chave] = '\0';
      }
 
      node.chaves[node.qtd_chaves][0] = '\0';
      // checar underflow
      if (node.qtd_chaves < (ceil(btree_order / 2) - 1)) {
        int pos_chave_pai;
        btree_node raiz = btree_read(t->rrn_raiz, t);
        btree_node pai = btree_father_search(&pos_chave_pai, chave, t->rrn_raiz, t, &raiz);
        btree_write(node, t);
        return btree_borrow_or_merge(&pai, pos_chave_pai, t);
 
      } else {
        btree_write(node, t);
        return true;
      }
 
    } else {
      // se nao for folha, busca-se o predecessor imediato (maior elemento da subarvore esquerda), e troca-se com a chave a ser removida
      btree_node aux = btree_read(node.filhos[pos], t);
 
      while (!aux.folha) {  // enquanto nao chegar em um nó folha, continua-se a busca
        aux = btree_read(aux.filhos[aux.qtd_chaves], t);
      }
 
      char temp[t->tam_chave + 1];
      strncpy(temp, node.chaves[pos], t->tam_chave);
      strncpy(node.chaves[pos], aux.chaves[aux.qtd_chaves - 1], t->tam_chave);
      strncpy(aux.chaves[aux.qtd_chaves - 1], temp, t->tam_chave);
      aux.chaves[aux.qtd_chaves - 1][0] = '\0';
      aux.qtd_chaves--;
 
      // checar undwerflow
      if (aux.qtd_chaves < (ceil(btree_order / 2) - 1)) {
        int pos_chave_pai;
        btree_node raiz = btree_read(t->rrn_raiz, t);
        btree_node pai = btree_father_search(&pos_chave_pai, node.chaves[pos], t->rrn_raiz, t, &raiz);
        btree_write(node, t);
        btree_write(aux, t);
        return btree_borrow_or_merge(&pai, pos_chave_pai, t);
 
      } else {
        btree_write(node, t);
        btree_write(aux, t);
        return true;
      }
    }
  } else {
    return btree_delete_aux(chave, node.filhos[pos], t);
  }
}
// x
bool btree_borrow_or_merge(btree_node *node, int i, btree *t) {
  // i é a posição da chave separadora do nó pai
  // node é o nó pai
  // ver se tem espaco no nó da direita
  btree_node no = btree_read(node->filhos[i + 1], t);
 
  if (no.this_rrn != -1 && no.qtd_chaves > (ceil(btree_order / 2) - 1)) {  // entao faz o emprestimo do irmao a direita
    // desce a chave do pai para o filho i
    btree_node filho = btree_read(node->filhos[i], t);
    strcpy(filho.chaves[filho.qtd_chaves], node->chaves[i]);
    filho.qtd_chaves++;
    // sobe a menor chave do irmao a direita para o pai
    strcpy(node->chaves[i], no.chaves[0]);
    // desloca as chaves do irmao a direita
    for (int j = 0; j < no.qtd_chaves - 1; j++) {
      strcpy(no.chaves[j], no.chaves[j + 1]);
    }
    strcpy(no.chaves[no.qtd_chaves - 1], "\0");
    no.qtd_chaves--;
 
    btree_write(filho, t);
    btree_node_free(filho);
    btree_write(no, t);
    btree_node_free(no);
    btree_write(*node, t);
    btree_node_free(*node);
    return true;
  } else {  // se nao e se tiver irmao a esquerda, ve se tem espaco nele
    btree_node no_esq = btree_read(node->filhos[i - 1], t);
 
    if (no_esq.this_rrn != 1 && no_esq.qtd_chaves > (ceil(btree_order / 2) - 1)) {
      // entao faz o emprestimo do irmao da esquerda
      // desce a chave do pai para o filho i
      btree_node filho = btree_read(node->filhos[i], t);
      strcpy(filho.chaves[filho.qtd_chaves], node->chaves[i]);
      filho.qtd_chaves++;
      // sobe a maior chave do irmao a esquerda para o pai
      strcpy(node->chaves[i], no_esq.chaves[no_esq.qtd_chaves - 1]);
      strcpy(no_esq.chaves[no_esq.qtd_chaves - 1], "\0");
      no_esq.qtd_chaves--;
 
      btree_write(filho, t);
      btree_node_free(filho);
      btree_write(no_esq, t);
      btree_node_free(no_esq);
      btree_write(*node, t);
      btree_node_free(*node);
 
      return true;
    } else {  // merge (se nao tem espaco nos irmaos, entao deve-se concatenar chave separadora do pai+irmaos)
      // concatena o filho i com o irmao a direita
      btree_node filho = btree_read(node->filhos[i], t);
      // desce a chave do pai para o filho i
      strcpy(filho.chaves[filho.qtd_chaves], node->chaves[i]);
      node->chaves[i][0] = '\0';
      filho.qtd_chaves++;
 
      // concatena as chaves do filho no irmao a direita
      for (int j = 0; j < filho.qtd_chaves; j++) {
        strcpy(no.chaves[no.qtd_chaves], filho.chaves[j]);
        no.qtd_chaves++;
        filho.qtd_chaves--;
        filho.chaves[j][0] = '\0';
      }
 
      // filho vai ficar vazio
 
      // reordena as chaves do no
      for (int j = 0; j < no.qtd_chaves - 1; j++) {
        for (int k = j + 1; k < no.qtd_chaves; k++) {
          if (strcmp(no.chaves[j], no.chaves[k]) > 0) {
            char temp[t->tam_chave + 1];
            strncpy(temp, no.chaves[j], t->tam_chave);
            strncpy(no.chaves[j], no.chaves[k], t->tam_chave);
            strncpy(no.chaves[k], temp, t->tam_chave);
          }
        }
      }
 
      // desloca as chaves do pai
      int j = i;
 
      for (; j < node->qtd_chaves - 1; j++) {
        strcpy(node->chaves[j], node->chaves[j + 1]);
      }
 
      strcpy(node->chaves[j], "\0");
      node->filhos[i] = -1;
      node->qtd_chaves--;
 
      if (node->this_rrn == t->rrn_raiz && node->qtd_chaves == 0) {
        for (int k = 0; k < btree_order; k++) {
          node->filhos[k] = -1;
        }
 
        t->rrn_raiz = no.this_rrn;
 
        btree_write(*node, t);
        btree_node_free(*node);
        btree_write(filho, t);
        btree_node_free(filho);
        btree_write(no, t);
        btree_node_free(no);
        return true;
      }
 
      // desloca os filhos do pai
      for (j = i; j < btree_order; j++) {
        node->filhos[j] = node->filhos[j + 1];
      }
 
      node->filhos[j - 1] = -1;
 
      btree_write(filho, t);
      btree_node_free(filho);
      btree_write(no, t);
      btree_node_free(no);
      btree_write(*node, t);
      btree_node_free(*node);
 
      // checar underflow
      if (node->qtd_chaves < (ceil(btree_order / 2) - 1)) {
        int pos_chave_pai;
        btree_node raiz = btree_read(t->rrn_raiz, t);
        btree_node pai = btree_father_search(&pos_chave_pai, node->chaves[i], t->rrn_raiz, t, &raiz);
        return btree_borrow_or_merge(&pai, pos_chave_pai, t);
      }
 
      return true;
    }
  }
}
 
bool btree_search(char *result, bool exibir_caminho, char *chave, int rrn, btree *t) {
  int pos;
 
  if (rrn == -1)
    return false;
 
  btree_node aux = btree_read(rrn, t);
 
  if (exibir_caminho)
    printf(" %d", rrn);
 
  if (btree_binary_search(&pos, exibir_caminho, chave, &aux, t) == true) {
    if (result)
      strncpy(result, aux.chaves[pos], t->tam_chave);
 
    btree_node_free(aux);
    if (exibir_caminho)
      printf("\n");
 
    return true;
  }
 
  if (aux.folha == true) {
    btree_node_free(aux);
 
    if (exibir_caminho)
      printf("\n");
 
    return false;
  } else {
    rrn = aux.filhos[pos];
    btree_node_free(aux);
    return btree_search(result, exibir_caminho, chave, rrn, t);
  }
}
 
// na chamada da função, pai deve ser o nó raíz
btree_node btree_father_search(int *pos_chave, char *chave, int rrn, btree *t, btree_node *pai) {
  btree_node node = btree_read(rrn, t);
  int pos;
 
  if (rrn == -1)  // caso base
    return *pai;
 
  if (btree_binary_search(&pos, false, chave, &node, t) == true) {
    int i = 0;
 
    while (i < pai->qtd_chaves - 1) {  // devolve sempre a posicao da chave pai que é maior que a chave buscada,i.e. a chave buscada está entre pai->chaves[i] e pai->chaves[i-1]
      if (t->compar(chave, pai->chaves[i]) > 0) {
        i++;
      } else {
        break;
      }
    }
 
    *pos_chave = i;
    return *pai;
  }
 
  if (node.folha == true) {
    btree_node_free(node);
    *pos_chave = pos;
    return *pai;
 
  } else {
    rrn = node.filhos[pos];
    *pai = node;
    return btree_father_search(pos_chave, chave, rrn, t, pai);
  }
}
 
bool btree_binary_search(int *result, bool exibir_caminho, char *chave, btree_node *node, btree *t) {
  // bb padrao
  int start;
  int end;
  int mid;
 
  if (exibir_caminho) {
    printf(" ( ");
  }
  end = node->qtd_chaves - 1;
  start = 0;
 
  while (start <= end) {
    mid = (start + (end + 1)) / 2;
    if (exibir_caminho == true)
      printf("%d", mid);
 
    if (mid == -1)
      mid = 0;
 
    if (result != NULL)
      *result = mid;
 
    if (t->compar(chave, node->chaves[mid]) > 0) {
      start = mid + 1;
      *result = start;
 
    } else if (t->compar(chave, node->chaves[mid]) < 0) {
      end = mid - 1;
 
    } else {
      if (exibir_caminho)
        printf(")");
 
      *result = mid;
      return true;
    }
 
    if (exibir_caminho && start <= end)
      printf(" ");
  }
 
  if (exibir_caminho)
    printf(")");
 
  return false;
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_binary_search");
}
 
bool btree_print_in_order(char *chave_inicio, char *chave_fim, bool (*exibir)(char *chave), int rrn, btree *t) {
  btree_node node = btree_read(rrn, t);
  // vars de controle p chave inicio e chave fim
  int final = -1;
  int inicio = 1;
 
  bool imprimir_flag = false;
 
  if (rrn == -1) {  // se o rrn é -1, não há o que imprimir, caso base da recursao
    btree_node_free(node);
    return false;
  }
 
  if (node.folha == true) {
    for (int i = 0; i < node.qtd_chaves; i++) {
      if (chave_fim)
        final = t->compar(node.chaves[i], chave_fim);  // compara a chave atual com a chave_fim
 
      if (chave_inicio)
        inicio = t->compar(node.chaves[i], chave_inicio);  // compara a chave atual com a chave de início
 
      if (final > 0)  // se a chave atual for maior que a chave_fim, entao a impressão acabou
        break;
 
      else if (inicio >= 0 && final <= 0) {  // se estiver entre o começo e o fim, entao deve imprimir
        imprimir_flag = true;                // seta flag de retorno true, indicando que há o que imprimir, e exibe a chave
        node.chaves[i][t->tam_chave] = '\0';
        exibir(node.chaves[i]);
      }
    }
 
    btree_node_free(node);
    return imprimir_flag;
  }
 
  // se não é folha, chama recursivamente até chegar em folha
  btree_print_in_order(chave_inicio, chave_fim, exibir, node.filhos[0], t);
 
  imprimir_flag = false;  // flag é setada para false voltando da recursao
 
  for (int i = 0; i < node.qtd_chaves; i++) {  // faz a impressão do nó pai do que foi pra recursão, para ficar em ordem. Processo similar ao outro
    if (chave_fim)
      final = t->compar(node.chaves[i], chave_fim);
 
    if (chave_inicio)
      inicio = t->compar(node.chaves[i], chave_inicio);
 
    if (inicio < 0 || final > 0)
      break;
    node.chaves[i][t->tam_chave] = '\0';
    exibir(node.chaves[i]);
    imprimir_flag = true;
 
    if (node.filhos[i + 1] >= 0) {
      btree_print_in_order(chave_inicio, chave_fim, exibir, node.filhos[i + 1], t);
    }
  }
 
  btree_node_free(node);  // libera o nó auxiliar
  return imprimir_flag;   // retorna se tinha algo a ser imprimido
}
 
btree_node btree_read(int rrn, btree *t) {
  // auxiliar que vai guardar uma cópia da árvore
  char aux[btree_register_size(t) + 1];
  // nó auxiliar que será o retorno da funcao de acordo com rrn passado
  btree_node x = btree_node_malloc(t);
 
  // var temporária que será usada na atribuição de filhos ao nó
  char filhos[4];
  // variável que acompanha o tamanho do nó
  int n = 3;
  // var temporária que será usada para guardar a qtd de chaves (no arquivo, a chave é um char de tamanho 3)
  char key_qtd[4];
  // bool que guardará a informação sobre o nó ser folha ou não
  char folha[2];
 
  // pegando o nó dentro do arquivo da árvore e atribuindo à aux
  strncpy(aux, t->arquivo + (btree_register_size(t) * rrn), btree_register_size(t));
 
  // colocando o EOF no aux
  aux[btree_register_size(t)] = '\0';
 
  // atribuindo o rrn ao nó
  x.this_rrn = rrn;
 
  // atribuindo a qtd de chaves ao nó
  strncpy(key_qtd, aux, 3);
  key_qtd[3] = '\0';
  x.qtd_chaves = atoi(key_qtd);  // conversao
 
  // copiando as chaves do arquivo para o nó
  for (int i = 0; i < btree_order - 1; i++) {  // percorrer até ordem da arvore
    if (strncmp(n + aux + (t->tam_chave * i), "#", 1) == 0) {
      *x.chaves[i] = '\0';
    } else {
      strncpy(x.chaves[i], n + aux + (t->tam_chave * i), t->tam_chave);
    }
  }
 
  // recalcula o tamanho do nó para achar a posição que diz se é folha ou não
  n = n + ((btree_order - 1) * t->tam_chave);
  // copia o bool indicando se é folha ou não para a váriavel folha(da p fazer usando só um char ao inves de string)
  strncpy(folha, n + aux, 1);
  folha[1] = '\0';
 
  if (strncmp(folha, "F", 1) == 0) {  // verificar se é folha
    x.folha = false;
  } else {
    x.folha = true;
  }
  // aumentou o tamanho do nó em um
  n++;
 
  // arrumando os ponteiros
  for (int i = 0; i < btree_order; i++) {
    if (strncmp(n + aux + (3 * i), "*", 1) == 0) {  // verificar se existe filho
      x.filhos[i] = -1;
    } else {
      strncpy(filhos, n + aux + (3 * i), 3);  // informacao de filhos (filhos[] é uma var temp que guarda o rrn do filho sendo visitado por 'i')
      filhos[3] = '\0';
      int aux_filhos;
      aux_filhos = atoi(filhos);
      x.filhos[i] = aux_filhos;
    }
  }
  return x;
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_read");
}
 
void btree_write(btree_node no, btree *t) {
  // write faz o contrário do read
 
  // o que vai ser escrito
  char aux[btree_register_size(t)];
  aux[0] = '\0';
 
  // var auxiliar para o filho
  char aux_filho[4];
 
  // escreve no formato
  sprintf(aux, "%03d", no.qtd_chaves);
 
  for (int i = 0; i < btree_order - 1; i++) {  // percorrer até ordem da arvore
    if (i < no.qtd_chaves) {
      strncat(aux, no.chaves[i], t->tam_chave);  // copia a chave se ela existe
    } else {
      for (unsigned j = 0; j < t->tam_chave; j++) {
        strcat(aux, "#");  // se nao, insere os '#'
      }
    }
  }
 
  if (no.folha)  // inserir T ou F
    strcat(aux, "T");
  else
    strcat(aux, "F");
 
  for (int i = 0; i < btree_order; i++) {
    if (no.filhos[i] >= 0) {  // se tem filho, insere ele no aux//&& i < no.qtd_chaves
      sprintf(aux_filho, "%03d", no.filhos[i]);
      strcat(aux, aux_filho);
    } else {
      strcat(aux, "***");  // se nao tem, insere '*'
    }
  }
  strncpy(t->arquivo + no.this_rrn * btree_register_size(t), aux, btree_register_size(t));  // escreve aux no nó de rrn passado no arquivo da árvore
 
  t->arquivo[btree_register_size(t) * t->qtd_nos] = '\0';  // coloca '/0' no fim do arquivo
  // printf(ERRO_NAO_IMPLEMENTADO, "btree_write");
}
 
int btree_register_size(btree *t) {
  int chaves_ordenadas = (btree_order - 1) * t->tam_chave;
  return 3 + chaves_ordenadas + 1 + (btree_order * 3);
}
 
btree_node btree_node_malloc(btree *t) {
  btree_node no;
 
  no.chaves = malloc((btree_order - 1) * sizeof(char *));
  for (int i = 0; i < btree_order - 1; ++i) {
    no.chaves[i] = malloc(t->tam_chave + 1);
    no.chaves[i][0] = '\0';
  }
 
  no.filhos = malloc(btree_order * sizeof(int));
  for (int i = 0; i < btree_order; ++i)
    no.filhos[i] = -1;
 
  return no;
}
 
void btree_node_free(btree_node no) {
  for (int i = 0; i < btree_order - 1; ++i)
    free(no.chaves[i]);
 
  free(no.chaves);
  free(no.filhos);
}
 
char *strpadright(char *str, char pad, unsigned size) {
  for (unsigned i = strlen(str); i < size; ++i)
    str[i] = pad;
  str[size] = '\0';
  return str;
}
 
char *strupr(char *str) {
  for (char *p = str; *p; ++p)
    *p = toupper(*p);
  return str;
}
 
char *strlower(char *str) {
  for (char *p = str; *p; ++p)
    *p = tolower(*p);
  return str;
}