# Project Readme

This readme file describes the usage and instructions for building the project. The project consists of three main executables: `reader`, `writer`, and `main`.

## Use Cases

### `reader`
The `reader` executable is used to read data from a specified file based on given parameters. The following command-line options are available:

```bash
./reader -f filename -l recid[,recid] -d time -s shmid -p pid
```

- `-f filename`: Specifies the name of the file to read data from.
- `-l recid[,recid]`: Specifies the student IDs to be read. Multiple record IDs can be provided, separated by commas.
- `-d time`: Specifies the time duration for which the data should be read (in seconds).
- `-s shmid`: Specifies the shared memory ID for accessing shared data.
- `-p pid`: Specifies the process ID of the coordinator process.

### `writer`
The `writer` executable is used to write data to a specified file based on given parameters. The following command-line options are available:

```bash
./writer -f filename -l recid -d time -s shmid -p pid
```


- `-f filename`: Specifies the name of the file to write data to.
- `-l recid`: Specifies the record ID to be written.
- `-d time`: Specifies the time for which the writing should be performed.
- `-s shmid`: Specifies the shared memory ID for accessing shared data.
- `-p pid`: Specifies the process ID of the coordinator process.

### `main`
The `main` executable is the main entry point of the project. It is the coordinator program that displays stats.

## Building the Project

To build the project, use the `make` command in the project directory. This assumes that the necessary build environment and dependencies are already installed on your system. The `make` command will compile the source code and generate the executable files.

```bash
make
```

Once the compilation process is complete, the executables (reader, writer, and main) will be available for use.