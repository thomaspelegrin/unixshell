////////////////////////////////////////////////////////////////////////////////
/// Simulates a shell 
/// @author Thomas Pelegrin
/// @date 10.19.2023
////////////////////////////////////////////////////////////////////////////////

/***************************** Imports ****************************************/

#include <stdio.h>

/**************************** Constants ***************************************/

/* 1 = debug mode enabled, disabled otherwise */
#define DEBUG_MODE 0

#define TRUE 1
#define FALSE 0
#define MAX_INPUT_SIZE 100
#define SUCCESS 0
#define ERROR 1

#define READ_END	0
#define WRITE_END	1

#define R_PIPE 0b001
#define W_PIPE 0b010
#define W_FILE 0b100

/***************************** Macros *****************************************/

void print_debug(char * arg1, const char * f_name, int ln_num, char * sentinel);

#define PRINT_ERROR(s) printf( "\t# [ERROR: %s (Ln.%d) - %s] #\n", __func__, __LINE__, (s) );

/* Prints the function name at the start of the function */
#define START_FUNC print_debug( "START", __func__, __LINE__, "+" )
/* Prints the function name at the end of the function */
#define END_FUNC print_debug( "END", __func__, __LINE__, "-" )
/* Prints a test message with function name and line number */
#define TEST print_debug( "\tTEST", __func__, __LINE__, "***" )

/*******************************************************************************
 *                       Type and Struct Definitions
 ******************************************************************************/

typedef char * string_t;
typedef struct arg_s arg_t;
typedef struct cmd_s cmd_t;
typedef struct cmd_set_s cmd_set_t;

struct arg_s 
{ /* argument structure (a linked list) */
    string_t text; 
    struct arg_s * next;
};

struct cmd_s 
{ /* command structure (a linked list) */
    arg_t * head; 
    short argc; 
    int fd[ 2 ];
    char handler_flags;
    struct cmd_s * next; 
};

struct cmd_set_s 
{ /* command set struct -- an auxiliary wrapper structure */
    cmd_t * head;
    char async;
};

/*******************************************************************************
 *                          Public Functions
 ******************************************************************************/

arg_t * create_arg(string_t);
int add_arg_to_cmd(cmd_t *, string_t);
void free_args(arg_t **);

int create_cmd(cmd_t **);
void free_cmd(cmd_t **);

int create_cmd_set(cmd_set_t **);
void free_cmd_set(cmd_set_t **);