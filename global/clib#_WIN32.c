#include "clib#_WIN32.h"

__declspec(dllexport) FILE *getstdout(void) {
	return stdout;
}
__declspec(dllexport) FILE *getstderr(void) {
	return stderr;
}
__declspec(dllexport) FILE *getstdin(void) {
	return stdin;
}
