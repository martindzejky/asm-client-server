# Client-server application in system C

The task was to make a client-server application in system C and inline assembler.
The client sends commands to the server, the server evaluates them, and sends the
result back. The program was written to work on my Mac. Almost all the code should
be portable, except for the inline assembler and functions querying the memory usage.

## Files

The codebase is nicely split into multiple files:

- Constants contains constant values used throughout the whole codebase.
- Options parses the command line arguments into a nice `struct`.
- Prompt handles the prompt and reading user input.
- Client contains client code. It sends user input, receives the output, and prints it.
- Server contains server code. It forks processes for newly connected clients. Input data is send
to the interpreter and the result is send back to clients. Also, `stdin` and `stdout` is working as well.
- Interpreter interprets the commands passed to it. This is the main part of the program (and the not portable one).

## Help

For help about the program, use the `-h` command line argument. For the
list of usable commands, run `help`. If a command is not recognized, the program
tries to evaluate it using the Shell.
