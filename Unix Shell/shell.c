////////////////////////////////////////////////////////////////////////////////
/// Simulates a shell 
/// @author Thomas Pelegrin
/// @date 10.19.2023
////////////////////////////////////////////////////////////////////////////////

/***************************** Imports ****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "util.h"

/**************************** Constants ***************************************/

#define HIST_SIZE 5

/*******************************************************************************
 *                            Functions
 ******************************************************************************/

/// @brief Extracts an argument from the input and adds it to the command
/// @param cmd_p the command to add the argument to
/// @param in_buf the input buffer
/// @param len the length of the argument text
void extract_arg(cmd_t * cmd_p, string_t in_buf, int len)
{
    START_FUNC;

    char arg_str[ len + 1 ];        // temp argument string

    /* copys the arg string and terminates it */
    strncpy( arg_str, in_buf, len );
    arg_str[ len ] = '\0';

    /* add_arg will allocate for a for and make a copy of temp */
    add_arg_to_cmd( cmd_p, arg_str );

    END_FUNC;
}

/// @brief Extracts the commands from input and adds them to cmd_set
/// @param in_buf the input buffer
/// @param cmd_set_pp the command set to add commands to
void extract_cmds(string_t in_buf, cmd_set_t ** cmd_set_pp)
{
    START_FUNC;

    int length = 0;
    int i = 0;
    cmd_t * curr_cmd_p = NULL;

    create_cmd( &curr_cmd_p );      // allocate for a a new command 
    (*cmd_set_pp)->head = curr_cmd_p;   // add the new cmd to cmd set

    while (in_buf[ i ] != '\0')
    {
        if (in_buf[ i ] == ' ' || in_buf[ i ] == '&')
        { /* handles the delimiter and the async flag */
            if (in_buf[ i ] == '&') 
            { /* set the async flag for the command set */
                (*cmd_set_pp)->async = TRUE;

            } else if (length > 0)
            { /* add the argument to the command */
                extract_arg( curr_cmd_p, &in_buf[ i - length ], length );
            }
            length = 0;

        } else if (in_buf[ i ] == '|')
        { /* setup the pipe between the two commands */
            curr_cmd_p->handler_flags |= W_PIPE;    // set the write pipe flag

            create_cmd( &curr_cmd_p->next );    // allocate new cmd for reading

            curr_cmd_p = curr_cmd_p->next;          // move to the next command
            curr_cmd_p->handler_flags |= R_PIPE;    // set the read pipe flag

            length = 0;

        } else if (in_buf[ i ] == '>') 
        { /* set the write file flag for the command */
            curr_cmd_p->handler_flags |= W_FILE;
            length = 0;

        } else if (in_buf[ i ] == '\n')
        { /* add the last argument to the command */
            if (length > 0 || i == 1)
            {
                extract_arg( curr_cmd_p, &in_buf[ i - length ], length );
            }
            length = 0;

        } else 
        {
            length++;
        }
        i++;
    }
    END_FUNC;
}

/// @brief Converts a commands arguments to array of strings
/// @param cmd_p the command whose arguments are wanted
/// @param out_args the variable to assign the array to
void args_to_array(cmd_t * cmd_p, string_t * out_args)
{
    START_FUNC;

    arg_t * curr = cmd_p->head;
    int i = 0;

    while (curr && i < cmd_p->argc )
    { /* while curr isn't NULL and i < length */
        out_args[ i ] = curr->text;
        curr = curr->next;
        i++;
    }
    out_args[ i ] = NULL;

    END_FUNC;
}

/// @brief Prints the command sets in history
/// @param hist the history array
/// @param cmd_num the number of commands executed so far
void print_history(cmd_set_t * hist[], int cmd_num)
{
    START_FUNC;

    cmd_set_t * curr_cmd_set_p = NULL;
    cmd_t * curr_cmd_p = NULL;
    arg_t * arg = NULL;

    for (int i = cmd_num >= HIST_SIZE ? HIST_SIZE : cmd_num; i > 0; i--)
    { /* i = cmd or HIST_SIZE depending on how many cmds are in hist */
        printf( "%d. ", i );
        curr_cmd_set_p = hist[ (cmd_num - i) % HIST_SIZE ];
        curr_cmd_p = curr_cmd_set_p->head;

        while (curr_cmd_p)
        { /* walks through each of the commands */
            arg = curr_cmd_p->head;

            while (arg)
            { /* walks through each of the arguments and prints them */
                if (curr_cmd_p->handler_flags & W_FILE && !arg->next)
                { /* print > for file write at 2nd to last argument */
                    printf( "> " );
                }
                printf( "%s ", arg->text );
                arg = arg->next;
            }

            if (curr_cmd_p->handler_flags & (R_PIPE | W_PIPE) 
                && curr_cmd_p->next)
            { /* write the pipe operator in between commands */
                printf( "| " );
            }
            curr_cmd_p = curr_cmd_p->next;
        }
        if (curr_cmd_set_p->async)
        {
            printf( "&" );
        }
        printf( "\n" );
    }
    END_FUNC;
}

