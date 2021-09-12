#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include "fs.h"

char *path_dir(const char *path, char *buf)
{
	char *sep;

	if(!buf) buf = (char*)path;
	if(path != buf) {
		strcpy(buf, path);
	}

	if((sep = strrchr(buf, '/')) && sep > buf) {
		*sep = 0;
	}
	return buf;
}

char *path_file(const char *path, char *buf)
{
	int len;
	char *sep;

	if(!buf) buf = (char*)path;
	if(path != buf) {
		strcpy(buf, path);
	}

	if((sep = strrchr(buf, '/'))) {
		len = strlen(sep + 1);
		memmove(buf, sep + 1, len + 1);
	}
	return buf;
}

char *combine_path(const char *dirname, const char *fname, char *buf)
{
	char *dest;

	if(!buf) return 0;

	if(!dirname || !*dirname) {
		strcpy(buf, fname);
		return buf;
	}

	dest = buf;
	while(*dirname) *dest++ = *dirname++;

	if(dest[-1] != '/') *dest++ = '/';

	strcpy(dest, fname);
	return buf;
}

FILE *fopenat(const char *dirname, const char *fname, const char *attr)
{
	char *buf;

	if(!dirname || !*dirname) {
		return fopen(fname, attr);
	}

	buf = alloca(strlen(dirname) + strlen(fname) + 2);
	combine_path(dirname, fname, buf);
	return fopen(buf, attr);
}
