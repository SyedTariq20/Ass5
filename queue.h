#include <stdio.h>
#include <stdlib.h>
int item;

struct Build_Node {
	int data;
	struct Build_Node *nxt;
};
 
struct Queue {
	struct Build_Node *fnt;
	struct Build_Node *lst;
	unsigned int sze;
};
 
void init(struct Queue *queue) {
	queue->fnt = NULL;
	queue->lst = NULL;
	queue->sze = 0;
}
 

int pop(struct Queue *queue) {
	if(queue->sze == 0)
    {
        return -1;
    }
	item = queue->fnt->data;
	queue->sze--;
    
	struct Build_Node *tmp = queue->fnt;
	queue->fnt = queue->fnt->nxt;
	free(tmp);
	return item;
}
 
void push(struct Queue *queue, int data) {
	queue->sze++;
 
	if (queue->fnt == NULL) {
		queue->fnt = (struct Build_Node *) malloc(sizeof(struct Build_Node));
		queue->fnt->data = data;
		queue->fnt->nxt = NULL;
		queue->lst = queue->fnt;
	} else {
		queue->lst->nxt = (struct Build_Node *) malloc(sizeof(struct Build_Node));
		queue->lst->nxt->data = data;
		queue->lst->nxt->nxt = NULL;
		queue->lst = queue->lst->nxt;
	}
}
 

