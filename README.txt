THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING AN EXTERNAL TUTOR
OR ANY CODE NOT OBTAINED FROM THE COURSE INSTRUCTOR. FURTHERMORE, I DID NOT
COLLABORATE WITH ANYONE, RECEIVE ANY ASSISTANCE NOR USE ANY EXTERNAL SOURCES
NOT CITED BELOW.

 struct options from ppmcvt.c (my lab 1)
 tarSearch from du5/6/7/8.c
 getOptions from ppmcvt.c
 Looked at man pages for pretty much every library function
 Modtime aided by touch.c on demos
 Looked at slides (20-25) for various information
 makefile is from mymalloc 

 - OWEN TUCKER

commit ceb74c853b6cdca2dfe62d1e66c5ac663042874f
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Sat Mar 23 13:27:10 2024 -0400

    fixed makefile
---
 Labs/Lab2/Makefile | 5 ++++-
 1 file changed, 4 insertions(+), 1 deletion(-)

diff --git a/Labs/Lab2/Makefile b/Labs/Lab2/Makefile
index d92443b..1794f6c 100644
--- a/Labs/Lab2/Makefile
+++ b/Labs/Lab2/Makefile
@@ -3,4 +3,7 @@ CFLAGS=-std=gnu99 -Wno-deprecated-declarations -g -Wall
 
 .PHONY: cleanall cleanobj
 
-all: $(TARGET)
\ No newline at end of file
+all: $(TARGET)
+
+cleanall: cleanobj
+	$(RM) $(TARGET)

commit 8d809c07903d59f852531df10b32e5ef943eaa10
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Sat Mar 23 13:06:32 2024 -0400

    did a lot, almost done but might be error with lab sol
---
 Labs/Lab2/mytar.c | 67 ++++++++++++++++++++++++++++++++++++++++++++++++++-----
 1 file changed, 62 insertions(+), 5 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index ce6a96a..cc20d11 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -9,6 +9,8 @@
 #include <unistd.h>
 #include <dirent.h>
 #include <string.h>
+#include <sys/time.h>
+#include <utime.h>
 
 
 #define MAGICNUMBER 0X7261746D
@@ -196,8 +198,14 @@ void pTar(char* infile, currentFile* p) {
 
 }
 
