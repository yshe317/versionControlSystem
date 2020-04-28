#include "svc.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
typedef struct node node;
typedef struct{
    int hash;
    char* filename;
}s_file;

struct node{ // every node is a commit
    char* commitid;
    char* message;
    node* last_node;
    int size;
    s_file* files;
};

typedef struct{ // branch
    char* branchname;
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
}help;

void *svc_init(void) {
    // TODO: Implement
    // help* helper = malloc(sizeof(help));
    // return helper;
    help* h = (help*)malloc(sizeof(help));

    h->branches = (branch**)malloc(sizeof(branch*));  // create space for master
    h->branches[0] = (branch*)malloc(sizeof(branch));
    h->branches[0]->branchname = (char*)malloc(sizeof(char)*7);
    strcpy(h->branches[0]->branchname,"master\0");
    h->branches[0]->size = 0;
    h->branches[0]->m = NULL;
    h->n_branches = 1;


    h->ws = (working_space*)malloc(sizeof(working_space));
    h->ws->file_num= 0;
    h->ws->folder = NULL;
    return h;
}

void cleanup(void *helper) {
    // TODO: Implement
    help* h = (help*)helper;
    for(int i=0;i<h->ws->file_num;i++) {
        free(h->ws->folder[i].filename);
    }
    free(h->ws->folder);
    free(h->ws);

    for(int i =h->n_branches-1;i>=0;i--) {
        free(h->branches[i]->branchname);
        for(int j =h->branches[i]->size-1;j>0;j++) {//keep every first node of branch
            free(h->branches[i]->m[j]->commitid);
            free(h->branches[i]->m[j]->message);
            free(h->branches[i]->m[j]);
        }
    }
    if(h->branches[0]->m!=NULL){
        free(h->branches[0]->m[0]->commitid); //delete the firstnode in master
        free(h->branches[0]->m[0]->message);
    }
    for(int i=0;i<h->n_branches;i++){
        free(h->branches[i]->m);
        free(h->branches[i]);
    }
    free(h->branches);
    free(h);
}

int hash_file(void *helper, char *file_path) {
    // TODO: Implement
    if(file_path==NULL){
        return -1;
    }
    FILE * fin =NULL;
    fin = fopen(file_path,"r");
    if(fin==NULL) {  //if the file_path do not exist
        return -2;
    }
    int hash = 0;
    for(int i = 0;i<strlen(file_path);i++) {
        hash = hash+file_path[i];
        hash = hash%1000;
    }
    int temp = fgetc(fin);
    while(temp!=EOF) {
        hash += (unsigned char)temp;
        hash = hash%2000000000;
        temp = fgetc(fin);
    }
    fclose(fin);
    return hash;
}
float sort_num(char* s) {
    float num = 0;
    float level = 1;
    for(int i = 0;i<strlen(s);i++){//the length of sa
        num = num + (s[i]/level);
        level = level*10;
    }
    return num;
}
void swap(s_file* a,s_file*b) {
    char* temp;
    temp = a->filename;
    a->filename = b->filename;
    b->filename = temp;
    int tempnum;
    tempnum = a->hash;
    a->hash = b->hash;
    b->hash = tempnum;
}
void sort_s_file(working_space* ws) {
    for(int i = 0;i<ws->file_num;i++) {
        for(int j = i;j<ws->file_num;j++) {
            if(sort_num(ws->folder[i].filename)>sort_num(ws->folder[j].filename)){
                swap(&ws->folder[i],&ws->folder[j]);
            }
        }
    }
}

char *svc_commit(void *helper, char *message) {
    // TODO: Implement
    //get commit id
    help* h = (help*)helper;
    char* result = NULL;
    int id = 0;
    for(int i = 0;i<strlen(message);i++) {
        id = (id+message[i])%1000;
    }
    if(h->head->m==NULL){//init, first commit 
        h->head->m = (node**)malloc(sizeof(node*)); //make a node for commit
        h->head->m[0] = (node*)malloc(sizeof(node));
        h->head->m[0]->size = h->ws->file_num; //copy n_files
        h->head->m[0]->message = (char*)malloc(sizeof(char)*strlen(message)+1);
        strcpy(h->head->m[0]->message,message);//copy message
        h->head->m[0]->last_node =NULL; //set the last node
        h->head->m[0]->files = (s_file*)malloc(h->head->m[0]->size*sizeof(s_file));//create memory for weak_file
        sort_s_file(h->ws);
        id = 0;
        for(int i = 0;i<h->ws->file_num;i++) {//insert the weak file in 
            id+=376591;
            for(int j = 0;j<strlen(h->ws->folder[i].filename);j++) {
                //id = (id * (byte % 37)) % 15485863 + 1;
                id = (id * (h->ws->folder[i].filename[j]%37) ) % 15485863 + 1;
            }
        }
        result = (char*)malloc(7*sizeof(char));  //maybe the problem that test file will free the result
        itoa(id,result,16);
        h->head->m[0]->commitid = result; //set the commit id
    }
    return result;
}

void *get_commit(void *helper, char *commit_id) {
    // TODO: Implement
    return NULL;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    // TODO: Implement
    return NULL;
}

void print_commit(void *helper, char *commit_id) {
    // TODO: Implement
}

int svc_branch(void *helper, char *branch_name) {
    // TODO: Implement
    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement
    if(branch_name == NULL){
        return -1;
    }
    return 0;
}

char **list_branches(void *helper, int *n_branches) {
    // TODO: Implement
    return NULL;
}

int svc_add(void *helper, char *file_name) {
    // TODO: Implement
    if(file_name == NULL){
        return -1;
    }
    FILE* fin = NULL;
    fin = fopen(file_name,"r");
    if(fin == NULL){
        return -3;
    }
    // add things into svc
    int exist = -1;
    help* h = (help*)helper;
    // if the file has the same name in the working space
    for(int i = 0;i<h->ws->file_num;i++) {
        if(strcmp(h->ws->folder[i].filename,file_name)==0) {  
            exist = i;
            break;
        }
    }
    if(exist == -1) {//if file do not exist, put it in
        h->ws->file_num++;
        h->ws->folder = (s_file*)realloc(h->ws->folder,sizeof(s_file)*h->ws->file_num);
        //copy the file name 
        h->ws->folder[h->ws->file_num-1].filename = (char*)malloc(sizeof(char)*(strlen(file_name)+1));
        strcpy(h->ws->folder[h->ws->file_num-1].filename,file_name);
        h->ws->folder[h->ws->file_num-1].hash = hash_file(NULL,file_name); // save the hash
    }else{
        return -2;
    }
    return h->ws->folder[h->ws->file_num-1].hash;
}

int svc_rm(void *helper, char *file_name) {
    // TODO: Implement
    help* h = (help*)helper;
    if(file_name==NULL){
        return -1;
    }
    int exist = -1;
    for(int i = 0;i<h->ws->file_num;i++){
        if(strcmp(file_name,h->ws->folder[i].filename)==0){
            exist = i;
            break;
        }
    }
    int hash  =0;
    if(exist==-1){
        return -2;
    }else{
        hash = h->ws->folder[exist].hash;
        free(h->ws->folder[exist].filename);
        for(int i = exist+1;i<h->ws->file_num;i++){
            h->ws->folder[i-1] = h->ws->folder[i]; 
        }
        h->ws->file_num--;
        h->ws->folder = (s_file*)realloc(h->ws->folder,h->ws->file_num*sizeof(s_file));
    }
    return hash;
}

int svc_reset(void *helper, char *commit_id) {
    // TODO: Implement
    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement
    return NULL;
}
