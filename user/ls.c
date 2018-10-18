/*
 * An example demonstrating basic directory listing.
 *
 * Compile this file with Visual Studio and run the produced command in
 * console with a directory name argument.  For example, command
 *
 *     ls "c:\Program Files"
 *
 * might output something like
 *
 *     ./
 *     ../
 *     7-Zip/
 *     Internet Explorer/
 *     Microsoft Visual Studio 9.0/
 *     Microsoft.NET/
 *     Mozilla Firefox/
 *
 * The ls command provided by this file is only an example: the command does
 * not have any fancy options like "ls -al" in Linux and the command does not
 * support file name matching like "ls *.c".
 *
 * Copyright (C) 2006-2012 Toni Ronkko
 * This file is part of dirent.  Dirent may be freely distributed
 * under the MIT license.  For all details and documentation, see
 * https://github.com/tronkko/dirent
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define	DT_UNKNOWN	 0
#define	DT_FIFO		 1
#define	DT_CHR		 2
#define	DT_DIR		 4
#define	DT_BLK		 6
#define	DT_REG		 8
#define	DT_LNK		10
#define	DT_SOCK		12
typedef long int off_t;
typedef   unsigned short ino_t;
struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* offset to the next dirent */
    unsigned short d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported
                                   by all file system types */
    char           d_name[256]; /* filename */
};

typedef struct DIR {
	int fd;
	int cur_entry;
} DIR;


static void list_directory (const char *dirname);


int
main(
    int argc, char *argv[])
{
    int i;

    /* For each directory in command line */
    i = 1;
    while (i < argc) {
        list_directory (argv[i]);
        i++;
    }

    /* List current working directory if no arguments on command line */
    if (argc == 1) {
        list_directory (".");
    }
    return EXIT_SUCCESS;
}

/*
 * List files and directories within a directory.
 */
static void
list_directory(
    const char *dirname)
{
    DIR *dir;
    struct dirent *ent;

    /* Open directory stream */
    dir = opendir (dirname);
    if (dir != NULL) {

        /* Print all files and directories within the directory */
        while ((ent = readdir (dir)) != NULL) {
            switch (ent->d_type) {
            case DT_REG:
                printf ("   %s", ent->d_name);
                break;

            case DT_DIR:
                printf ("   %s", ent->d_name);
                break;

            case DT_LNK:
                printf ("   %s@", ent->d_name);
                break;

            default:
                printf ("   %s", ent->d_name);
            }
        }
 printf ("\n");
        closedir (dir);

    } else {
        /* Could not open directory */
        printf ("Cannot open directory %s\n", dirname);
        exit (EXIT_FAILURE);
    }
}

