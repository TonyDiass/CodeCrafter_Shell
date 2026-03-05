[![progress-banner](https://backend.codecrafters.io/progress/shell/0839e248-79ad-416e-b01a-9ec9d0fd38d6)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)



# Custom POSIX Shell in C



This repository contains my custom implementation of a POSIX-compliant shell in C, built as part of the [CodeCrafters "Build Your Own Shell" challenge](https://app.codecrafters.io/courses/shell/overview). 

---



## 🚀 Features



This custom shell implementation currently supports the following functionalities:



### 1. Built-in Commands

The shell natively handles the following commands without spawning external processes:

- `exit`: Gracefully terminates the shell.

- `echo`: Prints arguments to standard output.

- `pwd`: Displays the current working directory.

- `cd`: Changes the current directory (supports relative/absolute paths and `~` for HOME).

- `type`: Identifies whether a command is a built-in or an external executable.



### 2. External Program Execution

- Parses the `$PATH` environment variable to locate and execute external binaries (e.g., `ls`, `cat`).

- Uses `fork()` and `execv()` to run programs in child processes while the main shell waits.



### 3. Advanced Input Parsing

- **Quotes Support:** Accurately parses arguments enclosed in single quotes (`'...'`) and double quotes (`"..."`).

- **Escape Characters:** Handles the backslash (`\`) escape character inside and outside of quotes.



### 4. Output Redirection

- Supports redirecting `stdout` (`1>`) and `stderr` (`2>`) to files.

- Handles truncation (`>`) and append mode (`>>`).







