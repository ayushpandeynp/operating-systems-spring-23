#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
#include "metadata.h"

int main(int argc, char *argv[])
{
    // parsing command line argumments
    if (argc < 3)
    {
        printf("Usage: ./adzip {-c|-a|-x|-m|-p} <archive-file> <file/directory list>\n");
        return EXIT_FAILURE;
    }

    char *flag = argv[1];
    char *archive_file = argv[2];
    char *file_list = argv[3];

    if (strcmp(flag, "-c") == 0)
    {
        // create archive
        create_archive(archive_file, file_list);
    }
    else if (strcmp(flag, "-a") == 0)
    {
        // append to archive
        append_to_archive(archive_file, file_list);
    }
    else if (strcmp(flag, "-x") == 0)
    {
        // extract from archive
        extract(archive_file);
    }
    else if (strcmp(flag, "-m") == 0)
    {
        // print metadata
        print_metadata(archive_file);
    }
    else if (strcmp(flag, "-p") == 0)
    {
        // print hierarchy
        print_hierarchy(archive_file);
    }
    else
    {
        printf("Usage: ./adzip {-c|-a|-x|-m|-p} <archive-file> <file/directory list>\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}