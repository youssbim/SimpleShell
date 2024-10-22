# Simple Shell (dsh)

This project implements a simple Unix shell in C, called `dsh`. The shell supports executing commands, managing history, redirecting output, and using pipes for inter-process communication.

## Features

- **Command Execution**: You can execute system commands directly from the shell.
- **History Management**: Executed commands are stored, allowing you to access your command history.
- **Output Redirection**: Supports redirecting output to files.
- **Pipes**: Supports using pipes to connect the output of one command to the input of another.

## Functionality

### Supported Commands

- **Execute Commands**: Enter the desired command and press Enter to execute it.
  
  Example:
  ```
  ls -l
  ```

- **Exit the Shell**: To exit the shell, you can use the `exit` command.

  Example:
  ```
  exit
  ```

- **Set the Path**: You can change the command search path using the `setpath` command.

  Example:
  ```
  setpath /usr/local/bin
  ```

### Output Redirection

You can redirect the output of a command to a file using the `>` symbol for writing and `>>` for appending.

Examples:
```
echo "Hello World" > output.txt   # Writes "Hello World" to output.txt
echo "Hello again" >> output.txt   # Appends "Hello again" to output.txt
```

### Pipes

You can use the `|` symbol to connect the output of one command to the input of another.

Example:
```
ls -l | grep ".c"   # Displays only files with a .c extension
```

## Installation

To compile and use `dsh`, ensure you have a C compiler installed. You can use `gcc` to compile the program.

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```

2. Compile the program:
   ```bash
   gcc -o dsh dsh.c -lreadline
   ```

3. Run the shell:
   ```bash
   ./dsh
   ```

## Requirements

- A Unix-like system (Linux or macOS).
- `readline` library for input management.

## Usage Examples

1. Execute a simple command:
   ```bash
   dsh$ ls -l
   ```

2. Set a new path:
   ```bash
   dsh$ setpath /usr/local/bin
   ```

3. Redirect output to a file:
   ```bash
   dsh$ echo "Hello World" > hello.txt
   ```

4. Use a pipe:
   ```bash
   dsh$ ps | grep "bash"
   ```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Acknowledgments

- Thanks to the C developer community for support and resources.
