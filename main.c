#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TEXT 1024*1024
#define MAX_TAG 1024

#define HEADER "<!DOCTYPE html> <html><head><link rel='stylesheet' href='/main.css'><meta name='viewport' content='width=device-width, initial-scale=1'></head><body><article>"

#define FOOTER "</article></body></html>"

#define FIND_MATCH(match, str) \
    } else if (strcmp(text+1,match) == 0) { \
        fgets(text, MAX_TEXT, inFile); \
        char *ln = strchr(text,'\n'); \
        if (!ln) break; \
        ln[0] = '\0'; \
        snprintf(outText,MAX_TEXT,str,text,text); \

struct tag {
    char tag[MAX_TEXT];
    char body[MAX_TEXT];
};

struct tag *tags;

struct tag *findTag(char *tag) {
    for (int i = 0; i < MAX_TEXT; i++) {
        if (strcmp(tags[i].tag,tag) == 0) return &tags[i];
        if (tags[i].tag[0] == '\0') {
            strcpy(tags[i].tag,tag);
            snprintf(tags[i].body,MAX_TEXT,"<h1>Tag @%s</h1>",tag);
            return &tags[i];
        }
    }
}

void gen(char *basePath, char *inPathI, char *outPathI) {
    char *path, *text, *outText, *title, *desc;

    // this is probably awful
    path = malloc(PATH_MAX);
    text = malloc(MAX_TEXT);
    outText = malloc(MAX_TEXT);
    title = malloc(MAX_TEXT);   
    desc = malloc(MAX_TEXT);
    
    int matchTotal = 0;
    snprintf(path,PATH_MAX,"%s/!%s.html",outPathI,basePath);
    
    FILE *outFile = fopen(path,"w");
   
    memset(path,0,PATH_MAX); 
    snprintf(path,PATH_MAX,"%s/%s",inPathI,basePath);
    FILE *inFile = fopen(path,"r");

    if (!inFile || !outFile) return;

    fputs(HEADER,outFile);

    while (fgets(text, MAX_TEXT, inFile)) {
        snprintf(outText,MAX_TEXT,"%s",text);

        if (text[0] == '\n' || text[0] == '\0') continue;

        if (text[0] != '#') {
            snprintf(outText,MAX_TEXT,"%s<p>%s</p>",outText,text);

            FIND_MATCH("TITLE\n","<h1>%s</h1>")
            snprintf(title,MAX_TEXT,"%s",text);

            FIND_MATCH("TAG\n","<a href='@%s.html'>@%s</a>")
            struct tag *found = findTag(text);          
            char *body = found->body;
            snprintf(strchr(body,'\0'),MAX_TEXT,"<a href='!%s.html'><h3>%s</h3></a><p><i>%s</i></p></p>\n", basePath, title, desc);

            FIND_MATCH("DESC\n","<i>%s</i>");
            snprintf(desc,MAX_TEXT,"%s",text);
        }

        fputs(outText,outFile);
    }

    fputs(FOOTER,outFile);

    free(text);
    free(outText);
    free(title);
    free(desc);
}

void list(char *basePath, char *outPath) {
    char *path = malloc(PATH_MAX);
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir) {
        printf("directory not found: %s\n",basePath);
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            char *name = dp->d_name;
            printf("iterated over %s\n", name);
            gen(name,basePath,outPath);
            printf("finalized over %s\n", name);    
        }   
    }

    closedir(dir);

    free(path);
}

void tag(char *outPathI) {
    tags = malloc(sizeof(struct tag) * MAX_TAG);

    for (int i = 0; i < MAX_TEXT; i++) {
        char *outPath = malloc(PATH_MAX);

        char *tag = tags[i].tag;
        if (tag[0] == '\0') break;
        
        snprintf(outPath,PATH_MAX,"%s/@%s.html",outPathI,tag);
    
        FILE *outFile = fopen(outPath,"w");
        fputs(HEADER,outFile);
        fputs(tags[i].body,outFile);
        fputs(FOOTER,outFile);
    
        fclose(outFile);
    
        free(outPath);
    }

    free(tags);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: %s [input dir] [output dir]\n",argv[0]);
        return 1;
    }

    mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    list(argv[1], argv[2]);
    printf("done iterating, starting tags\n");
    tag(argv[2]);
    printf("done everything\n");
    fflush(stdout);
    return 0;
}

//TODO: cleanup
