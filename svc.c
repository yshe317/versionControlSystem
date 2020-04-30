#include "svc.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>

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
struct changing{
    int w;
    char* filename;
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
    h->branches[0]->lastnode =NULL;
    h->n_branches = 1;
    h->head = h->branches[0];
    h->ws = (working_space*)malloc(sizeof(working_space));
    h->ws->file_num= 0;
    h->ws->folder = NULL;
    return h;
}

void cleanup(void *helper) {
    // TODO: Implement
    //free the working space
    help* h = (help*)helper;
    for(int i=0;i<h->ws->file_num;i++) {
        free(h->ws->folder[i].filename);
    }
    free(h->ws->folder);
    free(h->ws);
    

    // for(int i =h->n_branches-1;i>=0;i--) {
    //     free(h->branches[i]->branchname);
    //     for(int j =h->branches[i]->size;j>=0;j--) {//keep every first node of branch
    //         free(h->branches[i]->m[j]->commitid);
    //         free(h->branches[i]->m[j]->message);
    //         free(h->branches[i]->m[j]);
    //     }
    // }
    for(int i=0;i < h->n_branches;i++) {
        for(int j = 0;j < h->branches[i]->size;j++) {
            node* e = h->branches[i]->m[j];
            free(e->message);
            free(e->commitid);
            for(int x = 0;x <e->size;x++) {
                free(h->branches[i]->m[j]->files[x].filename);    
            }
            free(h->branches[i]->m[j]->files);
            
            free(h->branches[i]->m[j]);
        }
        free(h->branches[i]->branchname);
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
unsigned long long int sort_num(char* s) {
    unsigned long long int num = 0.00;
    unsigned long long int level = 10000000000000000;
    //65-122
    for(int i = 0;i<strlen(s);i++){//the length of sa
        if(s[i]>=65 && s[i]<=90) {
            
            num = num + ((s[i]-65)*level);
            level = level/100;
        }else if(s[i]>=97 && s[i]<=122) {
            num = num + ((s[i]-97)*level);
            level = level/100;
        }
    }
    //printf("%s:  <%lld>\n",s,num);
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
        for(int j = i+1;j<ws->file_num;j++) {
            // printf("<%f\n",sort_num(ws->folder[i].filename));
            // printf("%f>\n",sort_num(ws->folder[j].filename));
            if(sort_num(ws->folder[i].filename)>sort_num(ws->folder[j].filename)){
                swap(&ws->folder[i],&ws->folder[j]);
            }
        }
    }
}
struct changing* changes(node* n,working_space* ws,int* num){
    struct changing* result = NULL;
    (*num) = 0;
    int j = 0;
    int i = 0;
    for(;i<ws->file_num&&j<n->size;) {
        (*num)++;
        result = (struct changing*)realloc(result,sizeof(struct changing)*(*num));
        
        
        if(strcmp(ws->folder[i].filename,n->files[j].filename)==0) {
            int t = hash_file(NULL,ws->folder[i].filename);
            //when two file is the same 
            if(t!=-2) {
                ws->folder[i].hash = t;
            }
            if(t==-2) {
                result[(*num)-1].w = 0;
            }else if(ws->folder[i].hash==n->files[j].hash) {
                //printf("nochange");
                //keep same 
                result[(*num)-1].w = 0;
            }else{
                //modifi
                //printf("modified");
                result[(*num)-1].w = 1;
            }
            result[(*num)-1].filename = n->files[j].filename;
            i++;
            j++;
        }else if(sort_num(ws->folder[i].filename)>sort_num(n->files[j].filename)) {
            //delete 
            result[(*num)-1].filename = n->files[j].filename;
            result[(*num)-1].w = 2;
            j++;
        }else if(sort_num(ws->folder[i].filename)<sort_num(n->files[j].filename)) {
            //adding
            result[(*num)-1].filename = ws->folder[i].filename;
            result[(*num)-1].w = 3;
            i++;
        }
    }
    if(j == n->size) {
        for(;i<ws->file_num;i++) {
            (*num)++;
            result = (struct changing*)realloc(result,sizeof(struct changing)*(*num));
            result[(*num)-1].filename = ws->folder[i].filename;
            result[(*num)-1].w = 3;
        }
        //the remain in ws is adding
    }else{
        //the remain in node is deleting
        for(;j<n->size;j++) {
            (*num)++;
            result = (struct changing*)realloc(result,sizeof(struct changing)*(*num));
            result[(*num)-1].filename = n->files[j].filename;
            result[(*num)-1].w = 2;
        }
    }
    return result;
}
void save_file(node* n,working_space* ws){
    n->size = ws->file_num;
    n->files = (s_file*)malloc(sizeof(s_file)*n->size);
    for(int i = 0;i<ws->file_num;i++) {
        int temp = strlen(ws->folder[i].filename)+1;
        n->files[i].filename = (char*)malloc(sizeof(char)*temp);
        strcpy(n->files[i].filename,ws->folder[i].filename);
        n->files[i].hash = ws->folder[i].hash;
    }
}

char *svc_commit(void *helper, char *message) {
    // TODO: Implement
    if(message == NULL) {return NULL;}
    //get commit id
    help* h = (help*)helper;
    char* result = NULL;
    int id = 0;
    for(int i = 0;i<strlen(message);i++) {
        id = (id+message[i])%1000;
    }
    
    if(h->head->m==NULL && h->head->lastnode == NULL){//init, first commit 
        if(h->ws->file_num==0){
            return NULL;
        }
        h->head->size++;
        h->head->m = (node**)malloc(sizeof(node*)); //make a node for commit
        h->head->m[0] = (node*)malloc(sizeof(node));
        h->head->m[0]->message = (char*)malloc(sizeof(char)*strlen(message)+1);
        strcpy(h->head->m[0]->message,message);//copy message
        h->head->m[0]->last_node =NULL; //set the last node

        sort_s_file(h->ws);
        save_file(h->head->m[0],h->ws);
        // for(int i=0;i<h->ws->file_num;i++) {
        //     printf("%s\n",h->ws->folder[i].filename);
        // }
        for(int i = 0;i<h->ws->file_num;i++) {//insert the weak file in 
            id+=376591;
            for(int j = 0;j<strlen(h->ws->folder[i].filename);j++) {
                //id = (id * (byte % 37)) % 15485863 + 1;
                id = (id * ((unsigned int)(h->ws->folder[i].filename[j])%37)%15485863+1 );
            }
        }
        //printf("\n%d\n",id);
        result = (char*)malloc(7*sizeof(char));  //maybe the problem that test file will free the result
        sprintf(result,"%06x",id);
        h->head->m[0]->commitid = result; //set the commit id
    }else{
        //when it is not the first time commit
        sort_s_file(h->ws);
        node* lastcommit = h->head->m[h->head->size-1];
        
        int len;
        int same = 1;
        struct changing* change = changes(lastcommit,h->ws,&len); 
        for(int i =0;i<len;i++) {
            
            if(change[i].w!=0) {
                
                same = 0;
                if(change[i].w==1) {//modi
                    id = id + 9573681;
                }else if(change[i].w == 2) {//deleting
                    //printf("the name is : %s and the change is %d\n",change[i].filename,change[i].w);
                    id = id + 85973;
                }else if(change[i].w == 3) {//adding
                    id = id + 376591;
                }
                for(int j = 0;j<strlen(change[i].filename);j++) {
                    id = (id*(change[i].filename[j]%37))%15485863+1;
                }   
            }
        }
        if(same == 1) {//if everything same, return NULL
            free(change);
            return NULL;
        }
        //if can input;
        h->head->size++;
        h->head->m = (node**)realloc(h->head->m,sizeof(node*)*(h->head->size));
        h->head->m[h->head->size-1] = (node*)malloc(sizeof(node));
        h->head->m[h->head->size-1]->message = (char*)malloc(sizeof(char)*strlen(message)+1);
        strcpy(h->head->m[h->head->size-1]->message,message);
        h->head->m[h->head->size-1]->last_node = lastcommit; // set the lastnode
        save_file(h->head->m[h->head->size-1],h->ws);

        result = (char*)malloc(7*sizeof(char));
        sprintf(result,"%06x",id);
        h->head->m[h->head->size-1]->commitid = result; 
        free(change);
    }
    
    return result;
}

void *get_commit(void *helper, char *commit_id) {
    // TODO: Implement
    if(commit_id==NULL) { return NULL; }
    //help* h = (help*)helper;
    //traverse branches

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
    if(branch_name==NULL) { 
        return -1;
    }
    for(int i = 0;i<strlen(branch_name);i++) {
        if((branch_name[i]<=122&&branch_name[i]>=97)||(branch_name[i]<=90&&branch_name[i]>=65)) {
            //while the letter is a-z , A-Z;
        }else if(branch_name[i] == 47||branch_name[i]==45||branch_name[i]==95) {
            //when letter is - / _
        }else{  
            return -1;
        }
    }
    //traverse branches
    help* h = (help*)helper;
    for(int i = 0;i<h->n_branches;i++) {
        if(strcmp(h->branches[i]->branchname,branch_name) == 0) {
            return -2;
        }
    }
    int length;
    struct changing* temp = changes(h->head->m[h->head->size-1],h->ws,&length);
    for(int i = 0;i<length;i++) {
        if(temp[i].w != 0) {
            free(temp);
            return -3;
        }
    }
    free(temp);
    h->n_branches ++;
    h->branches = (branch**)realloc(h->branches, sizeof(branch*)*h->n_branches );
    h->branches[h->n_branches-1] = (branch*) malloc(sizeof(branch));
    branch* focus = h->branches[h->n_branches-1];
    focus->branchname = (char*)malloc(sizeof(char)*(strlen(branch_name)+1));
    strcpy(focus->branchname,branch_name);
    focus->lastnode = h->head->m[h->head->size-1];
    focus->size = 0;
    focus->m = NULL;    
    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement
    if(branch_name == NULL){
        return -1;
    }
    help* h = (help*)helper;

    //check if change with out commit
    int length;
    struct changing* temp;
    if(h->head->size==0) {
        temp = changes(h->head->lastnode,h->ws,&length);
    }else{
        temp = changes(h->head->m[h->head->size-1],h->ws,&length);
    }
    for(int i = 0;i<length;i++) {
        if(temp[i].w != 0) {
            free(temp);
            return -2;
        }
    }
    free(temp);

    int can_not_find = 1;
    for(int i = 0;i<h->n_branches;i++) {
        if(strcmp(branch_name,h->branches[i]->branchname) == 0) {
            can_not_find = 0;
            h->head = h->branches[i];
            return 0;
        }
    }
    if(can_not_find == 1) {
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
