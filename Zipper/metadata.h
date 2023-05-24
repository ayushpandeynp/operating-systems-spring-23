#ifndef METADATA
#define METADATA

typedef struct Metadata Metadata;

struct Metadata
{
    long long file_size;
    char file_name[256];
    char path[4096];

    char permissions[11];
    mode_t mode;

    char owner[32];
    char group[32];

    uid_t uid;
    gid_t gid;
};

// function to create metadata
Metadata createMetadata(long long file_size, char *file_name, char *path, char *permissions, mode_t mode, char *owner, char *group, uid_t uid, gid_t gid)
{
    Metadata metadata;

    metadata.file_size = file_size;
    metadata.mode = mode;
    metadata.uid = uid;
    metadata.gid = gid;

    strcpy(metadata.file_name, file_name);
    strcpy(metadata.path, path);
    strcpy(metadata.permissions, permissions);    
    strcpy(metadata.owner, owner);
    strcpy(metadata.group, group);

    return metadata;
}

// function to print metadata
void printMetadata(Metadata metadata)
{
    printf("File size: %lld\n", metadata.file_size == 1? 4096 : metadata.file_size);
    printf("File name: %s\n", metadata.file_name);
    printf("File path: %s\n", metadata.path);
    printf("File permission code: %o\n", metadata.mode);
    printf("File permissions: %s\n", metadata.permissions);
    printf("File owner: %s\n", metadata.owner);
    printf("File group: %s\n\n", metadata.group);
}

#endif