# grupOS

## About

The project's objective is to perform the md5sum hashing algorithm on multiple files at the same time taking advantage of multiprocessing.
It consists of three programs, the application, the worker and the viewer.

The application handles the creation of worker processes that run simultaneously as children.
The application sends through pipes the path of the files and the workers perform the hash, sending it back through another pipe.

To view the results live, the viewer application reads from a shared memory buffer, and it prints it through standard output.

## Prerequisites

The compilation of the program is done in the agodio/itba-so:1.0 docker image.
To install the image use:

```bash
$ docker pull agodio/itba-so:1.0
```

## Compilation

1. Clone git repository or extract zip with project:

```bash
$ git clone https://github.com/Santiago-Rivas/grupOS
```

Or

```bash
$ unzip grupOS.zip
```

2. Change into the project's directory

```bash
$ cd grupOS
```

3. Compile:

```bash
$ make all
```

Each program can be compiled separately using make and the name of the program.

### Worker

To make the worker run the following command:

```bash
$ make worker
```

### Application

To make the application run the following command:

```bash
$ make app
```

Note: To run this program, the worker must be compiled

### Viewer

To make the viewer run the following command:

```bash
$ make viewer
```

### All

To make the worker, the application and the viewer run the following command:

```bash
$ make all
```

### Clean

To remove all binary files run the following command:

```bash
$ make clean
```

## Usage

All the programs run from the main project directory (Directory where the project was compiled)

### Worker

The worker performs the md5sum hashing by forking processes.
It will receive from standard input an existing file name and it will return its process id, the hash of the file and the file path through standard output.
The program will stop once it receives an end of file (CTRL + D)

The program will run with the following command:

```bash
$ ./bin/worker
```

#### Errors

When the worker receives the path of a file that does not exist, it will show an error message and it will return through standard output its process id, 32 characters of the letter 'X' and the file path that was not found.
The worker program will not finish, it will continue to receive through standard input file paths and it will function as normal until the EOF signal is received.

| Name       | Value | Description                             |
|------------|-------|-----------------------------------------|
| RW_ERROR   | 1     | Error when reading or writing to a file |

### Application

The application will receive by parameter the list of all the file paths for it to calculate the md5sum hash.
The application will print though standard output a code that the viewer process will require to view the shared memory of the application program.
It will wait a set amount of time (2 seconds) for a viewer program to start.
In any case after set time, it will continue running.
The application will finish once all the file paths are processed by worker applications.
If a viewer application was opened and is reading the shared memory, the application will wait until it finishes.
An output.txt will be created and each line will show the pid of the worker that performed the hash, the hash and the file path for which the hash was performed.

The program will run with the following command:

```bash
$ ./bin/application [list_of_file_path]
```

#### Example

```bash
$ ./bin/application BIG/*
```

#### Return

The application will return 0 if everything happened as expected.
It will return a non zero value if an error occurred.

#### Errors

| Name            | Value | Description                                         |
|-----------------|-------|-----------------------------------------------------|
| PARAMETER_ERROR | 1     | Wrong number of input parameters                    |
| MEMORY_ERROR    | 2     | Error when allocating memory or when creating pipes |
| SELECT_ERROR    | 3     | Select function error                               |
| RW_ERROR        | 4     | Error when reading or writing to a file             |
| FILE_ERROR      | 5     | Error when opening a file                           |

### Viewer

The viewer program reads shared memory created by the application and it will print it to standard output.
To connect to said shared memory, it requires the code printed by the application.
This code may be received as a parameter or through standard input.
The viewer will print all of the results even if they were processed before it started running.
The viewer will run until each result is printed.

The program will run with the following command:

```bash
$ ./bin/viewer [code]
```

Or

```bash
$ ./bin/viewer
```
Note: In this case it will read from standard input for the code

#### Example

In this example the code printed by the application is piped into the viewer standard input.

```bash
$ ./bin/application BIG/* | ./bin/viewer
```

#### Return

The viewer will return 0 if everything happened as expected.
It will return a non zero value if an error occurred.

#### Errors

| Name            | Value | Description                             |
|-----------------|-------|-----------------------------------------|
| PARAMETER_ERROR | 1     | Wrong number of input parameters        |
| MEMORY_ERROR    | 2     | Error when allocating memory            |

## Authors

[Perri Lucas](https://github.com/lperri5)

[Panighini Franco](https://github.com/fpanighini)

[Rivas Santiago](https://github.com/Santiago-Rivas)


















