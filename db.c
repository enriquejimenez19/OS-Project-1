#include "db.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


Node_t *Node_constructor(char *arg_name, char *arg_value, Node_t *arg_left, Node_t *arg_right) {
	Node_t *new_node = (Node_t *)malloc(sizeof(Node_t));
	if (new_node == 0)
		return 0;
	if ((new_node->name = (char *)malloc(strlen(arg_name)+1)) == 0) {
		free(new_node);
		return 0;
	}
	if ((new_node->value = (char *)malloc(strlen(arg_value)+1)) == 0) {
		free(new_node->name);
		free(new_node);
		return 0;
	}
	strcpy(new_node->name, arg_name);
	strcpy(new_node->value, arg_value);
	new_node->lchild = arg_left;
	new_node->rchild = arg_right;
	return new_node;
}


void Node_destructor(Node_t *node) {
	if (node->name != 0)
		free(node->name);
	if (node->value != 0)
		free(node->value);
	free(node);
}


Node_t *search(char *, Node_t *, Node_t **);

void query(char *name, char *result, int len) {
	Node_t *target;
	target = search(name, head, 0);
	if (target == 0) {
		strncpy(result, "not found", len-1);
		return;
	} else {
		strncpy(result, target->value, len-1);
		return;
	}
}

int add(char *name, char *value) {
	Node_t *parent;
	Node_t *target;
	Node_t *newnode;

	if ( head == NULL ) {
		head = Node_constructor(name,value,0,0);
		return (1);
	}

	if ((target = search(name, head, &parent)) != 0) {
		return(0);
	}

	newnode = Node_constructor(name, value, 0, 0);

	if (strcmp(name, parent->name) < 0)
		parent->lchild = newnode;
	else
		parent->rchild = newnode;

	return(1);
}

int xremove(char *name) {
	Node_t *parent;
	Node_t *dnode;
	Node_t *next;

	// first, find the node to be removed
	if ((dnode = search(name, head, &parent)) == 0)
		return(0);
	

	if ( dnode == head ) {
		Node_destructor(head);
		head = NULL;
		return (1);
	}

	// we found it.  Now check out the easy cases.  If the node has no
	// right child, then we can merely replace its parent's pointer to
	// it with the node's left child.

	if (dnode->rchild == 0) {
		if (strcmp(dnode->name, parent->name) < 0)
			parent->lchild = dnode->lchild;
		else
			parent->rchild = dnode->lchild;

		// done with dnode
		Node_destructor(dnode);
	} else if (dnode->lchild == 0) {
		// ditto if the node had no left child
		if (strcmp(dnode->name, parent->name) < 0)
			parent->lchild = dnode->rchild;
		else
			parent->rchild = dnode->rchild;

		// done with dnode
		Node_destructor(dnode);
	} else {
		// So much for the easy cases ...
		// We know that all nodes in a node's right subtree have lexicographically
		// greater names than the node does, and all nodes in a node's left subtree
		// have lexicographically smaller names than the node does. So, we find
		// the lexicographically smallest node in the right subtree and replace
		// the node to be deleted with that node. This new node thus is
		// lexicographically smaller than all nodes in its right subtree, and
		// greater than all nodes in its left subtree. Thus the modified tree
		// is well formed.
		
		Node_t **pnext = &dnode->rchild;
			// pnext is the address of the field within the parent of next that
			// points to next.
		next = dnode->rchild;
		while (next->lchild != 0) {
			// work our way down the lchild chain, finding the smallest node
			// in the subtree. 
			Node_t *nextl = next->lchild;
			pnext = &next->lchild;
			next = nextl;
		}
		strcpy(dnode->name, next->name);
		strcpy(dnode->value, next->value);
		*pnext = next->rchild;
		Node_destructor(next);
	}

	return(1);
}

Node_t* search(char *name, Node_t *parent, Node_t **parentpp) {
	// Search the tree, starting at parent, for a node containing
	// name (the "target node").  Return a pointer to the node,
	// if found, otherwise return 0.  If parentpp is not 0, then it points
	// to a location at which the address of the parent of the target node
	// is stored.  If the target node is not found, the location pointed to
	// by parentpp is set to what would be the the address of the parent of
	// the target node, if it were there.

	// Assumptions:
	// parent is not null and it does not contain name

	if ( head == NULL ) 
		return 0;

	if ( strcmp(name, head->name) == 0 ){
		parentpp = NULL;
		return head;
	}

	Node_t *next;
	Node_t *result;

	if (strcmp(name, parent->name) < 0) {
		if ((next = parent->lchild) == 0) {
			result = 0;
		} else {
			if (strcmp(name, next->name) == 0) {
				result = next;
			} else {
				result = search(name, next, parentpp);
				return result;
			}
		}
	} else {
		if ((next = parent->rchild) == 0) {
			result = 0;
		} else {
			if (strcmp(name, next->name) == 0) {
				result = next;
			} else {
				result = search(name, next, parentpp);
				return result;
			}
		}
	}

	if (parentpp != 0)
		*parentpp = parent;

	return(result);
}

void interpret_command(char *command, char *response, int len) {
	char value[256];
	char ibuf[256];
	char name[256];
	
	if (strlen(command) <= 1) {
		strncpy(response, "ill-formed command", len-1);
		return;
	}

	// which command is it?
	switch (command[0]) {
	case 'q':
		 // Query
		sscanf(&command[1], "%255s", name);
		if (strlen(name) == 0){
			strncpy(response, "ill-formed command", len-1);
			return;
		}
		query(name, response, len);
		if (strlen(response) == 0) {
			strncpy(response, "not found", len-1);
			return;
		} else
			return;

		break;

	case 'a':
		// Add to the database
		sscanf(&command[1], "%255s %255s", name, value);
		if ((strlen(name) == 0) || (strlen(value) == 0)) {
			strncpy(response, "ill-formed command", len-1);
			return;
		}
		if (add(name, value)) {
			strncpy(response, "added", len-1);
			return;
		} else {
			strncpy(response, "already in database", len-1);
			return;
		}

	case 'd':
		// Delete from the database
		sscanf(&command[1], "%255s", name);
		if (strlen(name) == 0) {
			strncpy(response, "ill-formed command", len-1);
			return;
		}
		
		if (xremove(name)) {
			strncpy(response, "removed", len-1);
			return;
		} else {
			strncpy(response, "not in database", len-1);
			return;
		}

	case 'f':
		// process the commands in a file (silently)
		sscanf(&command[1], "%255s", name);
		if (name[0] == '\0') {
			strncpy(response, "ill-formed command", len-1);
			return;
		}
		{
			FILE *finput = fopen(name, "r");
			if (!finput) {
				strncpy(response, "bad file name", len-1);
				return;
			}
			while(fgets(ibuf, sizeof(ibuf), finput) != 0) {
				interpret_command(ibuf, response, len);
			}
			fclose(finput);
		}
		strncpy(response, "file processed", len-1);
		return;

	default:
		strncpy(response, "ill-formed command", len-1);
		return;
	}
}
