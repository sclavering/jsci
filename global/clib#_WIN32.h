//#define _DLL

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#pragma JSEXT dl "msvcr90.dll"

// These functions need a wrapper, as they are sometimes inlined

__declspec(dllexport) FILE *getstdin(void);
__declspec(dllexport) FILE *getstdout(void);
__declspec(dllexport) FILE *getstderr(void);
#define dup _dup
#define chdir _chdir
#define getcwd _getcwd
#define read _read
#define close _close
#define open _open
#define write _write
#define tell _tell
#define access _access
#define tempnam _tempnam
#define tmpnam _tmpnam
#define unlink _unlink
#define popen _popen
#define rmdir _rmdir
#define pclose _pclose
#define R_OK 04
#define W_OK 02
#define F_OK 00

#pragma JSEXT dl main // Functions are found in clib.c

