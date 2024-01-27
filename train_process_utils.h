#ifndef LINKED_LIST_H
#define LINKED_LIST_H


// LINKED LIST -------------------

// Self referential structure to create node.
typedef struct tmp
{
	char* item;
	struct tmp*  next;
} Node;

// Structure to create linked list.
typedef struct
{
	Node* head;
	Node* tail;

} List;

// Create node and return reference of it.
Node* create_node(char* item);
// Initialize List
void init_list(List * lp);
// Add new item at the end of list.
void add_at_tail(List* lp, char* item);
// Delete item from Start of list.
void del_from_head(List * lp);
// Print the list
void print_list(List* lp);

//  -------------------------------



#endif