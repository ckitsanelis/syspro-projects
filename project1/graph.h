#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "stack.h"

//Create/Delete Graph
graph *create_graph(int size);
void delete_graph(graph *mygraph);

//Create/Delete nodes
void create_node(hash_table *hashtable,unsigned long key);
void delete_node(hash_table *hashtable,unsigned long num);

//Create/Delete edges
void add_tran(hash_table *hashtable,unsigned long source_key,unsigned long dest_key,double ammount);
void del_tran(hash_table *hashtable,unsigned long source_key,unsigned long dest_key);

//Graph functions
void unmark_nodes(graph *mygraph);
void lookup(graph *mygraph,int mode,unsigned long key);

void triangle(graph *mygraph,unsigned long num,double limit);
void triangle_func(graph *mygraph,hash_entry *current_entry,hash_entry *source_entry,stack_node *head_node,int checked,double limit);

void allcycles(graph *mygraph,unsigned long num);
void allcycles_func(graph *mygraph,hash_entry *current_entry,hash_entry *source_entry,stack_node *head_node,int checked,unsigned long num);

void connected_nodes(graph *mygraph,unsigned long source_key,unsigned long dest_key);
void search_path(graph *mygraph,unsigned long dest_key,hash_entry *current_entry,int checked,stack_node *head_node);

void traceflow(graph *mygraph,unsigned long num,int depth);
void traceflow_func(graph *mygraph,hash_entry *source_entry,hash_entry *current_entry,stack_node *head_node,int current_depth,int depth,double ammount);

void print_graph(graph *mygraph);
void dump(graph *mygraph,char *filename);
