#ifndef FS_H_
#define FS_H_

#include <stdio.h>

/* buf should be at least as large as strlen(path) + 1
 * if buf is null, or buf points to path, will do in-place replacement
 * only UNIX-style path separators are supported
 * returns buf
 */
char *path_dir(const char *path, char *buf);
char *path_file(const char *path, char *buf);

/* buf should be at least strlen(dirname) + strlen(fname) + 2
 * returns buf
 */
char *combine_path(const char *dirname, const char *fname, char *buf);

/* if dirname is null or empty, this is equivalent to fopen(fname, attr) */
FILE *fopenat(const char *dirname, const char *fname, const char *attr);

#endif	/* FS_H_ */
