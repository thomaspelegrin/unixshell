////////////////////////////////////////////////////////////////////////////////
/// Simulates a shell 
/// @author Thomas Pelegrin
/// @date 10.19.2023
////////////////////////////////////////////////////////////////////////////////

/***************************** Imports ****************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

/**************************** Constants ***************************************/

/// @brief Allocates memory for a new argument
/// @param text the text that belongs to this argument
/// @return a pointer to the argument memory section
arg_t * create_arg(string_t text)
{
    arg_t * out_arg_p = NULL;
    int length = strlen( text );

    if (!(out_arg_p = (arg_t *) malloc( sizeof( arg_t ) ) ))
    {
        PRINT_ERROR( "malloc failed" );
    } else 
    {
        if (!(out_arg_p->text = (string_t) malloc( sizeof( char ) * length ) ))
        {
            free( out_arg_p );
            out_arg_p = NULL;
            PRINT_ERROR( "malloc failed" );
        } else 
        {
            strcpy( out_arg_p->text, text );
            out_arg_p->next = NULL;
        }
    }
    return out_arg_p;
}

/// @brief Frees all of the arguments and their strings
/// @param arg_pp the argument to free
void free_args(arg_t ** arg_pp)
{
    if (*arg_pp)
    {
        if ((*arg_pp)->text)
        {
            free( (*arg_pp)->text );    // free the arg string
            (*arg_pp)->text = NULL;
        }
        free_args( &(*arg_pp)->next );  // recursively free each argument
        free( *arg_pp );            // free the structure itself
        *arg_pp = NULL;         // set to NULL for safety
    }
}

/// @brief Allocates memory for a new command
/// @param out_cmd_pp the command to allocate for
/// @return 0 if SUCCESS, else 1 for ERROR
int create_cmd(cmd_t ** out_cmd_pp)
{
    int result = SUCCESS;

    if (!(*out_cmd_pp = (cmd_t *) malloc( sizeof( cmd_t ) )))
    { 
        result = ERROR;
        PRINT_ERROR( "malloc failed" );

    } else
    { /* initialize the cmd members */
        (*out_cmd_pp)->head = NULL;
        (*out_cmd_pp)->next = NULL;
        (*out_cmd_pp)->fd[ 0 ] = (*out_cmd_pp)->fd[ 1 ] = -1;
        (*out_cmd_pp)->argc = 1;
        (*out_cmd_pp)->handler_flags = 0;
    }
    return result;
}

/// @brief Adds an argument string to a command
/// @param cmd_p the command this argument belongs to
/// @param text the text of the argument
/// @return 0 if SUCCESS, else 1 for ERROR
int add_arg_to_cmd(cmd_t * cmd_p, string_t text) 
{
    arg_t * arg_p = NULL;
    
    if (!cmd_p->head)
    { /* create the first argument for this command */
        cmd_p->head = create_arg( text );

    } else 
    { /* add the argument to the command */
        arg_p = cmd_p->head;
        while (arg_p->next) 
        { /* loop through each argument until an open spot is found */
            arg_p = arg_p->next;
        }
        arg_p->next = create_arg( text );
    }
    cmd_p->argc++;

    return SUCCESS;
}

/// @brief Frees all of the commands and their arguments
/// @param cmd_pp the command to free
void free_cmd(cmd_t ** cmd_pp)
{
    if (*cmd_pp)
    {
        if ((*cmd_pp)->head)
        {
            free( (*cmd_pp)->head );    // free the command's arguments 
        }
        free_cmd( &(*cmd_pp)->next );   // recursively free each of the cmds
        free( *cmd_pp );            // free the cmd struct itself
        *cmd_pp = NULL;         // set to NULL for safety
    }
}

/// @brief Allocates memory for a new command set
/// @param out_cmd_set_pp the command set to allocate for
/// @return 0 if SUCCESS, else 1 for ERROR
int create_cmd_set(cmd_set_t ** out_cmd_set_pp)
{
    int result = SUCCESS;

    if (!(*out_cmd_set_pp = (cmd_set_t *) malloc( sizeof( cmd_set_t ) )))
    {
        result = ERROR;
        PRINT_ERROR( "malloc failed" );

    } else
    { /* initialize the cmd set members */
        (*out_cmd_set_pp)->head = NULL;
        (*out_cmd_set_pp)->async = FALSE;
    }
    return result;
}

/// @brief Frees the command set, commands, and arguments associated with it
/// @param cmd_set_pp the command set to free
void free_cmd_set(cmd_set_t ** cmd_set_pp)
{
    if (*cmd_set_pp)
    {
        if ((*cmd_set_pp)->head)
        {
            free_cmd( &(*cmd_set_pp)->head );   // free the cmd set's commands
        }
        free( *cmd_set_pp );    // free the cmd_set struct itself
        *cmd_set_pp = NULL;     // set to NULL for safety
    }
}

/// @brief A function defined to help with the debugging MACROS in header
/// @param arg1 custom string argument
/// @param f_name the name of the function
/// @param ln_num the line number
/// @param sentinel a sentinel (to make line easy to spot in output)
void print_debug(char * arg1, const char * f_name, int ln_num, char * sentinel)
{
    #ifdef DEBUG_MODE
        #if DEBUG_MODE == 1
            printf( "\n\t %s: [%s] (Ln.%d) pid=%d\t%s \n", arg1, f_name,
                    ln_num, getpid(), sentinel );
        #endif
    #endif
}