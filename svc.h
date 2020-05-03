#ifndef svc_h
#define svc_h

#include <stdlib.h>
typedef struct node node;
typedef struct{
    int hash;
    char* filename;
}   s_file;

struct changing{
    int w;
    char* filename;
};
struct node{ // every node is a commit
    char* commitid;
    char* message;
    node* last_node;
    node* mother;
    int size;
    int n_change;
    struct changing* changes;
    s_file* files;

};


typedef struct{ // branch
    char* branchname;
    node* lastnode;
    node** m;
    int size;
}branch;



typedef struct{ // current focus place
    int file_num;
    s_file* folder;
}working_space;

typedef struct{
    branch** branches;
    int n_branches;
    branch* head;
    working_space* ws;
    node* reset_node;
}help;

typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char *file_name;
    char *resolved_file;
} resolution;
void *svc_init(void);

void cleanup(void *helper);

int hash_file(void *helper, char *file_path);

char *svc_commit(void *helper, char *message);

void *get_commit(void *helper, char *commit_id);

char **get_prev_commits(void *helper, void *commit, int *n_prev);

void print_commit(void *helper, char *commit_id);

int svc_branch(void *helper, char *branch_name);

int svc_checkout(void *helper, char *branch_name);

char **list_branches(void *helper, int *n_branches);

int svc_add(void *helper, char *file_name);

int svc_rm(void *helper, char *file_name);

int svc_reset(void *helper, char *commit_id);

char *svc_merge(void *helper, char *branch_name, resolution *resolutions, int n_resolutions);

#endif

