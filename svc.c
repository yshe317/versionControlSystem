#include "svc.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>


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
    h->reset_node = NULL;
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
            free(e->changes);
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

//10233/a.txt,a.txt
int copyFile(char *in, char *out) {
    FILE* fin = NULL;
    fin = fopen(in,"r");
    //printf("ss");
    if(fin == NULL) {
        return -1;
    }
    FILE* fout = fopen(out,"w");
    //printf("%s and %s",in,out);
    int c;
    while((c = fgetc(fin)) != EOF)
        fputc(c,fout);
    fclose(fin);
    fclose(fout);
    return 0;
}
void localize_file(s_file* file,char* id) {
    char* current = (char*)malloc(sizeof(char)*(strlen(file->filename)+8));
    strcpy(current,id);
    mkdir(current,0777);
    char* temp = "\0";
    char* temp_filename;
    temp_filename = strdup(file->filename);
    char* ready = strtok(temp_filename,"/");//deep copy the filename first
    while(ready != NULL) {
        strcat(current,temp);
        mkdir(current,0777);
        strcat(current,"/");
        temp = ready;
        ready = strtok(NULL, "/");
    }
    strcat(current,temp);
    copyFile(file->filename,current);
    free(current);
    free(temp_filename);
    //printf("\nthe number is %d\n",a);
}