-void eTar(char* infile) {
-    int exists, i;
+ void eTar(char* infile, currentFile* e) {
+    int exists, i, sz;
+    struct stat buf;
+    struct timeval newtime;
+    struct utimbuf new_times;
+    char* c; //probably have to change this to some variable value based on size of infile
+    exists = stat(infile, &buf);
+    c = (char*)calloc(1, buf.st_size * sizeof(char));
     FILE* fd; 
     fd = fopen(infile, "r");
     fseek(fd, 0, SEEK_SET);
@@ -207,9 +215,53 @@ void eTar(char* infile) {
         printf("Error: Bad magic number (%d), should be: %d.\n", mg, MAGICNUMBER);
         exit(-1);
     }
-    while (fread(&(p->inode), 1, 8, fd) == 8) {
-
+    free(c);
+    while (fread(&(e->inode), 1, 8, fd) == 8) {
+        c = (char*)calloc(1, buf.st_size * sizeof(char));
+        if (!find_Inode(e->inode)) { //treat it as file or dir
+            fread(c, 1, 4, fd); //read file name size
+            i = *(int*)(c);
+            char bname[i];
+            fread(bname, 1, i, fd); //file name
+            e->filename = (char*)malloc((i + 1) * sizeof(char));
+            strncpy(e->filename, bname, i);
+            fread(c, 1, 4, fd); //file mode
+            e->mode = *(int*)(c);
+            if (S_ISDIR(e->mode)) { // for dir
+                mkdir(e->filename, e->mode);
+                fread(c, 8, 1, fd); //mod
+                add_Inode(e->inode, e->filename);
+            }
+            else { //its a file
+                FILE* new;
+                new = fopen(e->filename, "w");
+                fread(c, 8, 1, fd); //mod 
+                gettimeofday(&newtime, NULL); 
+                newtime.tv_usec = 0;
+                new_times.actime = newtime.tv_sec;
+                new_times.modtime = newtime.tv_sec;
+                utime(e->filename, &new_times);
+                fread(c, 1, 8, fd); //file size
+                e->size = *(int*)(c);
+                fread(c, 1, e->size, fd);
+                fwrite(c, 1, e->size, new);
+                add_Inode(e->inode, e->filename);
+                chmod(e->filename, e->mode);
+                fclose(new);
+            }
+        }
+        else { //its a link
+            fread(c, 1, 4, fd); //read file name size
+            i = *(int*)(c);
+            char bname[i];
+            fread(bname, 1, i, fd); //file name
+            e->filename = (char*)malloc((i + 1) * sizeof(char));
+            strncpy(e->filename, bname, i);
+            link(find_Inode(e->inode), e->filename);
+        }
+        free(c);
     }
+    fclose(fd);
 
 }
 
@@ -266,6 +318,11 @@ void getOptions(int argc, char* argv[], Options* t) {
         
         if (t->mode == 't') pTar(t->outfilename, p);
         if (t->mode == 'c') makeTar(t->outfilename, t->dirname);
-        if (t->mode == 'x') eTar(t->outfilename);
+        if (t->mode == 'x') {
+            currentFile ext, * e;
+            e = &ext; 
+            eTar(t->outfilename, e);
+        } 
+       // if (t->mode == 'x') eTar(t->outfilename);
        // printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit 954b67968403ec68326c943bcc390f24402fbe58
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Thu Mar 21 15:31:11 2024 -0400

    start -x
---
 Labs/Lab2/mytar.c | 172 +++++++++++++++++++++++++++++++++++++-----------------
 1 file changed, 117 insertions(+), 55 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index 188df59..ce6a96a 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -1,4 +1,5 @@
 #include "inodemap.h"
+#include "inodemap.c"
 #include <stdio.h>
 #include <stdlib.h>
 #include "getopt.h"
@@ -9,12 +10,14 @@
 #include <dirent.h>
 #include <string.h>
 
-//TEST IF PRINT WORKS WITH MG
+
 #define MAGICNUMBER 0X7261746D
 
+
+
 /* reminders are where things are from
  struct options from ppmcvt.c
- tarSearch from du5.c and du6.c
+ tarSearch from du5/6/7/8.c
  pTar from nothing really, used lseek, fopen, fread.. etc man pages
  getOptions from ppmcvt.c
 */
@@ -37,14 +40,6 @@ typedef struct {
     char* filename; 
 } currentFile;
 
-void myWrite(FILE* fd, long int data) //use u_int32
-{
-    int c[] = { data }; 
-    fwrite(c, 1, sizeof(c), fd);
-    printf("wrote: %ls\n", c);
-}
-
-
 void tarSearch(char* dir, FILE* fd) {
     struct stat buf;
     struct dirent* de;
@@ -58,9 +53,11 @@ void tarSearch(char* dir, FILE* fd) {
     }
     //CHANGE EVERYTHING TO UINT32 OR 64 
     for (de = readdir(d); de != NULL; de = readdir(d)) {
+        
         char path[1024]; //might need to change this to something longer 
-        sprintf(path, "%s/%s", dir, de->d_name);//concat dir and de name to path --MAYBE CANT USE SPRINTF
+        sprintf(path, "%s/%s", dir, de->d_name);
         exists = stat(path, &buf);
+        
         if (exists < 0) {
             fprintf(stderr, "%s not found\n", path);
             perror("stat()");
@@ -81,32 +78,43 @@ void tarSearch(char* dir, FILE* fd) {
             continue;
         }
         else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
-            
-            FILE* ft; 
-            u_int64_t c[] = { buf.st_ino };
-            fwrite(c, 1, 8, fd);
-            size_t path_len = strlen(path);
-            fwrite(&path_len, 4, 1, fd);
-            fwrite(path, path_len, 1, fd); 
-            u_int32_t f[] = { buf.st_mode };
-            fwrite(f, 4, 1, fd);
-            u_int64_t k[] = { buf.st_mtim.tv_sec };
-            fwrite(k, 8,1, fd);
-            u_int64_t si[] = { buf.st_size };
-            fwrite(si, 8, 1, fd);
-            if (buf.st_size == 0) continue; 
-            char* cnts = (char*) calloc(1, buf.st_size);
-            ft = fopen(path, "r");
-            if (ft == NULL) perror("stat()");
-            fread(cnts, buf.st_size, 1, ft);
-            fwrite(cnts, 1, buf.st_size, fd);
-            printf("%10ld %s*\n", buf.st_size, path);
-            fclose(ft);
+            if (find_Inode(buf.st_ino)) {
+                u_int64_t c[] = { buf.st_ino };
+                fwrite(c, 1, 8, fd);
+                size_t path_len = strlen(path);
+                fwrite(&path_len, 4, 1, fd);
+                fwrite(path, path_len, 1, fd);
+            }
+            else {
+                FILE* ft;
+                u_int64_t c[] = { buf.st_ino };
+                fwrite(c, 1, 8, fd);
+                size_t path_len = strlen(path);
+                fwrite(&path_len, 4, 1, fd);
+                fwrite(path, path_len, 1, fd);
+                u_int32_t f[] = { buf.st_mode };
+                fwrite(f, 4, 1, fd);
+                u_int64_t k[] = { buf.st_mtim.tv_sec };
+                fwrite(k, 8, 1, fd);
+                u_int64_t si[] = { buf.st_size };
+                fwrite(si, 8, 1, fd);
+                if (buf.st_size == 0) continue;
+                char* cnts = (char*)calloc(1, buf.st_size);
+                ft = fopen(path, "r");
+                if (ft == NULL) perror("stat()");
+                fread(cnts, buf.st_size, 1, ft);
+                fwrite(cnts, 1, buf.st_size, fd);
+                printf("%10ld %s*\n", buf.st_size, path);
+                add_Inode(buf.st_ino, path);
+                fclose(ft);
+            }
         }
         if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
             tarSearch(path, fd);
         }
+        
     }
+    
     closedir(d);
 }
 
@@ -147,51 +155,104 @@ void pTar(char* infile, currentFile* p) {
     fseek(fd, 0, SEEK_SET);
     sz = fread(c, 1, 4, fd);
     int mg = *(int*)c;
-    if (mg != MAGICNUMBER) exit(-1);
+    if (mg != MAGICNUMBER) {
+        printf("Error: Bad magic number (%d), should be: %d.\n", mg, MAGICNUMBER);
+        exit(-1);
+    }
+    
+        while (fread(&(p->inode), 1, 8, fd) == 8) {
+            if (!find_Inode(p->inode)) {
+                sz = fread(c, 1, 4, fd); //read file name size
+                i = *(int*)(c);
+                char bname[i];
+                sz = fread(bname, 1, i, fd); //file name
+                p->filename = (char*)malloc((i + 1) * sizeof(char));
+                strncpy(p->filename, bname, i);
+                sz = fread(c, 1, 4, fd); //file mode
+                p->mode = *(int*)(c);
+                sz = fread(c, 1, 8, fd); //file mod time
+                p->mdtime = *(int*)(c);
+                if (S_ISREG(p->mode) && p->mode) {
+                    sz = fread(c, 1, 8, fd); //file size
+                    p->size = *(int*)(c);
+                    sz = fread(c, 1, p->size, fd);
+                    printf("%s/ -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", p->filename, p->inode, p->mode, p->mdtime, p->size);
+                }
+                else printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", p->filename, p->inode, p->mode, p->mdtime);
+                add_Inode(p->inode, p->filename);
+               
+            }
+            else {
+                sz = fread(c, 1, 4, fd); //read file name size
+                i = *(int*)(c);
+                char bname[i];
+                sz = fread(bname, 1, i, fd); //file name
+                p->filename = (char*)malloc((i + 1) * sizeof(char));
+                strncpy(p->filename, bname, i);
+                printf("%s/ -- inode: %lu\n", p->filename, p->inode);
+            }
+    }
+  
+
+}
+
+void eTar(char* infile) {
+    int exists, i;
+    FILE* fd; 
+    fd = fopen(infile, "r");
+    fseek(fd, 0, SEEK_SET);
+    sz = fread(c, 1, 4, fd);
+    int mg = *(int*)c;
+    if (mg != MAGICNUMBER) {
+        printf("Error: Bad magic number (%d), should be: %d.\n", mg, MAGICNUMBER);
+        exit(-1);
+    }
     while (fread(&(p->inode), 1, 8, fd) == 8) {
-        sz = fread(c, 1, 4, fd); //read file name size
-        i = *(int*)(c);
-        char bname[i]; 
-        sz = fread(bname, 1, i, fd); //file name
-        p->filename = (char*)malloc((i+1) * sizeof(char));
-        strncpy(p->filename, bname, i);
-        sz = fread(c, 1, 4, fd); //file mode
-        p->mode = *(int*)(c);
-        sz = fread(c, 1, 8, fd); //file mod time
-        p->mdtime = *(int*)(c);
-        if (S_ISREG(p->mode) && p->mode) {
-            sz = fread(c, 1, 8, fd); //file size
-            p->size = *(int*)(c);
-            sz = fread(c, 1, p->size, fd); 
-            printf("%s/ -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", p->filename, p->inode, p->mode, p->mdtime, p->size);
-        }
-        else printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", p->filename, p->inode, p->mode, p->mdtime);
 
     }
 
 }
 
 void getOptions(int argc, char* argv[], Options* t) {
-    unsigned int o; 
+    unsigned int o;
+    t->mode = 'n';
     while ((o = getopt(argc, argv, "cxtf:")) != -1) {
         switch (o) {
         case 'c':
             t->mode = 'c';
             break;
         case 't':
-            t->mode = 't';
+            if (t->mode == 'n') t->mode = 't';
+            else {
+                printf("Error: Multiple modes specified.\n");
+                exit(-1);
+            }
             break;
         case 'x':
-            t->mode = 'x';
+            if (t->mode == 'n') t->mode = 'x';
+            else {
+                printf("Error: Multiple modes specified.\n");
+                exit(-1);
+            }
             break;
         case 'f':
-            t->outfilename = optarg; 
+            if (t->outfilename == 0) printf("Error: No tarfile specified.\n");
+            t->outfilename = optarg;
             break;
         default:
-            if(t->outfilename == NULL) printf("Usage: mytar [-cxtf] [TAR]\n");
+            if (t->outfilename == NULL) printf("Usage: mytar [-cxtf] [TAR]\n");
             break;
         }
+        if (t->mode == 'n') {
+            printf("Error: No mode specified.\n");
+            exit(-1);
+        }
         t->dirname = argv[argc - 1];
+
+        if (t->mode == 'c' && opendir(t->dirname) == NULL) {
+            printf("Error: No directory target specified.\n");
+            exit(-1);
+        }
     }
 }
     int main(int argc, char* argv[]) {
@@ -205,5 +266,6 @@ void getOptions(int argc, char* argv[], Options* t) {
         
         if (t->mode == 't') pTar(t->outfilename, p);
         if (t->mode == 'c') makeTar(t->outfilename, t->dirname);
+        if (t->mode == 'x') eTar(t->outfilename);
        // printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit f1a5e78df83fae3645aa5ecc780f2eff0cc93ebc
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Thu Mar 21 13:17:28 2024 -0400

    getting ready to test hard links
---
 Labs/Lab2/mytar.c | 20 ++++++++++----------
 1 file changed, 10 insertions(+), 10 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index b8265ef..188df59 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -71,28 +71,28 @@ void tarSearch(char* dir, FILE* fd) {
             size_t path_len = strlen(path); 
             fwrite(&path_len, 4, 1, fd); 
             fwrite(path, path_len, 1, fd); 
-            int f[] = { buf.st_mode };
-            fwrite(f, 1, sizeof(f), fd); 
-            int k[] = { buf.st_mtim.tv_sec };
+            u_int32_t f[] = { buf.st_mode };
+            fwrite(f, 4, 1, fd); 
+            u_int64_t k[] = { buf.st_mtim.tv_sec };
             fwrite(k, 8, 1, fd);
             printf("%10ld %s/\n", buf.st_size, path);
         }
         else if (S_ISLNK(buf.st_mode)) {
-            printf("%10ld %s/\n", buf.st_size, path);
+            continue;
         }
         else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
             
             FILE* ft; 
-            int c[] = { buf.st_ino };
+            u_int64_t c[] = { buf.st_ino };
             fwrite(c, 1, 8, fd);
             size_t path_len = strlen(path);
             fwrite(&path_len, 4, 1, fd);
             fwrite(path, path_len, 1, fd); 
-            int f[] = { buf.st_mode };
+            u_int32_t f[] = { buf.st_mode };
             fwrite(f, 4, 1, fd);
-            int k[] = { buf.st_mtim.tv_sec };
+            u_int64_t k[] = { buf.st_mtim.tv_sec };
             fwrite(k, 8,1, fd);
-            int si[] = { buf.st_size };
+            u_int64_t si[] = { buf.st_size };
             fwrite(si, 8, 1, fd);
             if (buf.st_size == 0) continue; 
             char* cnts = (char*) calloc(1, buf.st_size);
@@ -125,9 +125,9 @@ void tarSearch(char* dir, FILE* fd) {
      size_t path_len = strlen(dir);
      fwrite(&path_len, 4, 1, fd);
      fwrite(dir, path_len, 1, fd);
-     int f[] = { buf.st_mode };
+     u_int32_t f[] = { buf.st_mode };
      fwrite(f, 1, sizeof(f), fd);
-     int k[] = { buf.st_mtim.tv_sec };
+     u_int64_t k[] = { buf.st_mtim.tv_sec };
      fwrite(k, 8, 1, fd);
 
      tarSearch(dir, fd); 

commit a7115964d3e37747846fbd23007051e9ce153119
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Wed Mar 20 23:28:34 2024 -0400

    print and create seem to be working well, sans the link part
---
 Labs/Lab2/mytar.c | 36 +++++++++++++++++++++++++-----------
 1 file changed, 25 insertions(+), 11 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index 423d011..b8265ef 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -56,6 +56,7 @@ void tarSearch(char* dir, FILE* fd) {
         perror("opendir()");
         exit(1);
     }
+    //CHANGE EVERYTHING TO UINT32 OR 64 
     for (de = readdir(d); de != NULL; de = readdir(d)) {
         char path[1024]; //might need to change this to something longer 
         sprintf(path, "%s/%s", dir, de->d_name);//concat dir and de name to path --MAYBE CANT USE SPRINTF
@@ -65,15 +66,15 @@ void tarSearch(char* dir, FILE* fd) {
             perror("stat()");
         }
         else if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
-            int c[8] = { buf.st_ino };
+            u_int64_t c[] = { buf.st_ino };
             fwrite(c, 8, 1, fd);
             size_t path_len = strlen(path); 
-            fwrite(&path_len, sizeof(path_len), 1, fd); 
+            fwrite(&path_len, 4, 1, fd); 
             fwrite(path, path_len, 1, fd); 
             int f[] = { buf.st_mode };
             fwrite(f, 1, sizeof(f), fd); 
-            int k[] = { buf.st_mtime };
-            fwrite(k, 1, sizeof(k), fd);
+            int k[] = { buf.st_mtim.tv_sec };
+            fwrite(k, 8, 1, fd);
             printf("%10ld %s/\n", buf.st_size, path);
         }
         else if (S_ISLNK(buf.st_mode)) {
@@ -85,14 +86,14 @@ void tarSearch(char* dir, FILE* fd) {
             int c[] = { buf.st_ino };
             fwrite(c, 1, 8, fd);
             size_t path_len = strlen(path);
-            fwrite(&path_len, sizeof(path_len), 1, fd);
+            fwrite(&path_len, 4, 1, fd);
             fwrite(path, path_len, 1, fd); 
             int f[] = { buf.st_mode };
-            fwrite(f, 1, sizeof(f), fd);
-            int k[] = { buf.st_mtime };
-            fwrite(k, 1, sizeof(k), fd);
+            fwrite(f, 4, 1, fd);
+            int k[] = { buf.st_mtim.tv_sec };
+            fwrite(k, 8,1, fd);
             int si[] = { buf.st_size };
-            fwrite(si, 1, sizeof(si), fd);
+            fwrite(si, 8, 1, fd);
             if (buf.st_size == 0) continue; 
             char* cnts = (char*) calloc(1, buf.st_size);
             ft = fopen(path, "r");
@@ -112,10 +113,23 @@ void tarSearch(char* dir, FILE* fd) {
 
 
  void makeTar(char* outfilename, char* dir) {
+     int exists;
      FILE* fd;
      fd = fopen(outfilename, "w"); 
-     u_int32_t c[] = { 0X7261746D }; 
-     fwrite(c, 1, sizeof(c), fd); 
+     u_int32_t ch[] = { 0X7261746D }; 
+     fwrite(ch, 4, 1, fd); 
+     struct stat buf;
+     exists = stat(dir, &buf);
+     u_int64_t c[] = { buf.st_ino };
+     fwrite(c, 8, 1, fd);
+     size_t path_len = strlen(dir);
+     fwrite(&path_len, 4, 1, fd);
+     fwrite(dir, path_len, 1, fd);
+     int f[] = { buf.st_mode };
+     fwrite(f, 1, sizeof(f), fd);
+     int k[] = { buf.st_mtim.tv_sec };
+     fwrite(k, 8, 1, fd);
+
      tarSearch(dir, fd); 
      fclose(fd);
 }

commit 1986b70353672b065cd34f0e254f26f55b5464c6
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Wed Mar 20 16:07:25 2024 -0400

    did a lot of work. creating tar is almost done
---
 Labs/Lab2/mytar.c | 86 +++++++++++++++++++++++++++++++++++++++++++------------
 1 file changed, 67 insertions(+), 19 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index f6e3429..423d011 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -9,12 +9,20 @@
 #include <dirent.h>
 #include <string.h>
 
-#define MAGICNUMBER 1918989421
+//TEST IF PRINT WORKS WITH MG
+#define MAGICNUMBER 0X7261746D
 
-/*readsize = atoi(outfilename);
-    c = (char*)malloc(readsize * sizeof(char));*/
+/* reminders are where things are from
+ struct options from ppmcvt.c
+ tarSearch from du5.c and du6.c
+ pTar from nothing really, used lseek, fopen, fread.. etc man pages
+ getOptions from ppmcvt.c
+*/
+/* u_int64_t* data;
+            data = (u_int64_t*) malloc(buf.st_ino * sizeof(u_int64_t));
+            data = (u_int64_t*)buf.st_ino;*/
 
-typedef struct {
+typedef struct { 
     char mode;
     char* outfilename;
     char* dirname;
@@ -29,7 +37,15 @@ typedef struct {
     char* filename; 
 } currentFile;
 
-void tarSearch(char* dir) {
+void myWrite(FILE* fd, long int data) //use u_int32
+{
+    int c[] = { data }; 
+    fwrite(c, 1, sizeof(c), fd);
+    printf("wrote: %ls\n", c);
+}
+
+
+void tarSearch(char* dir, FILE* fd) {
     struct stat buf;
     struct dirent* de;
     int exists; 
@@ -42,36 +58,69 @@ void tarSearch(char* dir) {
     }
     for (de = readdir(d); de != NULL; de = readdir(d)) {
         char path[1024]; //might need to change this to something longer 
-        sprintf(path, "%s/%s", dir, de->d_name);//concat dir and de name to path
+        sprintf(path, "%s/%s", dir, de->d_name);//concat dir and de name to path --MAYBE CANT USE SPRINTF
         exists = stat(path, &buf);
         if (exists < 0) {
             fprintf(stderr, "%s not found\n", path);
             perror("stat()");
         }
-        else if (S_ISDIR(buf.st_mode)) {
+        else if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
+            int c[8] = { buf.st_ino };
+            fwrite(c, 8, 1, fd);
+            size_t path_len = strlen(path); 
+            fwrite(&path_len, sizeof(path_len), 1, fd); 
+            fwrite(path, path_len, 1, fd); 
+            int f[] = { buf.st_mode };
+            fwrite(f, 1, sizeof(f), fd); 
+            int k[] = { buf.st_mtime };
+            fwrite(k, 1, sizeof(k), fd);
             printf("%10ld %s/\n", buf.st_size, path);
         }
         else if (S_ISLNK(buf.st_mode)) {
             printf("%10ld %s/\n", buf.st_size, path);
         }
-        else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
+        else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
+            
+            FILE* ft; 
+            int c[] = { buf.st_ino };
+            fwrite(c, 1, 8, fd);
+            size_t path_len = strlen(path);
+            fwrite(&path_len, sizeof(path_len), 1, fd);
+            fwrite(path, path_len, 1, fd); 
+            int f[] = { buf.st_mode };
+            fwrite(f, 1, sizeof(f), fd);
+            int k[] = { buf.st_mtime };
+            fwrite(k, 1, sizeof(k), fd);
+            int si[] = { buf.st_size };
+            fwrite(si, 1, sizeof(si), fd);
+            if (buf.st_size == 0) continue; 
+            char* cnts = (char*) calloc(1, buf.st_size);
+            ft = fopen(path, "r");
+            if (ft == NULL) perror("stat()");
+            fread(cnts, buf.st_size, 1, ft);
+            fwrite(cnts, 1, buf.st_size, fd);
             printf("%10ld %s*\n", buf.st_size, path);
+            fclose(ft);
         }
         if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
-            tarSearch(path);
+            tarSearch(path, fd);
         }
     }
     closedir(d);
 }
 
- void makeTar(char* outfilename, char* dir) {
-     
-     //fd = fopen(outfilename, "w"); 
-    
-     tarSearch(dir); 
 
+
+ void makeTar(char* outfilename, char* dir) {
+     FILE* fd;
+     fd = fopen(outfilename, "w"); 
+     u_int32_t c[] = { 0X7261746D }; 
+     fwrite(c, 1, sizeof(c), fd); 
+     tarSearch(dir, fd); 
+     fclose(fd);
 }
 
+
 void pTar(char* infile, currentFile* p) {
     int sz, i, exists;
     struct stat buf;
@@ -96,12 +145,12 @@ void pTar(char* infile, currentFile* p) {
         p->mode = *(int*)(c);
         sz = fread(c, 1, 8, fd); //file mod time
         p->mdtime = *(int*)(c);
-        if (p->mode > 0100000) {
+        if (S_ISREG(p->mode) && p->mode) {
             sz = fread(c, 1, 8, fd); //file size
             p->size = *(int*)(c);
             sz = fread(c, 1, p->size, fd); 
             printf("%s/ -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", p->filename, p->inode, p->mode, p->mdtime, p->size);
-        }//really stupid way of seeing if something is a file
+        }
         else printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", p->filename, p->inode, p->mode, p->mdtime);
 
     }
@@ -122,7 +171,6 @@ void getOptions(int argc, char* argv[], Options* t) {
             t->mode = 'x';
             break;
         case 'f':
-            t->mode = 'f';
             t->outfilename = optarg; 
             break;
         default:
@@ -141,7 +189,7 @@ void getOptions(int argc, char* argv[], Options* t) {
         getOptions(argc, argv, t);
         p->dirname = t->dirname; 
         
-       // pTar(t->outfilename, p);
-        makeTar(t->outfilename, t->dirname);
+        if (t->mode == 't') pTar(t->outfilename, p);
+        if (t->mode == 'c') makeTar(t->outfilename, t->dirname);
        // printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit 32986c7d27a523b48fe01eed1d718a6734b467da
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Wed Mar 20 09:48:19 2024 -0400

    able to recursively go through directories
---
 Labs/Lab2/mytar.c | 41 ++++++++++++++++++++++-------------------
 1 file changed, 22 insertions(+), 19 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index bf55402..f6e3429 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -41,24 +41,27 @@ void tarSearch(char* dir) {
         exit(1);
     }
     for (de = readdir(d); de != NULL; de = readdir(d)) {
-        exists = stat(de->d_name, &buf);
-            if (exists < 0) {
-                fprintf(stderr, "%s not found\n", de->d_name);
-                perror("stat()");
-            }
-            else if (S_ISDIR(buf.st_mode)) {
-                printf("%10ld %s/\n", buf.st_size, de->d_name);
-                tarSearch(de->d_name);
-          
-
-            } else if(S_ISLNK(buf.st_mode)) {
-                printf("%10ld %s/\n", buf.st_size, de->d_name);
-            }
-            else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
-                printf("%10ld %s*\n", buf.st_size, de->d_name);
-            }
-
+        char path[1024]; //might need to change this to something longer 
+        sprintf(path, "%s/%s", dir, de->d_name);//concat dir and de name to path
+        exists = stat(path, &buf);
+        if (exists < 0) {
+            fprintf(stderr, "%s not found\n", path);
+            perror("stat()");
+        }
+        else if (S_ISDIR(buf.st_mode)) {
+            printf("%10ld %s/\n", buf.st_size, path);
+        }
+        else if (S_ISLNK(buf.st_mode)) {
+            printf("%10ld %s/\n", buf.st_size, path);
+        }
+        else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
+            printf("%10ld %s*\n", buf.st_size, path);
+        }
+        if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
+            tarSearch(path);
+        }
     }
+    closedir(d);
 }
 
  void makeTar(char* outfilename, char* dir) {
@@ -138,7 +141,7 @@ void getOptions(int argc, char* argv[], Options* t) {
         getOptions(argc, argv, t);
         p->dirname = t->dirname; 
         
-        pTar(t->outfilename, p);
-       // makeTar(t->outfilename, t->dirname);
+       // pTar(t->outfilename, p);
+        makeTar(t->outfilename, t->dirname);
        // printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit 1ff697342edcf8205804c049a57aa38f661098a3
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Tue Mar 19 19:05:56 2024 -0400

    printing works! except for links need to do that
---
 Labs/Lab2/mytar.c | 38 +++++++++++++++++++++-----------------
 1 file changed, 21 insertions(+), 17 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index 3f1fff0..bf55402 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -7,6 +7,7 @@
 #include <fcntl.h>
 #include <unistd.h>
 #include <dirent.h>
+#include <string.h>
 
 #define MAGICNUMBER 1918989421
 
@@ -34,6 +35,11 @@ void tarSearch(char* dir) {
     int exists; 
     DIR* d;
     d = opendir(dir);
+    if (d == NULL) {
+        fprintf(stderr, "Couldn't open \"%s\"\n", dir);
+        perror("opendir()");
+        exit(1);
+    }
     for (de = readdir(d); de != NULL; de = readdir(d)) {
         exists = stat(de->d_name, &buf);
             if (exists < 0) {
@@ -43,7 +49,8 @@ void tarSearch(char* dir) {
             else if (S_ISDIR(buf.st_mode)) {
                 printf("%10ld %s/\n", buf.st_size, de->d_name);
                 tarSearch(de->d_name);
-                
+          
+
             } else if(S_ISLNK(buf.st_mode)) {
                 printf("%10ld %s/\n", buf.st_size, de->d_name);
             }
@@ -65,38 +72,35 @@ void tarSearch(char* dir) {
 void pTar(char* infile, currentFile* p) {
     int sz, i, exists;
     struct stat buf;
-    struct dirent* dp;
     FILE* fd;
     char* c; //probably have to change this to some variable value based on size of infile
     exists = stat(infile, &buf);
     c = (char*) calloc(1, buf.st_size * sizeof(char));
 
-    
     fd = fopen(infile, "r");
     fseek(fd, 0, SEEK_SET);
     sz = fread(c, 1, 4, fd);
     int mg = *(int*)c;
     if (mg != MAGICNUMBER) exit(-1);
-    while (!feof(fd)) {
-        sz = fread(c, 1, 8, fd);//read inode
-        p->inode = *(int*)c; 
+    while (fread(&(p->inode), 1, 8, fd) == 8) {
         sz = fread(c, 1, 4, fd); //read file name size
-        i = *(int*)c;
-        sz = fread(c, 1, i, fd); //file name
-        p->filename = c;
+        i = *(int*)(c);
+        char bname[i]; 
+        sz = fread(bname, 1, i, fd); //file name
+        p->filename = (char*)malloc((i+1) * sizeof(char));
+        strncpy(p->filename, bname, i);
         sz = fread(c, 1, 4, fd); //file mode
-        p->mode = *(int*)c;
+        p->mode = *(int*)(c);
         sz = fread(c, 1, 8, fd); //file mod time
-        p->mdtime = *(int*)c;
+        p->mdtime = *(int*)(c);
         if (p->mode > 0100000) {
             sz = fread(c, 1, 8, fd); //file size
-            p->size = *(int*)c;
+            p->size = *(int*)(c);
             sz = fread(c, 1, p->size, fd); 
             printf("%s/ -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", p->filename, p->inode, p->mode, p->mdtime, p->size);
         }//really stupid way of seeing if something is a file
         else printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", p->filename, p->inode, p->mode, p->mdtime);
-       
-        
+
     }
 
 }
@@ -134,7 +138,7 @@ void getOptions(int argc, char* argv[], Options* t) {
         getOptions(argc, argv, t);
         p->dirname = t->dirname; 
         
-       // pTar(t->outfilename, p);
-        makeTar(t->outfilename, t->dirname);
-        printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
+        pTar(t->outfilename, p);
+       // makeTar(t->outfilename, t->dirname);
+       // printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit ad66b234cd0d0a63a26298aba8fa4d28d4c46a7b
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Tue Mar 19 15:29:36 2024 -0400

    started tar archive
---
 Labs/Lab2/mytar.c | 91 +++++++++++++++++++++++++++++++++++--------------------
 1 file changed, 58 insertions(+), 33 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index 074a772..3f1fff0 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -23,40 +23,54 @@ typedef struct {
     unsigned long int size;
     unsigned long int inode;
     unsigned long int mdtime;
-    unsigned  int mode; 
+    unsigned int mode;
+    char* dirname;
+    char* filename; 
 } currentFile;
 
-/* void printTar(char* outfilename) {
-    int sz = 0;
-    FILE* st;
-    DIR* att;
-    int exists;
-    char* c;
-    
-    exists = stat(outfilename, &buf);
-    printf("%10ld %s\n", buf.st_size, outfilename);
-    c = (char*)malloc(buf.st_size * sizeof(char)); 
+void tarSearch(char* dir) {
+    struct stat buf;
+    struct dirent* de;
+    int exists; 
+    DIR* d;
+    d = opendir(dir);
+    for (de = readdir(d); de != NULL; de = readdir(d)) {
+        exists = stat(de->d_name, &buf);
+            if (exists < 0) {
+                fprintf(stderr, "%s not found\n", de->d_name);
+                perror("stat()");
+            }
+            else if (S_ISDIR(buf.st_mode)) {
+                printf("%10ld %s/\n", buf.st_size, de->d_name);
+                tarSearch(de->d_name);
+                
+            } else if(S_ISLNK(buf.st_mode)) {
+                printf("%10ld %s/\n", buf.st_size, de->d_name);
+            }
+            else if (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
+                printf("%10ld %s*\n", buf.st_size, de->d_name);
+            }
 
-    st = fopen(outfilename, "r");
-    att = opendir(outfilename); 
-    while (att) {
-        dp = readdir(att);
-        if (dp != NULL) {
-            exists = stat(dp, &buf);
-            printf("%10ld %s\n", buf.st_size, dp);
-        }
     }
-    if (st < 0) { perror("fopen()"); exit(1); }
-    sz = fread(c, 4, 1, st);
-    printf("something:%d\n", sz);
 }
-*/
+
+ void makeTar(char* outfilename, char* dir) {
+     
+     //fd = fopen(outfilename, "w"); 
+    
+     tarSearch(dir); 
+
+}
+
 void pTar(char* infile, currentFile* p) {
-    int sz, i;
+    int sz, i, exists;
     struct stat buf;
     struct dirent* dp;
     FILE* fd;
-    char* c[100]; //probably have to change this to some variable value based on size of infile
+    char* c; //probably have to change this to some variable value based on size of infile
+    exists = stat(infile, &buf);
+    c = (char*) calloc(1, buf.st_size * sizeof(char));
+
     
     fd = fopen(infile, "r");
     fseek(fd, 0, SEEK_SET);
@@ -64,16 +78,24 @@ void pTar(char* infile, currentFile* p) {
     int mg = *(int*)c;
     if (mg != MAGICNUMBER) exit(-1);
     while (!feof(fd)) {
-        sz = fread(c, 1, 8, fd);
-        p->inode = *(int*)c;
-        sz = fread(c, 1, 4, fd);
+        sz = fread(c, 1, 8, fd);//read inode
+        p->inode = *(int*)c; 
+        sz = fread(c, 1, 4, fd); //read file name size
         i = *(int*)c;
-        sz = fread(c, 1, i, fd); //i is file name length
-        sz = fread(c, 1, 4, fd); //i is file name length
+        sz = fread(c, 1, i, fd); //file name
+        p->filename = c;
+        sz = fread(c, 1, 4, fd); //file mode
         p->mode = *(int*)c;
-        sz = fread(c, 1, 8, fd); //i is file name length
+        sz = fread(c, 1, 8, fd); //file mod time
         p->mdtime = *(int*)c;
-        printf("file inode: %lu, mode: %o, mtime: %lu\n", p->inode, p->mode, p->mdtime);
+        if (p->mode > 0100000) {
+            sz = fread(c, 1, 8, fd); //file size
+            p->size = *(int*)c;
+            sz = fread(c, 1, p->size, fd); 
+            printf("%s/ -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", p->filename, p->inode, p->mode, p->mdtime, p->size);
+        }//really stupid way of seeing if something is a file
+        else printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", p->filename, p->inode, p->mode, p->mdtime);
+       
         
     }
 
@@ -108,8 +130,11 @@ void getOptions(int argc, char* argv[], Options* t) {
         currentFile curr, * p; 
         p = &curr; 
         t = &options;
+        
         getOptions(argc, argv, t);
+        p->dirname = t->dirname; 
         
-        pTar(t->outfilename, p);
+       // pTar(t->outfilename, p);
+        makeTar(t->outfilename, t->dirname);
         printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit 7c22ef2ecc3de5c7d85ec71076467ed2292cd639
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Tue Mar 19 11:31:14 2024 -0400

    printing is almost done
---
 Labs/Lab2/mytar.c | 25 ++++++++++++++++++++-----
 1 file changed, 20 insertions(+), 5 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index d2b1868..074a772 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -19,6 +19,13 @@ typedef struct {
     char* dirname;
 } Options;
 
+typedef struct {
+    unsigned long int size;
+    unsigned long int inode;
+    unsigned long int mdtime;
+    unsigned  int mode; 
+} currentFile;
+
 /* void printTar(char* outfilename) {
     int sz = 0;
     FILE* st;
@@ -44,7 +51,7 @@ typedef struct {
     printf("something:%d\n", sz);
 }
 */
-void pTar(char* infile) {
+void pTar(char* infile, currentFile* p) {
     int sz, i;
     struct stat buf;
     struct dirent* dp;
@@ -57,10 +64,16 @@ void pTar(char* infile) {
     int mg = *(int*)c;
     if (mg != MAGICNUMBER) exit(-1);
     while (!feof(fd)) {
-        fseek(fd, 4, SEEK_CUR);
         sz = fread(c, 1, 8, fd);
-        mg = *(int*)c;
-        printf("Value is %d\n: ", mg);
+        p->inode = *(int*)c;
+        sz = fread(c, 1, 4, fd);
+        i = *(int*)c;
+        sz = fread(c, 1, i, fd); //i is file name length
+        sz = fread(c, 1, 4, fd); //i is file name length
+        p->mode = *(int*)c;
+        sz = fread(c, 1, 8, fd); //i is file name length
+        p->mdtime = *(int*)c;
+        printf("file inode: %lu, mode: %o, mtime: %lu\n", p->inode, p->mode, p->mdtime);
         
     }
 
@@ -92,9 +105,11 @@ void getOptions(int argc, char* argv[], Options* t) {
 }
     int main(int argc, char* argv[]) {
         Options options, * t;
+        currentFile curr, * p; 
+        p = &curr; 
         t = &options;
         getOptions(argc, argv, t);
         
-        pTar(t->outfilename);
+        pTar(t->outfilename, p);
         printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit c3d9360317e03b774fcc5fc7474614a346249343
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Mon Mar 18 22:18:01 2024 -0400

    started printing
---
 Labs/Lab2/mytar.c | 25 ++++++++++++++++---------
 1 file changed, 16 insertions(+), 9 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index 5fc3f2a..d2b1868 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -19,14 +19,13 @@ typedef struct {
     char* dirname;
 } Options;
 
-void printTar(char* outfilename) {
+/* void printTar(char* outfilename) {
     int sz = 0;
     FILE* st;
     DIR* att;
     int exists;
     char* c;
-    struct stat buf;
-    struct dirent* dp;
+    
     exists = stat(outfilename, &buf);
     printf("%10ld %s\n", buf.st_size, outfilename);
     c = (char*)malloc(buf.st_size * sizeof(char)); 
@@ -44,18 +43,26 @@ void printTar(char* outfilename) {
     sz = fread(c, 4, 1, st);
     printf("something:%d\n", sz);
 }
-
+*/
 void pTar(char* infile) {
     int sz, i;
+    struct stat buf;
+    struct dirent* dp;
     FILE* fd;
-    char* c[100];
+    char* c[100]; //probably have to change this to some variable value based on size of infile
     
     fd = fopen(infile, "r");
     fseek(fd, 0, SEEK_SET);
-    if ((sz = fread(c, 1, 4, fd)) == MAGICNUMBER) printf("LETS GO");
-    int value = *(int*)c;
-    printf("it is%d:\n", value);
-
+    sz = fread(c, 1, 4, fd);
+    int mg = *(int*)c;
+    if (mg != MAGICNUMBER) exit(-1);
+    while (!feof(fd)) {
+        fseek(fd, 4, SEEK_CUR);
+        sz = fread(c, 1, 8, fd);
+        mg = *(int*)c;
+        printf("Value is %d\n: ", mg);
+        
+    }
 
 }
 

commit d84c63480d633579fae982104c58fec5a9254791
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Mon Mar 18 16:54:35 2024 -0400

    able to process the magicnumber
---
 Labs/Lab2/mytar.c | 46 ++++++++++++++++++++++++++++++++++++++++------
 1 file changed, 40 insertions(+), 6 deletions(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index c527cf8..5fc3f2a 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -6,9 +6,13 @@
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
+#include <dirent.h>
 
 #define MAGICNUMBER 1918989421
 
+/*readsize = atoi(outfilename);
+    c = (char*)malloc(readsize * sizeof(char));*/
+
 typedef struct {
     char mode;
     char* outfilename;
@@ -16,16 +20,41 @@ typedef struct {
 } Options;
 
 void printTar(char* outfilename) {
-    int st, sz;
+    int sz = 0;
+    FILE* st;
+    DIR* att;
+    int exists;
     char* c;
-    int readsize; 
-    readsize = atoi(outfilename);
-    c = (char*)malloc(readsize * sizeof(char));
+    struct stat buf;
+    struct dirent* dp;
+    exists = stat(outfilename, &buf);
+    printf("%10ld %s\n", buf.st_size, outfilename);
+    c = (char*)malloc(buf.st_size * sizeof(char)); 
+
     st = fopen(outfilename, "r");
+    att = opendir(outfilename); 
+    while (att) {
+        dp = readdir(att);
+        if (dp != NULL) {
+            exists = stat(dp, &buf);
+            printf("%10ld %s\n", buf.st_size, dp);
+        }
+    }
     if (st < 0) { perror("fopen()"); exit(1); }
     sz = fread(c, 4, 1, st);
-    printf("read(%d,c, 10):\n", st);
+    printf("something:%d\n", sz);
+}
 
+void pTar(char* infile) {
+    int sz, i;
+    FILE* fd;
+    char* c[100];
+    
+    fd = fopen(infile, "r");
+    fseek(fd, 0, SEEK_SET);
+    if ((sz = fread(c, 1, 4, fd)) == MAGICNUMBER) printf("LETS GO");
+    int value = *(int*)c;
+    printf("it is%d:\n", value);
 
 
 }
@@ -36,15 +65,20 @@ void getOptions(int argc, char* argv[], Options* t) {
         switch (o) {
         case 'c':
             t->mode = 'c';
+            break;
         case 't':
             t->mode = 't';
+            break;
         case 'x':
             t->mode = 'x';
+            break;
         case 'f':
             t->mode = 'f';
             t->outfilename = optarg; 
+            break;
         default:
             if(t->outfilename == NULL) printf("Usage: mytar [-cxtf] [TAR]\n");
+            break;
         }
         t->dirname = argv[argc - 1];
     }
@@ -54,6 +88,6 @@ void getOptions(int argc, char* argv[], Options* t) {
         t = &options;
         getOptions(argc, argv, t);
         
-        printTar(t->outfilename);
+        pTar(t->outfilename);
         printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
 }

commit a3604d8af1a3af1fb9da80f971538fc781a5a4fe
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Sat Mar 16 19:38:58 2024 -0400

    going to test some stuff w instructor solution
---
 Labs/Lab2/mytar.c | 23 ++++++++++++++++++++++-
 1 file changed, 22 insertions(+), 1 deletion(-)

diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
index f8fdc28..c527cf8 100644
--- a/Labs/Lab2/mytar.c
+++ b/Labs/Lab2/mytar.c
@@ -2,6 +2,10 @@
 #include <stdio.h>
 #include <stdlib.h>
 #include "getopt.h"
+#include <sys/stat.h>
+#include <sys/types.h>
+#include <fcntl.h>
+#include <unistd.h>
 
 #define MAGICNUMBER 1918989421
 
@@ -11,6 +15,21 @@ typedef struct {
     char* dirname;
 } Options;
 
+void printTar(char* outfilename) {
+    int st, sz;
+    char* c;
+    int readsize; 
+    readsize = atoi(outfilename);
+    c = (char*)malloc(readsize * sizeof(char));
+    st = fopen(outfilename, "r");
+    if (st < 0) { perror("fopen()"); exit(1); }
+    sz = fread(c, 4, 1, st);
+    printf("read(%d,c, 10):\n", st);
+
+
+
+}
+
 void getOptions(int argc, char* argv[], Options* t) {
     unsigned int o; 
     while ((o = getopt(argc, argv, "cxtf:")) != -1) {
@@ -34,5 +53,7 @@ void getOptions(int argc, char* argv[], Options* t) {
         Options options, * t;
         t = &options;
         getOptions(argc, argv, t);
+        
+        printTar(t->outfilename);
         printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
-}
\ No newline at end of file
+}

commit b0ec67293fd1e8b66eaf8b9e770e9020586e8185
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Sat Mar 16 13:27:57 2024 -0400

    started options
---
 Labs/Lab2/Makefile |  6 ++++++
 Labs/Lab2/mytar.c  | 38 ++++++++++++++++++++++++++++++++++++++
 2 files changed, 44 insertions(+)

diff --git a/Labs/Lab2/Makefile b/Labs/Lab2/Makefile
new file mode 100644
index 0000000..d92443b
--- /dev/null
+++ b/Labs/Lab2/Makefile
@@ -0,0 +1,6 @@
+TARGET=mytar
+CFLAGS=-std=gnu99 -Wno-deprecated-declarations -g -Wall
+
+.PHONY: cleanall cleanobj
+
+all: $(TARGET)
\ No newline at end of file
diff --git a/Labs/Lab2/mytar.c b/Labs/Lab2/mytar.c
new file mode 100644
index 0000000..f8fdc28
--- /dev/null
+++ b/Labs/Lab2/mytar.c
@@ -0,0 +1,38 @@
+#include "inodemap.h"
+#include <stdio.h>
+#include <stdlib.h>
+#include "getopt.h"
+
+#define MAGICNUMBER 1918989421
+
+typedef struct {
+    char mode;
+    char* outfilename;
+    char* dirname;
+} Options;
+
+void getOptions(int argc, char* argv[], Options* t) {
+    unsigned int o; 
+    while ((o = getopt(argc, argv, "cxtf:")) != -1) {
+        switch (o) {
+        case 'c':
+            t->mode = 'c';
+        case 't':
+            t->mode = 't';
+        case 'x':
+            t->mode = 'x';
+        case 'f':
+            t->mode = 'f';
+            t->outfilename = optarg; 
+        default:
+            if(t->outfilename == NULL) printf("Usage: mytar [-cxtf] [TAR]\n");
+        }
+        t->dirname = argv[argc - 1];
+    }
+}
+    int main(int argc, char* argv[]) {
+        Options options, * t;
+        t = &options;
+        getOptions(argc, argv, t);
+        printf("Mode: %c, OutFileN: %s, dirname: %s", t->mode, t->outfilename, t->dirname);
+}
\ No newline at end of file

commit 7084df550623440d3c27a5270a0eed5a1097a689
Author: Owen Tucker <owen.tucker@emory.edu>
Date:   Sat Mar 16 10:49:08 2024 -0400

    init lab2
---
 Labs/Lab2/inodemap.c | 31 +++++++++++++++++++++++++++++++
 Labs/Lab2/inodemap.h | 12 ++++++++++++
 2 files changed, 43 insertions(+)

diff --git a/Labs/Lab2/inodemap.c b/Labs/Lab2/inodemap.c
new file mode 100644
index 0000000..f3b4565
--- /dev/null
+++ b/Labs/Lab2/inodemap.c
@@ -0,0 +1,31 @@
+#include "inodemap.h"
+#include <stdio.h>
+#include <stdlib.h>
+
+struct map{
+    ino_t key;
+    void * val;
+};
+
+struct map *Map=NULL;
+unsigned int MapSize=0;
+
+void add_Inode( ino_t ino, void * v ){
+    MapSize++;
+    
+    Map = realloc(Map, sizeof(struct map) * MapSize );
+    
+    Map[MapSize-1].key = ino;
+    Map[MapSize-1].val = v;
+}
+
+void * find_Inode( ino_t ino ){
+
+    for ( int i=0; i<MapSize; i++ ) {
+        if ( Map[i].key == ino ) {
+            return Map[i].val;
+        }
+    }
+    
+    return NULL;
+}
diff --git a/Labs/Lab2/inodemap.h b/Labs/Lab2/inodemap.h
new file mode 100644
index 0000000..cc0a49b
--- /dev/null
+++ b/Labs/Lab2/inodemap.h
@@ -0,0 +1,12 @@
+#ifndef INODEMAP_H
+#define INODEMAP_H
+
+#include <sys/types.h>
+
+/* Add ino to map ino as key and v as associated value */
+void add_Inode( ino_t ino, void * v );
+
+/* Search map for ino, if found return associated value */
+void * find_Inode( ino_t ino );
+
+#endif
