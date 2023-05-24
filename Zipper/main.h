#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <string.h>

#include "metadata.h"

#define BUFFER_SIZE 4096

Metadata getFileMetadata(char *filePath)
{
    // get file stats
    struct stat st;
    stat(filePath, &st);

    // get file name
    char *fileName = strrchr(filePath, '/');
    if (fileName == NULL)
    {
        fileName = filePath;
    }
    else
    {
        fileName++;
    }

    // get file permissions
    char permissions[] = "----------";
    permissions[0] = (S_ISDIR(st.st_mode)) ? 'd' : '-';
    permissions[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

    // get permission mode
    mode_t mode = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    // get file owner and group
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);

    // create metadata
    Metadata metadata = createMetadata(st.st_size, fileName, filePath, permissions, mode, pw->pw_name, gr->gr_name, st.st_uid, st.st_gid);
    return metadata;
}

void archive(char *filePath, char **store, unsigned long *storeSize, Metadata **mData, int *mDataSize)
{
    unsigned char path[BUFFER_SIZE];

    // get file metadata
    Metadata metadata = getFileMetadata(filePath);

    // ignore . and .. directories
    if (strcmp(metadata.file_name, ".") == 0 || strcmp(metadata.file_name, "..") == 0)
    {
        return;
    }

    if (metadata.permissions[0] == 'd')
    {
        metadata.file_size = 1;
    }

    // memory allocation
    if (*storeSize == 0)
    {
        *store = (char *)malloc(sizeof(char) * metadata.file_size);
        strcpy(*store, "\0");
    }
    else
    {
        *store = (char *)realloc(*store, sizeof(char) * (*storeSize + metadata.file_size + 1));
    }

    if (*mDataSize == 0)
    {
        *mData = (Metadata *)malloc(sizeof(Metadata));
    }
    else
    {
        *mData = (Metadata *)realloc(*mData, sizeof(Metadata) * (*mDataSize + 1));
    }

    // write metadata to mData
    (*mData)[*mDataSize] = metadata;
    *mDataSize += 1;

    if (metadata.permissions[0] == 'd')
    {
        strcat(*store, "D");
    }
    else
    {
        char buffer[BUFFER_SIZE] ;
        size_t bytes_read;

        FILE *file = fopen(filePath, "rb");

        if (file == NULL)
        {
            printf("Error opening file\n");
            return;
        }

        // read and write data
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
        {
            strncat(*store, buffer, bytes_read);
        }

        memset(buffer, 0, BUFFER_SIZE);
        fclose(file);
    }

    *storeSize += metadata.file_size;

    struct dirent *dp;
    DIR *dir = opendir(metadata.path);

    // if not a directory, return
    if (dir == NULL)
    {
        closedir(dir);
        return;
    }

    while ((dp = readdir(dir)) != NULL)
    {
        // create path
        memset(path, 0, BUFFER_SIZE);
        strcpy(path, filePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        archive(path, store, storeSize, mData, mDataSize);
    }

    closedir(dir);
}

void create_archive(char *archive_file, char *file_list)
{
    FILE *fd = fopen(archive_file, "wb");

    char *store = NULL;
    unsigned long storeSize = 0;

    Metadata *mData;
    int mDataSize = 0;

    // parse file list
    char *token = strtok(file_list, ",");

    // Iterate File list
    while (token != NULL)
    {
        archive(token, &store, &storeSize, &mData, &mDataSize);
        token = strtok(NULL, ",");
    }

    // write to the archive_file
    fwrite(&storeSize, sizeof(unsigned long), 1, fd);
    fwrite(store, sizeof(char), storeSize, fd);
    fwrite(mData, sizeof(Metadata), mDataSize, fd);

    // free memory
    free(store);
    free(mData);

    fclose(fd);

    printf("Archive created successfully.\n");
}

void append_to_archive(char *archive_file, char *file_list)
{
    FILE *fd = fopen(archive_file, "rb+");

    // get store size
    unsigned long storeSize;
    fread(&storeSize, sizeof(unsigned long), 1, fd);

    // read prev metadata
    fseek(fd, storeSize, SEEK_CUR);

    Metadata *mData;
    int mDataSize = 0;

    Metadata metadata;
    while (fread(&metadata, sizeof(Metadata), 1, fd))
    {
        // memory allocation
        if (mDataSize == 0)
        {
            mData = (Metadata *)malloc(sizeof(Metadata));
        }
        else
        {
            mData = (Metadata *)realloc(mData, sizeof(Metadata) * (mDataSize + 1));
        }

        mData[mDataSize] = metadata;
        mDataSize++;
    }

    // read store
    char *store = (char *)malloc(sizeof(char) * storeSize);
    fseek(fd, sizeof(unsigned long), SEEK_SET);
    fread(store, sizeof(char), storeSize, fd);

    char *token = strtok(file_list, ",");

    // iterate file list and add to archive
    while (token != NULL)
    {
        archive(token, &store, &storeSize, &mData, &mDataSize);
        token = strtok(NULL, ",");
    }

    fclose(fd);

    fd = fopen(archive_file, "wb");

    // write appended data to the archive_file
    fwrite(&storeSize, sizeof(unsigned long), 1, fd);
    fwrite(store, sizeof(char), storeSize, fd);
    fwrite(mData, sizeof(Metadata), mDataSize, fd);

    // free memory
    free(store);
    free(mData);

    fclose(fd);

    printf("Appended to archive successfully.\n");
}

void extract(char *archive_file)
{
    unsigned long storeReadSize = 0;
    int metadataReadSize = 0;

    FILE *fd = fopen(archive_file, "rb");

    // get store size
    unsigned long storeSize;
    fread(&storeSize, sizeof(unsigned long), 1, fd);

    // go to the start of metadata
    fseek(fd, storeSize, SEEK_CUR);

    Metadata metadata;
    while (fread(&metadata, sizeof(Metadata), 1, fd))
    {
        metadataReadSize++;
        if (metadata.permissions[0] == 'd')
        {
            // create directories
            char *command = (char *)malloc(sizeof(char) * (strlen(metadata.path) + 9));
            strcpy(command, "mkdir -p ");
            strcat(command, metadata.path);
            system(command);
            storeReadSize++;

            free(command);
        }

        else
        {
            // reading from store
            fseek(fd, storeReadSize + sizeof(unsigned long), SEEK_SET);

            FILE *file = fopen(metadata.path, "wb");
            char buffer[metadata.file_size];
            size_t bytes_read;

            // read and write data
            fread(buffer, 1, metadata.file_size, fd);
            fwrite(buffer, sizeof(char), metadata.file_size, file);
            memset(buffer, 0, metadata.file_size);

            storeReadSize += metadata.file_size;

            fclose(file);

            // set permissions
            chmod(metadata.path, metadata.mode);
            chown(metadata.path, metadata.uid, metadata.gid);
        
            fseek(fd, storeSize + sizeof(unsigned long) + sizeof(Metadata) * metadataReadSize, SEEK_SET);
        }
    }

    fclose(fd);

    printf("Extracted archive successfully.\n");
}

void print_metadata(char *archive_file)
{
    // reading and printing metadata
    FILE *fd = fopen(archive_file, "rb");

    unsigned long storeSize;
    fread(&storeSize, sizeof(unsigned long), 1, fd);

    fseek(fd, storeSize, SEEK_CUR);

    Metadata metadata;
    while (fread(&metadata, sizeof(Metadata), 1, fd))
    {
        printMetadata(metadata);
    }

    fclose(fd);
}

void print_hierarchy(char *archive_file)
{
    // reading and printing hierarchy
    FILE *fd = fopen(archive_file, "rb");

    unsigned long storeSize;
    fread(&storeSize, sizeof(unsigned long), 1, fd);

    fseek(fd, storeSize, SEEK_CUR);

    Metadata metadata;
    while (fread(&metadata, sizeof(Metadata), 1, fd))
    {
        printf("%s\n", metadata.path);
    }

    fclose(fd);
}

#endif