/// @brief Fetches the command set from history
/// @param in_buf the input buffer
/// @param hist the command set history array
/// @param cmd_num the command number
/// @param out_p where to output the command after its found (output)
/// @return 0 (SUCCESS) if found, else 1 (ERROR) if not found
int fetch_cmd_set(string_t in_buf, cmd_set_t * hist[], int cmd_num, 
    cmd_set_t ** out_p)
{
    START_FUNC;

    int result = SUCCESS;
    int choice = (cmd_num - (in_buf[ 2 ] - '0')) % HIST_SIZE;

    if (choice >= HIST_SIZE || choice < 0 || !hist[ choice ]
        || (in_buf[ 3 ] >= '0' && in_buf[ 3 ] <= '9'))
    { /* ensures that user's choice is within bounds and not null */
        printf( "Invalid choice!\n" );
        result = ERROR;
    } else {
        *out_p = hist[ choice ];
    }
    END_FUNC;

    return result;
}

/// @brief Returns the number of occurrences of a command set in history
/// @param hist the history array
/// @param cmd_p the command set you are checking for duplicates of
/// @return number of occurrences of a command set in history
int cmd_set_occurrences(cmd_set_t * hist[], cmd_set_t * cmd_p)
{
    START_FUNC;

    int count = 0;

    for (int i = 0; i < HIST_SIZE; i++)
    {
        if (cmd_p == hist[ i ])
        {
            count++;
        }
    }
    END_FUNC;

    return count;
}

/// @brief Sets up the pipes required for processes to communicate
/// @param cmd_p the command to execute
void setup_pipes(cmd_t * cmd_p)
{ /* sets up pipes in the parent process */
    START_FUNC;

    int fd[ 2 ];

    if (!cmd_p->next && !(cmd_p->handler_flags & R_PIPE))
    { /* If the command has a successor command to pipe to */
        PRINT_ERROR( "Invalid pipe" );
        return;
    }

    if (cmd_p->handler_flags & W_PIPE)
    { /* redirects STDOUT to the write end of the pipe */
        pipe( fd );

        if (!(cmd_p->handler_flags & R_PIPE))
        { /* if the first pipe, add READ_END to cmd so it can be closed */
            cmd_p->fd[ READ_END ] = fd[ READ_END ];
        }
        cmd_p->fd[ WRITE_END ] = fd[ WRITE_END ];
        cmd_p->next->fd[ READ_END ] = fd[ READ_END ];
        cmd_p->next->fd[ WRITE_END ] = -1;
    }
    END_FUNC;
}

/// @brief Executes the command
/// @param cmd_p the command to execute
void exec_cmd(cmd_t * cmd_p)
{
    START_FUNC;

    int file = -1;
    string_t args[ cmd_p->argc ];
    args_to_array( cmd_p, args );

    /* A command will never read before writing command happens */
    if (cmd_p->handler_flags & R_PIPE)
    { /* redirects STDIN to the read end of the pipe */
        dup2( cmd_p->fd[ READ_END ], STDIN_FILENO );
    }

    if (cmd_p->handler_flags & W_PIPE)
    { /* redirects STDOUT to the write end of the pipe */
        close( cmd_p->fd[ READ_END ] );
        dup2( cmd_p->fd[ WRITE_END ], STDOUT_FILENO );
    }

    if (cmd_p->handler_flags & W_FILE) 
    { /* redirects STDOUT to the file specified */

        if (cmd_p->argc < 3)
        { /* there must be at least three commands to execute: prgm file NULL */
            PRINT_ERROR( "illegal syntax" );
            return;
        }
        file = open( args[ cmd_p->argc - 2 ], 
            O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU );

        close( STDOUT_FILENO );             // close STDOUT
        dup2( file, STDOUT_FILENO );        // redirect STDOUT to file

        args[ cmd_p->argc - 2 ] = NULL; // remove file from args
    }

    END_FUNC;

    if (execvp( args[ 0 ], args ) < 0)
    { /* added for clarity: if exec succeeds nothing will execute past exec */
        PRINT_ERROR( "program not found" );
        exit( ERROR );                  // close child if exec failed
    }
}

