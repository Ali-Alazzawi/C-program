#include <stdio.h>
#include <stdlib.h>
#include "train_process_utils.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>



// LINKED LIST -----------
void init_list(List * lp)
{
	lp->head = NULL;
	lp->tail = NULL;
}


Node* create_node(char* item)
{
	Node* nNode;

	nNode = (Node*) malloc(sizeof(Node));

	nNode->item = item;
	nNode->next = NULL;

	return nNode;
}


void add_at_tail(List* lp, char* item)
{
	Node* node;
	node = create_node(item);

	//if list is empty.
	if(lp->head == NULL)
	{
		lp->head = node;
		lp->tail = node;
	}
	else
	{
		lp->tail->next  = node;
		lp->tail = lp->tail->next;
	}		
}

void del_from_head(List * lp)
{
	char* item = NULL;

	if(lp->head == NULL)
	{	
		printf("\nList is Empty ..."); 	
		return;
	}
	else
	{
		item = lp->head->item;
		lp->head = lp->head->next;
        free(item);
	}	
}

void print_list(List* lp)
{
	Node* node;

	if(lp->head == NULL)
	{
		printf("List: \t<Empty List>\n");
		return;
	}
	
	node = lp->head;

	printf("List: \t"); 
	while(node != NULL)
	{
		printf("| %s |", node->item);
		node = node->next;

		if(node !=NULL)
			printf("--->");
	}
	printf("\n");
}

// -----------------------


