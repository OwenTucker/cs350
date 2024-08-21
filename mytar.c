#include "inodemap.h"
#include "inodemap.c"
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <utime.h>


#define MAGICNUMBER 0X7261746D

typedef struct {
    char mode;
    char* outfilename;
    char* dirname;
} Options;

typedef struct {
    unsigned long int size;
    unsigned long int inode;
    unsigned long int mdtime;
    unsigned int mode;
    char* dirname;
    char* filename;
} currentFile;

void tarSearch(char* dir, FILE* fd) {
    struct stat buf;
    struct dirent* de;
    int exists;
    DIR* d;
    char* fullname;
    chmod(dir, 0777);
    d = opendir(dir);
    if (d == NULL) {
        fprintf(stderr, "Couldn't open \"%s\"\n", dir);
        perror("opendir()");
        exit(1);
    }
    fullname = (char*)calloc(1, sizeof(char) * (strlen(dir) + 258));

    //CHANGE EVERYTHING TO UINT32 OR 64 
    for (de = readdir(d); de != NULL; de = readdir(d)) {


        sprintf(fullname, "%s/%s", dir, de->d_name);
        exists = lstat(fullname, &buf);

        if (exists < 0) {
            fprintf(stderr, "%s not found\n", fullname);
            perror("stat()");
        }
        else if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            u_int64_t c[] = { buf.st_ino };
            fwrite(c, 8, 1, fd);
            u_int32_t path_len = strlen(fullname);
            fwrite(&path_len, 4, 1, fd);
            fwrite(fullname, path_len, 1, fd);
            u_int32_t f[] = { buf.st_mode };
            fwrite(f, 4, 1, fd);
            u_int64_t k[] = { buf.st_mtim.tv_sec };
            fwrite(k, 8, 1, fd);

        }
        else if (S_ISLNK(buf.st_mode)) {
            continue;
        }
        else if (S_ISREG(buf.st_mode)) {
            if (find_Inode(buf.st_ino)) {
                u_int64_t c[] = { buf.st_ino };
                fwrite(c, 1, 8, fd);
                size_t path_len = strlen(fullname);
                fwrite(&path_len, 4, 1, fd);
                fwrite(fullname, path_len, 1, fd);
            }
            else {
                FILE* ft;
                u_int64_t c[] = { buf.st_ino };
                fwrite(c, 1, 8, fd);
                u_int32_t path_len = strlen(fullname);
                fwrite(&path_len, 4, 1, fd);
                fwrite(fullname, path_len, 1, fd);
                u_int32_t f[] = { buf.st_mode };
                fwrite(f, 4, 1, fd);
                u_int64_t k[] = { buf.st_mtim.tv_sec };
                fwrite(k, 8, 1, fd);
                u_int64_t si[] = { buf.st_size };
                fwrite(si, 8, 1, fd);
                if (buf.st_size == 0) continue;
                char* cnts = (char*)calloc(1, buf.st_size);
                ft = fopen(fullname, "r");
                if (ft == NULL) perror("stat()");
                fread(cnts, buf.st_size, 1, ft);
                fwrite(cnts, 1, buf.st_size, fd);
                add_Inode(buf.st_ino, fullname);
                fclose(ft);
                free(cnts);
            }
        }
        if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            tarSearch(fullname, fd);
        }

    }

    closedir(d);
}



void makeTar(char* outfilename, char* dir) {
    chmod(dir, 0777);
    int exists;
    FILE* fd;
    fd = fopen(outfilename, "w");
    u_int32_t ch[] = { 0X7261746D };
    fwrite(ch, 4, 1, fd);
    struct stat buf;
    exists = stat(dir, &buf);
    u_int64_t c[] = { buf.st_ino };
    fwrite(c, 8, 1, fd);
    size_t path_len = strlen(dir);
    fwrite(&path_len, 4, 1, fd);
    fwrite(dir, path_len, 1, fd);
    u_int32_t f[] = { buf.st_mode };
    fwrite(f, 4, 1, fd);
    u_int64_t k[] = { buf.st_mtim.tv_sec };
    fwrite(k, 8, 1, fd);

    tarSearch(dir, fd);
    fclose(fd);
}


void pTar(char* infile, currentFile* p) {
    u_int32_t sz, i, exists;
    struct stat buf;
    FILE* fd;
    char* c; //probably have to change this to some variable value based on size of infile
    exists = stat(infile, &buf);
    c = (char*)calloc(1, buf.st_size * sizeof(char));

    fd = fopen(infile, "r");
    fseek(fd, 0, SEEK_SET);
    sz = fread(c, 1, 4, fd);
    int mg = *(int*)c;
    if (mg != MAGICNUMBER) {
        printf("Error: Bad magic number (%d), should be: %d.\n", mg, MAGICNUMBER);
        exit(-1);
    }
    free(c);
    while (fread(&(p->inode), 1, 8, fd) == 8) {
        if (!find_Inode(p->inode)) {
            c = (char*)calloc(1, buf.st_size * sizeof(char));
            memset(c, 0, buf.st_size);
            fread(c, 4, 1, fd); //read file name size
            i = *(u_int32_t*)(c);
            char bname[i];
            sz = fread(bname, 1, i, fd); //file name
            p->filename = (char*)calloc(1, (i + 1) * sizeof(char));
            strncpy(p->filename, bname, i);
            sz = fread(c, 1, 4, fd); //file mode
            p->mode = *(int*)(c);
            sz = fread(c, 1, 8, fd); //file mod time
            p->mdtime = *(int*)(c);
            if (S_ISREG(p->mode) && p->mode) {
                sz = fread(c, 1, 8, fd); //file size
                p->size = *(int*)(c);
                sz = fread(c, 1, p->size, fd);
                printf("%s/ -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", p->filename, p->inode, p->mode, p->mdtime, p->size);
            }
            else printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", p->filename, p->inode, p->mode, p->mdtime);
            add_Inode(p->inode, p->filename);

        }
        else {
            sz = fread(c, 1, 4, fd); //read file name size
            i = *(int*)(c);
            char bname[i];
            sz = fread(bname, 1, i, fd); //file name
            p->filename = (char*)calloc(1, (i + 1) * sizeof(char));
            strncpy(p->filename, bname, i);
            printf("%s/ -- inode: %lu\n", p->filename, p->inode);
            free(p->filename);
        }

    }
    free(c);

}