/// @brief Fork and execute the command in the child process
/// @param cmd_p the command to execute in child
/// @param prev_cmd_p the previously executed command
void fork_and_exec(cmd_t * cmd_p, cmd_t * prev_cmd_p) 
{
    START_FUNC;
    int pid;

    if (cmd_p->handler_flags & (W_PIPE | R_PIPE))
    { /* determine if pipes are needed */
        setup_pipes( cmd_p );       // setup any necessary pipes
    }
    pid = fork();

    if (pid < 0) 
    {
        PRINT_ERROR( "fork failed!" );
    } else if (pid == 0) 
    { /* child setups any file descriptors and execute the command */
        exec_cmd( cmd_p ); // does NOT return 
    } else            
    { /* parent closes the file descriptors */
        if (prev_cmd_p && prev_cmd_p->fd[ READ_END ] != -1)
        { /* close read end for the previous process */
            close( prev_cmd_p->fd[ READ_END ] );
        }
        if (cmd_p->next && cmd_p->fd[ WRITE_END ] != -1)
        { /* close write end for the current process unless its last */
            close( cmd_p->fd[ WRITE_END ] ); 
        }
    }
    END_FUNC;
}

/// @brief Executes each of the commands in the command set
/// @param cmd_set_p the command set
void exec_cmd_set(cmd_set_t * cmd_set_p)
{
    START_FUNC;

    cmd_t * curr_cmd_p = cmd_set_p->head;   // The currently executing cmd
    cmd_t * prev_cmd_p = NULL;              // The previously executed cmd

    while (curr_cmd_p)
    { /* performs exection of the current command */
        fork_and_exec( curr_cmd_p, prev_cmd_p ); // executes current command
        prev_cmd_p = curr_cmd_p;    // save cmd we can close the read end

        if (!(cmd_set_p->async))
        { /* parent does not wait if the cmd_set has the async flag set */
            wait( NULL );
        } // not waiting will cause prompt to disspear until program exit

        if (curr_cmd_p->next)
        { /* set the current command pointer to the next command */
            curr_cmd_p = curr_cmd_p->next;
        } else 
        { /* when there are no more commands left, then exit loop */
            curr_cmd_p = prev_cmd_p = NULL;
        }
    }
    END_FUNC;
}

/// @brief Simulates the execution of a shell
/// @return 0 if SUCCESS, else 1 for ERROR
int simulate_shell() 
{
    START_FUNC;
    
    int result = SUCCESS;
    int cmd_num = 0;                    // The number of commands executed
    int hist_index = 0;                 // The history index
    char in_buf[ MAX_INPUT_SIZE ];      // Input buffer
    cmd_set_t * hist[ HIST_SIZE ];      // The last 5 commands entered
    cmd_set_t * cmd_set_p = NULL;       // The current set of commands

    for (int i = 0; i < HIST_SIZE; i++)
    { /* initialize hist elements to NULL pointers */
        hist[ i ] = NULL;
    }

    while (!create_cmd_set( &cmd_set_p )) 
    { /* main loop: executes until quit or cmd set fails to allocate */
        printf( ">>" );

        if (!fgets( in_buf, sizeof( in_buf ), stdin ))
        { /* get the user input; if fgets fails, then break */
            result = ERROR;
            goto FUNC_EXIT;
        }

        if (in_buf[ 0 ] == 'q' && strstr( &in_buf[ 1 ], "uit" )) 
        { /* quit command -- checks char to be efficient in general case */
            goto FUNC_EXIT;

        } else if (in_buf[ 0 ] == '\n')
        { /* starts a new terminal line w/o executing any commands */
            continue;

        } else if (in_buf[ 0 ] == 'h' && strstr( &in_buf[ 1 ], "ist" ))
        { /* hist command -- checks char to be efficient in general case */
            print_history( hist, cmd_num );     // prints cmd set history
            continue;

        } else if (in_buf[ 0 ] == 'r' && in_buf[ 1 ] == ' ') 
        { /* r # - repeats a command in history, non-nums after # are ignored */
            if (fetch_cmd_set( in_buf, hist, cmd_num, &cmd_set_p))
            { /* if the user selects an invalid choice, go back to >> prompt */
                continue;
            }
        } else                                    // new command entered 
        { /* extract the arguments for normal execution */
            extract_cmds( in_buf, &cmd_set_p );
        }
        hist_index = cmd_num % HIST_SIZE;

        if (cmd_num >= HIST_SIZE && cmd_set_p != hist[ hist_index ]
            && cmd_set_occurrences( hist, hist[ hist_index ] ) <= 1)
        { /* if hist is full and isn't needed anymore, release its memory 
            -- we have to count the occurrences so we don't free a command set
            which is still pointed to in hist */
            free_cmd_set( &hist[ hist_index ] );
        }
        hist[ hist_index ] = cmd_set_p;
        cmd_num++;

        /* Execute each of the commands in the command set */
        exec_cmd_set( cmd_set_p );
    }
FUNC_EXIT:
    END_FUNC;

    return result;
}

/// @brief Main function for running the program
/// @param argc n/a
/// @param argv n/a
/// @return 0 if SUCCESS, else 1 for ERROR
int main(int argc, char *argv[])
{
    return simulate_shell();
}