void localize(node* n,char* id) {
    for(int i = 0;i<n->size;i++) {
        localize_file(&n->files[i],id);
    }
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
            // printf("%s\n",ws->folder[i].filename);
            // printf("%s\n",ws->folder[j].filename);
            // printf("<%d>\n",strcasecmp(ws->folder[i].filename,ws->folder[j].filename));
            if(strcasecmp(ws->folder[i].filename,ws->folder[j].filename)>0){
                swap(&ws->folder[i],&ws->folder[j]);
            }
            // printf("%s\n",ws->folder[i].filename);
            // printf("%s\n",ws->folder[j].filename);
        }
    }

}
struct changing* changes(node* n,working_space* ws,int* num) {
    struct changing* result = NULL;
    (*num) = 0;
    int j = 0;
    int i = 0;
    for(;i<ws->file_num&&j<n->size;) {
        (*num)++;
        result = (struct changing*)realloc(result,sizeof(struct changing)*(*num));
        int t = hash_file(NULL,ws->folder[i].filename);
        if(t!=-2) {
            ws->folder[i].hash = t;
        }
        if(strcasecmp(ws->folder[i].filename,n->files[j].filename)==0) {
            if(t==-2) {
                result[(*num)-1].w = 2;
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
        }else if(strcasecmp(ws->folder[i].filename,n->files[j].filename)>0) {
            //delete
            result[(*num)-1].filename = n->files[j].filename;
            result[(*num)-1].w = 2;
            j++;
        }else if(strcasecmp(ws->folder[i].filename,n->files[j].filename)<0) {
            //adding
            //for adding thing that be delete in local file

            result[(*num)-1].filename = ws->folder[i].filename;
            result[(*num)-1].w = 3;
            int t = hash_file(NULL,ws->folder[i].filename);
            if(t==-2) {
                result[(*num)-1].w = 99;//skip number
            }
            i++;
        }
    }
    if(j == n->size) {
        for(;i<ws->file_num;i++) {
            (*num)++;
            result = (struct changing*)realloc(result,sizeof(struct changing)*(*num));
            result[(*num)-1].filename = ws->folder[i].filename;
            result[(*num)-1].w = 3;
            int t = hash_file(NULL,ws->folder[i].filename);
            if(t==-2) {
                result[(*num)-1].w = 99;//skip number
            }
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
void save_file(node* n,working_space* ws) {
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
    for(int i = 0;i<h->ws->file_num;i++) {
        h->ws->folder[i].hash = hash_file(h,h->ws->folder[i].filename);
        if(h->ws->folder[i].hash == -2) {
            svc_rm(h,h->ws->folder[i].filename);
        }
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
        h->head->m[0]->last_node = NULL; //set the last node
        h->head->m[0] ->father = NULL;
        h->head->m[0]->n_change = h->ws->file_num;
        h->head->m[0]->mother = NULL;

        sort_s_file(h->ws);

        save_file(h->head->m[0],h->ws);

        h->head->m[0] -> changes = (struct changing*)malloc(sizeof(struct changing)*h->ws->file_num);
        for(int i = 0;i<h->ws->file_num;i++) {
            h->head->m[0]->changes[i].filename = h->head->m[0]->files[i].filename;
            h->head->m[0]->changes[i].w = 3;
            int t = hash_file(NULL,h->ws->folder[i].filename);
            if(t==-2) {
                svc_rm(h,h->head->m[0]->changes[i].filename);
                h->head->m[0]->changes[i].w = 99;//skip number
            }
        }

        //things wrong here
        for(int i = 0;i<h->ws->file_num;i++) {//insert the weak file in
            if(h->head->m[0]->changes[i].w == 99) {
               continue;
            }
            //printf("\n%s\n",h->ws->folder[i].filename);
            id+=376591;
            for(int j = 0;j<strlen(h->ws->folder[i].filename);j++) {
                //id = (id * (byte % 37)) % 15485863 + 1;
                //printf("%d, ",(h->ws->folder[i].filename[j]));
                id = (id * ((unsigned int)(h->ws->folder[i].filename[j])%37)%15485863+1 );
            }
        }
        //printf("<%d>",id);
        //printf("\n%d\n",id);
        result = (char*)malloc(7*sizeof(char));  //maybe the problem that test file will free the result
        sprintf(result,"%06x",id);
        h->head->m[0]->commitid = result; //set the commit id
    }else{
        //when it is not the first time commit
        sort_s_file(h->ws);
        node* lastcommit;
        if(h->head->size == 0) {
            lastcommit = h->head->lastnode;
        }else{
            lastcommit = h->head->m[h->head->size-1];
        }
        int len;
        int same = 1;
        struct changing* change;
        if(h->reset_node == NULL) {
          change = changes(lastcommit,h->ws,&len);
        }else {
          change = changes(h->reset_node,h->ws,&len);
        }
        
        for(int i =0;i<len;i++) {
            if(change[i].w!=0) {

                same = 0;
                if(change[i].w==1) {//modi
                    id = id + 9573681;
                }else if(change[i].w == 2) {//deleting
                    //printf("the name is : %s and the change is %d\n",change[i].filename,change[i].w);
                    svc_rm(h,change[i].filename);
                    id = id + 85973;
                }else if(change[i].w == 3) {//adding
                    id = id + 376591;
                }else if(change[i].w == 99) {
                    svc_rm(h,change[i].filename);
                    continue;
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
        // for(int i=0;i<len;i++) {
        //     printf("%s\n",change[i].filename);
        // }
        //if can input;
        h->head->size++;
        h->head->m = (node**)realloc(h->head->m,sizeof(node*)*(h->head->size));
        h->head->m[h->head->size-1] = (node*)malloc(sizeof(node));
        h->head->m[h->head->size-1]->message = (char*)malloc(sizeof(char)*strlen(message)+1);
        h->head->m[h->head->size-1]->mother = NULL;
        strcpy(h->head->m[h->head->size-1]->message,message);
        h->head->m[h->head->size-1]->last_node = lastcommit; // set the lastnode
        if(h->reset_node == NULL) {
            h->head->m[h->head->size-1]->father = lastcommit;
        }else{
            h->head->m[h->head->size-1]->father = h->reset_node;
        }
        
        save_file(h->head->m[h->head->size-1],h->ws);
        h->reset_node = NULL;
        result = (char*)malloc(7*sizeof(char));
        sprintf(result,"%06x",id);
        h->head->m[h->head->size-1]->commitid = result;
        h->head->m[h->head->size-1] ->n_change = len;
        h->head->m[h->head->size-1]->changes = (struct changing*)malloc(sizeof(struct changing)*len);
        
        for(int i = 0;i<len;i++) {
            h->head->m[h->head->size-1]->changes[i].w = change[i].w;
            if(change[i].w != 3) {
                h->head->m[h->head->size-1]->changes[i].filename = change[i].filename;
            }else{
                for(int j = 0;j<h->head->m[h->head->size-1]->size;j++) {
                    if(strcmp(change[i].filename, h->head->m[h->head->size-1]->files[j].filename) == 0) {
                        h->head->m[h->head->size-1]->changes[i].filename = h->head->m[h->head->size-1]->files[j].filename;
                        break;
                    }
                }
            }
        }
        free(change);
    }
    localize(h->head->m[h->head->size-1],result);
    return result;
}

void *get_commit(void *helper, char *commit_id) {
    // TODO: Implement
    if(commit_id==NULL) { return NULL; }

    help* h = (help*)helper;
    //traverse branches
    for(int i = 0; i<h->n_branches;i++) {
        for(int j = 0; j<h->branches[i]->size;j++) {
            if(strcmp(commit_id,h->branches[i]->m[j]->commitid) == 0) {
                return h->branches[i]->m[j];
            }
        }
    }
    return NULL;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    // TODO: Implement
    if(n_prev == NULL) { return NULL; }
    (*n_prev) = 0;
    if(commit == NULL) { return NULL; }
    node* n = (node*)commit;
    char** ls = NULL;
    if(n->father!=NULL) {
        (*n_prev)++;
        ls = (char**)realloc(ls,sizeof(char*)*(*n_prev));
        ls[(*n_prev)-1] = n->father->commitid;
    }
    if(n->mother!=NULL) {
        (*n_prev)++;
        ls = (char**)realloc(ls,sizeof(char*)*(*n_prev));
        ls[(*n_prev)-1] = n->mother->commitid;
    }
    //printf("%s\n",n->commitid);
    
    // while(n->last_node!=NULL) {
    //     n = n->last_node;
    //     (*n_prev)++;
    //     ls = (char**)realloc(ls,sizeof(char*)*(*n_prev));
    //     ls[(*n_prev)-1] = n->commitid;
    // }
    return ls;
}

void print_change(struct changing* c,node* n) {
    if(c->w == 99) {

    }else if(c->w == 0) {

    }else if(c->w == 2) {
        printf("    %c %s\n",'-',c->filename);
    }else if(c->w == 1) {
        int i;
        for(i = 0;i< n ->size;i++) {
            if(strcmp(c->filename,n->files[i].filename) == 0) {
                break;
            }
        }

        int j;
        for(j = 0;j<n->last_node->size;j++) {
            if(strcmp(c->filename,n->last_node->files[j].filename) == 0) {
                break;
            }
        }
        printf("    %c %s [%10d -> %10d]\n",'/',c->filename,n->last_node->files[j].hash,n->files[i].hash);
    }else if(c->w == 3) {
        printf("    %c %s\n",'+',c->filename);
    }
}
void print_commit(void *helper, char *commit_id) {
    // TODO: Implement
    if(commit_id == NULL) {
        printf("Invalid commit id\n");
        return;
    }
    help* h = (help*)helper;
    int can_not_find = 1;
    for(int i=0;i<h->n_branches;i++) {
        for(int j = 0; j < h->branches[i]->size; j++) {
            if(strcmp(commit_id,h->branches[i]->m[j]->commitid) == 0) {
                can_not_find = 0;
                printf("%s [%s]: %s\n",commit_id,h->branches[i]->branchname,h->branches[i]->m[j]->message);
                for(int x = 0;x<h->branches[i]->m[j]->n_change;x++) {
                    print_change(&h->branches[i]->m[j]->changes[x],h->branches[i]->m[j]);//wrong order
                }
                printf("\n    Tracked files (%d):\n",h->branches[i]->m[j]->size);
                for(int x = 0;x<h->branches[i]->m[j]->size;x++) {
                    printf("    [%10d] %s\n",h->branches[i]->m[j]->files[x].hash,h->branches[i]->m[j]->files[x].filename);
                }
            }
        }
    }
    if(can_not_find == 1) {
        printf("Invalid commit id\n");
        return;
    }
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
        }else if(branch_name[i]<=57&&branch_name[i]>=48) {
            //when letter is 0-9
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
    struct changing* temp;
    if(h->head->size==0) {
        temp = changes(h->head->lastnode,h->ws,&length);
    }else{
        temp = changes(h->head->m[h->head->size-1],h->ws,&length);
    }
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
            if(h->head->size==0) {
                svc_reset(helper,h->head->lastnode->commitid);
            }else{
                svc_reset(helper,h->head->m[h->head->size-1]->commitid);
            }
            return 0;
        }
    }
    if(can_not_find == 1) {
        return -1;
    }
    if(h->head->size==0) {
        svc_reset(helper,h->head->lastnode->commitid);
    }
    // }else{
    //     svc_reset(helper,h->);
    // }


    return 0;
}

char **list_branches(void *helper, int *n_branches) {
    // TODO: Implement
    if(n_branches==NULL) { return NULL; }

    help* h = (help*)helper;
    (*n_branches) = h->n_branches;
    char** result = NULL;
    for(int i = 0;i<(*n_branches);i++) {
        result = (char**)realloc(result,sizeof(char*)*(i+1));
        result[i] = h->branches[i]->branchname;
        printf("%s\n",result[i]);
    }
    return result;
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
    if(commit_id == NULL) {
        return -1;
    }
    help* h = (help*)helper;
    node* commit;
    if(h->head->size==0) {
        commit = h->head->lastnode;
    }else{
        commit = h->head->m[h->head->size-1];
    }
    while(commit!=NULL) {
        if(strcmp(commit_id,commit->commitid) == 0) {
            break;
        }
        commit = commit -> last_node;
    }
    // for(int i = 0; i<h->n_branches;i++) {
    //     for(int j = 0; j<h->branches[i]->size;j++) {
    //         if(strcmp(commit_id,h->branches[i]->m[j]->commitid) == 0) {
    //             commit = h->branches[i]->m[j];
    //         }
    //     }
    // }

    if(commit == NULL) {
        return -2;
    }
    for(int i=0;i<h->ws->file_num;i++) {
        free(h->ws->folder[i].filename);
    }
    free(h->ws->folder);
    h->ws->folder = NULL;
    h->ws->file_num = commit->size;
    h->ws->folder = (s_file*)malloc(sizeof(s_file)*commit->size);
    h->reset_node = commit;
    char* path = NULL;
    for(int i = 0;i<h->ws->file_num; i++) {
        path = (char*)malloc(sizeof(char)*(strlen(commit_id)+strlen(commit->files[i].filename)+2));
        strcpy(path, commit_id);
        strcat(path,"/");
        strcat(path,commit->files[i].filename);
        h->ws->folder[i].filename = strdup(commit->files[i].filename);
        h->ws->folder[i].hash = commit->files[i].hash;
        copyFile(path,h->ws->folder[i].filename);
        free(path);
    }

    //copy local things to ws

    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement
    if(branch_name == NULL) {
        printf("Invalid branch name\n");
        return NULL;
    }
    help* h = (help*)helper;
    if(strcmp(h->head->branchname, branch_name) == 0) {
        printf("Cannot merge a branch with itself\n");
        return NULL;
    }

    branch* target =NULL;
    for(int i = 0; i < h->n_branches; i++) {
        if(strcmp(h->branches[i]->branchname, branch_name) == 0) {
            target = h->branches[i];
        }
    }
    if(target == NULL) {
        printf("Branch not found\n");
        return NULL;
    }

    // for(int i = 0;i<h->ws->file_num;i++) {
    //     h->ws->folder[i].hash = hash_file(h,h->ws->folder[i].filename);
    //     //printf("%s\n",h->ws->folder[i].filename);
    //     if(h->ws->folder[i].hash == -2) {
    //         svc_rm(h,h->ws->folder[i].filename);
    //     }
    // }
    // printf("|||||||||\n");
    // for(int i = 0;i<h->head->m[h->head->size-1]->size;i++) {
    //     printf("%s\n",h->head->m[h->head->size-1]->files[i].filename);
    // }
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
            printf("Changes must be committed\n");
            return NULL;
        }
    }
    free(temp);



    temp = changes(target->m[target->size-1],h->ws,&length);
    char* path = NULL;
    for(int i = 0;i<length;i++) {
        if(temp[i].w == 2) {
            path = (char*)malloc(sizeof(char)*(strlen(temp[i].filename)+8));
            strcpy(path,target->m[target->size-1]->commitid);
            strcat(path,"/");
            strcat(path,temp[i].filename);
            copyFile(path,temp[i].filename);
            free(path);
            svc_add(helper,temp[i].filename);
            //move thing to local ws
        }
        if(temp[i].w == 1 || temp[i].w == 0) {
            for(int i =0;i<n_resolutions;i++) {
                if(strcmp(temp[i].filename, resolutions[i].file_name)==0) {
                    copyFile(resolutions[i].resolved_file,resolutions[i].file_name);
                }
            }
        }

    }
    free(temp);
    

    printf("Merge successful\n");



    char* message = (char*)malloc(sizeof(char)*(15+strlen(branch_name)));
    strcpy(message,"Merged branch ");
    strcat(message,branch_name);
    char* result = svc_commit(helper,message);
    free(message);
    node* newcommit = (node*)get_commit(helper,result);
    newcommit -> mother = target->m[target->size-1];
    if (strcmp("631bf5",result)==0)
    {
        print_commit(helper,result);
        print_commit(helper,target->m[target->size-1]->commitid);
        print_commit(helper,h->head->m[h->head->size-1]->commitid);
    }
    
    return result;
}