void eTar(char* infile, currentFile* e) {
    int exists, i, sz;
    struct stat buf;
    //struct timeval newtime;
    //struct utimbuf new_times;
    char* c; //probably have to change this to some variable value based on size of infile
    exists = stat(infile, &buf);
    c = (char*)calloc(1, buf.st_size * sizeof(char));
    FILE* fd;
    fd = fopen(infile, "r");
    fseek(fd, 0, SEEK_SET);
    sz = fread(c, 1, 4, fd);
    int mg = *(int*)c;
    if (mg != MAGICNUMBER) {
        printf("Error: Bad magic number (%d), should be: %d.\n", mg, MAGICNUMBER);
        exit(-1);
    }
    free(c);
    while (fread(&(e->inode), 1, 8, fd) == 8) {
        c = (char*)calloc(1, buf.st_size * sizeof(char));
        memset(c, 0, buf.st_size);
        if (!find_Inode(e->inode)) { //treat it as file or dir
            fread(c, 1, 4, fd); //read file name size
            i = *(int*)(c);
            char bname[i];
            fread(bname, i, 1, fd); //file name
            e->filename = (char*)calloc(1, (i + 1) * sizeof(char));
            strncpy(e->filename, bname, i);
            fread(c, 1, 4, fd); //file mode
            e->mode = *(int*)(c);
            if (S_ISDIR(e->mode)) { // for dir
                mkdir(e->filename, e->mode);
                fread(c, 8, 1, fd); //mod
                add_Inode(e->inode, e->filename);
            }
            else { //its a file
                FILE* new;
                new = fopen(e->filename, "w");
                struct timeval mtime;
                struct timeval newtimes[2];
                fread(c, 8, 1, fd); //mod 
                e->mdtime = *(int*)(c);
                gettimeofday(&mtime, NULL);
                mtime.tv_sec = e->mdtime;
                mtime.tv_usec = 0;
                fread(c, 1, 8, fd); //file size
                e->size = *(int*)(c);
                fread(c, 1, e->size, fd);
                fwrite(c, 1, e->size, new);
                add_Inode(e->inode, e->filename);
                chmod(e->filename, e->mode);
                newtimes[0] = newtimes[1] = mtime;
                utimes(e->filename, newtimes);
                fclose(new);
            }
        }
        else { //its a link
            fread(c, 1, 4, fd); //read file name size
            i = *(int*)(c);
            char bname[i];
            fread(bname, 1, i, fd); //file name
            e->filename = (char*)calloc(1, (i + 1) * sizeof(char));
            strncpy(e->filename, bname, i);
            link(find_Inode(e->inode), e->filename);
            free(e->filename);
        }
        free(c);
    }
    fclose(fd);

}

void getOptions(int argc, char* argv[], Options* t) {
    unsigned int o;
    t->mode = 'n';
    while ((o = getopt(argc, argv, "cxtf:")) != -1) {
        switch (o) {
        case 'c':
            t->mode = 'c';
            break;
        case 't':
            if (t->mode == 'n') t->mode = 't';
            else {
                printf("Error: Multiple modes specified.\n");
                exit(-1);
            }
            break;
        case 'x':
            if (t->mode == 'n') t->mode = 'x';
            else {
                printf("Error: Multiple modes specified.\n");
                exit(-1);
            }
            break;
        case 'f':
            t->outfilename = optarg;
            if (optarg == NULL) printf("Error: No tarfile specified.\n");
            break;
        default:
            if (t->outfilename == NULL) {
                printf("Usage: mytar [-cxtf] [TAR]\n");
                exit(-1);
            }
            break;
        }
        if (t->mode == 'n') {
            printf("Error: No mode specified.\n");
            exit(-1);
        }
        t->dirname = argv[argc - 1];

        if (t->mode == 'c' && opendir(t->dirname) == NULL) {
            printf("Error: No directory target specified.\n");
            exit(-1);
        }
    }
}
int main(int argc, char* argv[]) {
    Options options, * t;
    currentFile curr, * p;
    p = &curr;
    t = &options;

    getOptions(argc, argv, t);
    p->dirname = t->dirname;

    if (t->mode == 't') pTar(t->outfilename, p);
    if (t->mode == 'c') makeTar(t->outfilename, t->dirname);
    if (t->mode == 'x') {
        currentFile ext, * e;
        e = &ext;
        eTar(t->outfilename, e);
    }
    // if (t->mode == 'x') eTar(t->outfilename);
    // printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
}
