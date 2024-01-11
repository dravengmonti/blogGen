#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define MAX_PATH 16384
#define MAX_TAG 1024
#define MAX_TAGS 1024
#define MAX_DIR 1024

#define HEADER "<!DOCTYPE html> <html><head><link rel='stylesheet' href='/main.css'></head><body><article>"

#define FOOTER "</article></body></html>"

#define FIND_MATCH(match, str) \
	} else if (strcmp(text+1,match) == 0) { \
		fgets(text, MAX_PATH, inFile); \
		strchr(text,'\n')[0] = '\0'; \
		snprintf(outText,MAX_PATH,str,text,text); \

struct tag {
	char tag[MAX_TAG];
	char body[MAX_PATH];
};

struct tag tags[MAX_TAGS];

struct tag *findTag(char *tag) {
	for (int i = 0; i < MAX_TAGS; i++) {
		if (strcmp(tags[i].tag,tag) == 0) return &tags[i];
		if (tags[i].tag[0] == '\0') {
			strcpy(tags[i].tag,tag);
			snprintf(tags[i].body,MAX_PATH,"<h1>Tag @%s</h1>",tag);
			return &tags[i];
		}
	}
}

void gen(char *basePath) {
	char outPath[MAX_PATH], inPath[MAX_PATH], text[MAX_PATH], outText[MAX_PATH], title[MAX_PATH], desc[MAX_PATH];
	int matchTotal = 0;
	snprintf(outPath,MAX_PATH,"./out/!%s.html",basePath);
	snprintf(inPath,MAX_PATH,"./in/%s",basePath);

	FILE *inFile = fopen(inPath,"r");
	FILE *outFile = fopen(outPath,"w");
	
	if (!inFile || !outFile) return;

	fputs(HEADER,outFile);

	while (fgets(text, MAX_PATH, inFile)) {
		snprintf(outText,MAX_PATH,"%s<",text);

		if (text[0] != '#') {
			snprintf(outText,MAX_PATH,"%s<p>%s</p>",outText,text);

			FIND_MATCH("TITLE\n","<h1>%s</h1>")
			snprintf(title,MAX_PATH,"%s",text);

			FIND_MATCH("TAG\n","<a href='@%s.html'>@%s</a>")
			struct tag *found = findTag(text);			
			char *body = found->body;
			snprintf(strchr(body,'\0'),MAX_PATH,"<a href='!%s.html'><h3>%s</h3></a><p><i>%s</i></p></p>\n", basePath, title, desc);

			FIND_MATCH("DESC\n","<i>%s</i>");
			snprintf(desc,MAX_PATH,"%s",text);
		}

		fputs(outText,outFile);
	}

	fputs(FOOTER,outFile);
}

void list(char *basePath) {
    char path[MAX_DIR];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

	if (!dir) {
		printf("directory not found: ./in\n");
		return;
	}

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            char *name = dp->d_name;
			printf("iterated over %s\n", name);
        	gen(name);
			printf("finalized over %s\n", name);	
		}	
    }

    closedir(dir);
}

void tag() {
	for (int i = 0; i < MAX_TAGS; i++) {
		char outPath[MAX_PATH];

		char *tag = tags[i].tag;
		if (tag[0] == '\0') break;
		
		snprintf(outPath,MAX_PATH,"./out/@%s.html",tag);
	
		FILE *outFile = fopen(outPath,"w");
		fputs(HEADER,outFile);
		fputs(tags[i].body,outFile);
		fputs(FOOTER,outFile);
	
	}
}

int main() {
	mkdir("./out", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	list("./in");
	printf("done iterating, starting tags\n");
	tag();
	printf("done everything\n");
	fflush(stdout);
	return 0;
}

//TODO: cleanup
