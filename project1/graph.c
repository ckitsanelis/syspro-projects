#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"
#include "stack.h"

graph *create_graph(int size)
{
	graph *new_graph=malloc(sizeof(graph));
	new_graph->hashtable=create_hash(size);
	return new_graph;
}

void delete_graph(graph *mygraph)
{
	delete_table(mygraph->hashtable);		//Free the hashtable
	free(mygraph);					//Free the graph
	printf("success: Cleaned memory\n");
	return;
}

void create_node(hash_table *hashtable,unsigned long key)
{
	hash_entry *temp_entry;
	temp_entry=search_table(hashtable,key);		//To search if there is already an entry in the hashtable with this key
	if(temp_entry==NULL)				//There isn't
	{
		insert_to_table(hashtable,key);
		printf("success: created %li\n",key);
		return;
	}
	else
	{
		printf("failure: There is already a node in the graph with the key %li\n",key);
		return;
	}
}

void delete_node(hash_table *hashtable,unsigned long num)
{
	hash_entry *del_entry;
	del_entry=search_table(hashtable,num);
	if(del_entry==NULL)
	{
		printf("failure: There isn't a node in the graph with the key %li\n",num);
		return;
	}
	else
	{
		if(del_entry->node_pointer->edges_leaving!=NULL || del_entry->node_pointer->edges_coming!=NULL)
		{
			printf("failure: Can't delete %li as there are still transactions to be done\n",num);
			return;
		}
		int hashval;
		hashval=hash_func(hashtable,num);
		hash_entry *temp_entry;
		if(num==hashtable->table[hashval]->node_pointer->key)		//Node to be deleted is at the head of the bucket list
		{
			if(del_entry->next_entry!=NULL)			//There are other nodes
			{
				temp_entry=hashtable->table[hashval]->next_entry;
				hashtable->table[hashval]=temp_entry;
			}
			else			//Only node in bucket list
				hashtable->table[hashval]=NULL;
		}
		else
		{
			temp_entry=hashtable->table[hashval];
			while(temp_entry->next_entry!=del_entry)
				temp_entry=temp_entry->next_entry;
			if(del_entry->next_entry==NULL)		//Node to be deleted is at the end of the bucket list
				temp_entry->next_entry=NULL;
			else		//Node to be deleted is in the middle of the bucket list
			{
				temp_entry->next_entry=del_entry->next_entry;
				del_entry->next_entry=NULL;
			}
		}
		free(del_entry->node_pointer->edges_leaving);	//Freeing memory used by the entry
		free(del_entry->node_pointer->edges_coming);
		free(del_entry->node_pointer);
		free(del_entry);
		printf("success: deleted %li\n",num);
		return;
	}
}

void add_tran(hash_table *hashtable,unsigned long source_key,unsigned long dest_key,double ammount)
{
	hash_entry *source_entry,*dest_entry;
	source_entry=search_table(hashtable,source_key);	//Go to the correct bucket list
	dest_entry=search_table(hashtable,dest_key);
	if(source_entry==NULL)		//Check that both nodes exists
	{
		printf("failure: There isn't a node in the graph with the key %li\n",source_key);
		return;
	}
	if(dest_entry==NULL)
	{
		printf("failure: There isn't a node in the graph with the key %li\n",dest_key);
		return;
	}
	edge *new_edge_leaving;
	edge *new_edge_coming;
	new_edge_leaving=source_entry->node_pointer->edges_leaving;
	new_edge_coming=dest_entry->node_pointer->edges_coming;
	while(new_edge_leaving!=NULL)	//Check if there is already a transaction between these nodes
	{
		if(new_edge_leaving->node_head==source_key && new_edge_leaving->node_tail==dest_key)		//There is already an edge with the same directed nodes
		{
			new_edge_leaving->weight=new_edge_leaving->weight+ammount;
			break;
		}
		if(new_edge_leaving->next_edge==NULL)		//At the end of the list
			break;
		new_edge_leaving=new_edge_leaving->next_edge;
	}
	while(new_edge_coming!=NULL)
	{
		if(new_edge_coming->node_head==source_key && new_edge_coming->node_tail==dest_key)		//There is already an edge with the same directed nodes
		{
			new_edge_coming->weight=new_edge_coming->weight+ammount;
			printf("success: Added ammount %.0f to already existing transaction\n",ammount);
			return;
		}
		if(new_edge_coming->next_edge==NULL)
			break;
		new_edge_coming=new_edge_coming->next_edge;
	}
	//There isn't a transaction, so we create 2 new ones and place them in the correct list
	edge *new_edge=malloc(sizeof(edge));
	new_edge->next_edge=NULL;
	new_edge->weight=ammount;
	new_edge->node_head=source_key;
	new_edge->node_tail=dest_key;
	if(source_entry->node_pointer->edges_leaving==NULL)	//It is the first edge in the list
		source_entry->node_pointer->edges_leaving=new_edge;
	else			//There are other edges in the list
		new_edge_leaving->next_edge=new_edge;
	edge *new_edge2=malloc(sizeof(edge));
	new_edge2->next_edge=NULL;
	new_edge2->weight=ammount;
	new_edge2->node_head=source_key;
	new_edge2->node_tail=dest_key;
	if(dest_entry->node_pointer->edges_coming==NULL)		//It is the first edge in the list
		dest_entry->node_pointer->edges_coming=new_edge2;
	else
		new_edge_coming->next_edge=new_edge2;
	printf("success: Added transaction between %li and %li with ammount %.0f\n",source_key,dest_key,ammount);
}


void del_tran(hash_table *hashtable,unsigned long source_key,unsigned long dest_key)
{
	hash_entry *source_entry,*dest_entry;
	source_entry=search_table(hashtable,source_key);
	dest_entry=search_table(hashtable,dest_key);
	if(source_entry==NULL)
	{
		printf("failure: There isn't a node in the graph with the key %li\n",source_key);
		return;
	}
	if(dest_entry==NULL)
	{
		printf("failure: There isn't a node in the graph with the key %li\n",dest_key);
		return;
	}
	//Remove transaction from list of edges leaving of the source entry
	edge *before_rem_edge;
	edge *rem_edge;
	before_rem_edge=source_entry->node_pointer->edges_leaving;
	rem_edge=source_entry->node_pointer->edges_leaving;
	if(rem_edge==NULL)		//Check if source entry has any edges leaving
	{
		printf("Node with key %li has no transactions\n",source_key);
		return;
	}
	while(rem_edge!=NULL)		//Finding the transaction we want inside the list of edges leaving
	{
		if(rem_edge->node_head==source_key && rem_edge->node_tail==dest_key)
			break;
		if(rem_edge->next_edge==NULL)			//After running the whole list, we couldn't find the transaction we wanted
		{
			printf("failure: There isn't a transaction between %li and %li\n",source_key,dest_key);
			return;
		}
		rem_edge=rem_edge->next_edge;
	}
	if(rem_edge==before_rem_edge)		//Transaction is at the start of the list
	{
		source_entry->node_pointer->edges_leaving=rem_edge->next_edge;
	}
	else if(rem_edge->next_edge==NULL)		//Transaction is at the end of the list
	{
		while(before_rem_edge->next_edge!=rem_edge)
			before_rem_edge=before_rem_edge->next_edge;
		before_rem_edge->next_edge=NULL;
	}
	else			//Transaction is at the middle of the list
	{
		while(before_rem_edge->next_edge!=rem_edge)
			before_rem_edge=before_rem_edge->next_edge;
		before_rem_edge->next_edge=rem_edge->next_edge;
	}
	free(rem_edge);
	//Remove transaction from list of edges coming into the destination entry
	rem_edge=dest_entry->node_pointer->edges_coming;
	before_rem_edge=dest_entry->node_pointer->edges_coming;
	while(rem_edge!=NULL)		//No need to check, since we know for sure that the transaction exists
	{
		if(rem_edge->node_head==source_key && rem_edge->node_tail==dest_key)
			break;
		rem_edge=rem_edge->next_edge;
	}
	if(rem_edge==before_rem_edge)		//Transaction is at the start of the list
	{
		dest_entry->node_pointer->edges_coming=rem_edge->next_edge;

	}
	else if(rem_edge->next_edge==NULL)		//Transaction is at the end of the list
	{
		while(before_rem_edge->next_edge!=rem_edge)
			before_rem_edge=before_rem_edge->next_edge;
		before_rem_edge->next_edge=NULL;
	}
	else			//Transaction is at the middle of the list
	{
		while(before_rem_edge->next_edge!=rem_edge)
			before_rem_edge=before_rem_edge->next_edge;
		before_rem_edge->next_edge=rem_edge->next_edge;
	}
	free(rem_edge);
	printf("success: Deleted transaction between %li and %li\n",source_key,dest_key);
}

void lookup(graph *mygraph,int mode,unsigned long key)
{
	double ammount=0;
	hash_entry *temp_entry;
	edge *temp_edge;
	temp_entry=search_table(mygraph->hashtable,key);
	if(temp_entry==NULL)
	{
		printf("failure: There isn't a node in the graph with the key %li\n",key);
		return;
	}
	if(mode==0)		//lookup in
	{
		temp_edge=temp_entry->node_pointer->edges_coming;
		while(temp_edge!=NULL)		//Run the list of edges leaving
		{
			ammount=ammount+temp_edge->weight;	//Add the weight of every edge
			temp_edge=temp_edge->next_edge;
		}
		printf("success: In(%li) = %.0f\n",key,ammount);
	}
	else if(mode==1)	//lookup out
	{
		temp_edge=temp_entry->node_pointer->edges_leaving;
		while(temp_edge!=NULL)
		{
			ammount=ammount+temp_edge->weight;
			temp_edge=temp_edge->next_edge;
		}
		printf("success: Out(%li) = %.0f\n",key,ammount);
	}
	else			//lookup sum=in-out
	{
		temp_edge=temp_entry->node_pointer->edges_coming;
		while(temp_edge!=NULL)
		{
			ammount=ammount+temp_edge->weight;	//Add the weight of every edge coming
			temp_edge=temp_edge->next_edge;
		}
		temp_edge=temp_entry->node_pointer->edges_leaving;
		while(temp_edge!=NULL)
		{
			ammount=ammount-temp_edge->weight;	//Subtract the weight of every edge leaving
			temp_edge=temp_edge->next_edge;
		}
		printf("success: Sum(%li) = %.0f\n",key,ammount);
	}
}

void unmark_nodes(graph *mygraph)
{
	int buck;
	hash_entry *unmark_entry;
	for(buck=0;buck<mygraph->hashtable->size;buck++)	//Run all of the entries
	{
		for(unmark_entry=mygraph->hashtable->table[buck];unmark_entry!=NULL;unmark_entry=unmark_entry->next_entry)
			unmark_entry->node_pointer->visited=0;	//Change visited to 0
	}
}


void triangle(graph *mygraph,unsigned long num,double limit)
{
	hash_entry *source_entry;
	stack_node *head_node;
	source_entry=search_table(mygraph->hashtable,num);
	if(source_entry==NULL)		//There isn't a node in the graph with the source key
	{
		printf("failure: There isn't a node in the graph with the key %li\n",num);
		return;
	}
	else
		head_node=create_stack(num);		//Create the stack
	triangle_func(mygraph,source_entry,source_entry,head_node,0,limit);
	if(head_node->success==0)
		printf("No triangles found\n");
	unmark_nodes(mygraph);
	delete_stack(head_node);
}

void triangle_func(graph *mygraph,hash_entry *current_entry,hash_entry *source_entry,stack_node *head_node,int checked,double limit)
{
	hash_entry *temp_entry;
	edge *current_edge;
	if(checked>0)
		push(current_entry->node_pointer->key,head_node);
	if(checked==3 && current_entry==source_entry)	//Check if successful triangle
	{
		pop(head_node);
		if(head_node->success==0)			//First triangle we find
		{
			printf("success: Triangle(%li,%.0f)\n",source_entry->node_pointer->key,limit);
			head_node->success=1;
		}
		print_stack(head_node);
		return;
	}
	if(current_entry->node_pointer->visited==1)	//Check if we have already checked this entry
	{
		pop(head_node);
		return;
	}
	current_entry->node_pointer->visited=1;
	if(checked<3)				//If we still haven't reached the depth we want
	{
		for(current_edge=current_entry->node_pointer->edges_leaving;current_edge!=NULL;current_edge=current_edge->next_edge)
		{
			if(current_edge->weight>=limit)		//Check if it satisfies the minimum edge weight
			{
				temp_entry=search_table(mygraph->hashtable,current_edge->node_tail);
				triangle_func(mygraph,temp_entry,source_entry,head_node,checked+1,limit);
			}
		}
	}
	current_entry->node_pointer->visited=0;
	if(checked>0)
		pop(head_node);
}


void allcycles(graph *mygraph,unsigned long num)
{
	hash_entry *source_entry;
	stack_node *head_node;
	source_entry=search_table(mygraph->hashtable,num);
	if(source_entry==NULL)		//There isn't a node in the graph with the source key
	{
		printf("failure: There isn't a node in the graph with the key %li\n",num);
		return;
	}
	else
		head_node=create_stack(num);		//Create the stack
	allcycles_func(mygraph,source_entry,source_entry,head_node,0,num);
	if(head_node->success==0)
		printf("No cycle found for %li\n",num);
	unmark_nodes(mygraph);
	delete_stack(head_node);
}

void allcycles_func(graph *mygraph,hash_entry *current_entry,hash_entry *source_entry,stack_node *head_node,int checked,unsigned long num)
{
	hash_entry *temp_entry;
	edge *current_edge;
	if(checked>0)
		push(current_entry->node_pointer->key,head_node);
	if(checked>2 && current_entry==source_entry)	//Check if successful cycle
	{
		pop(head_node);
		if(head_node->success==0)			//This is the first cycle we find
		{
			printf("success: Cycles %li =\n",num);
			head_node->success=1;
		}
		print_stack(head_node);
		return;
	}
	if(current_entry->node_pointer->visited==1)	//Check if we have already checked this entry
	{
		pop(head_node);
		return;
	}
	current_entry->node_pointer->visited=1;
	if(current_entry->node_pointer->edges_leaving==NULL)
		return;
	for(current_edge=current_entry->node_pointer->edges_leaving;current_edge!=NULL;current_edge=current_edge->next_edge)		//Search all neighbors
	{
		temp_entry=search_table(mygraph->hashtable,current_edge->node_tail);
		allcycles_func(mygraph,temp_entry,source_entry,head_node,checked+1,num);
	}
	current_entry->node_pointer->visited=0;		//To find all cycles
	if(checked>0)
		pop(head_node);
}

void connected_nodes(graph *mygraph,unsigned long source_key,unsigned long dest_key)
{
	hash_entry *source_entry;
	stack_node *head_node;
	source_entry=search_table(mygraph->hashtable,source_key);
	if(source_entry==NULL)		//There isn't a node in the graph with the source key
	{
		printf("failure: There isn't a node in the graph with the key %li\n",source_key);
		return;
	}
	else
		head_node=create_stack(source_key);		//Create the stack
	search_path(mygraph,dest_key,source_entry,0,head_node);
	if(head_node->success==0)
		printf("conn(%li,%li) not found\n",source_key,dest_key);
	unmark_nodes(mygraph);
	delete_stack(head_node);
}

//Using recursion to search the whole graph
void search_path(graph *mygraph,unsigned long dest_key,hash_entry *current_entry,int checked,stack_node *head_node)
{
	hash_entry *temp_entry;
	edge *current_edge;
	if(checked>0)
		push(current_entry->node_pointer->key,head_node);
	if(current_entry->node_pointer->visited==1)	//Check if we have already checked this entry
	{
		pop(head_node);
		return;
	}
	if(current_entry->node_pointer->key==dest_key)	//Check if this is the entry we want
	{
		if(head_node->success==0)
		{
			printf("success: Conn(%li,%li) = ",head_node->key,dest_key);
			head_node->success=1;
		}
		print_stack(head_node);
	}
	current_entry->node_pointer->visited=1;
	for(current_edge=current_entry->node_pointer->edges_leaving;current_edge!=NULL;current_edge=current_edge->next_edge)		//Search all neighbors
	{
		temp_entry=search_table(mygraph->hashtable,current_edge->node_tail);
		search_path(mygraph,dest_key,temp_entry,checked+1,head_node);
	}
	if(checked>0)
		pop(head_node);
}

void traceflow(graph *mygraph,unsigned long num,int depth)
{
	hash_entry *source_entry;
	stack_node *head_node;
	source_entry=search_table(mygraph->hashtable,num);
	if(source_entry==NULL)		//There isn't a node in the graph with the source key
	{
		printf("failure: There isn't a node in the graph with the key %li\n",num);
		return;
	}
	else
	{
		if(source_entry->node_pointer->edges_leaving==NULL)			//No need to do anything else if no edges leave from this node
		{
			printf("There are no paths from %li\n",num);
			return;
		}
		head_node=create_stack(num);		//Create the stack
	}
	traceflow_func(mygraph,source_entry,source_entry,head_node,0,depth,0);
	if(head_node->success==0)
		printf("No paths were found from %li with depth %d\n",num,depth);
	unmark_nodes(mygraph);
	delete_stack(head_node);
}

void traceflow_func(graph *mygraph,hash_entry *source_entry,hash_entry *current_entry,stack_node *head_node,int current_depth,int depth,double ammount)
{
	double new_ammount;
	hash_entry *temp_entry;
	edge *current_edge;
	if(current_depth>0)		//So we don't push the head twice
		push(current_entry->node_pointer->key,head_node);
	if(current_entry->node_pointer->visited==1)	//Check if we have already checked this entry
	{
		pop(head_node);
		return;
	}
	current_entry->node_pointer->visited=1;
	if(current_depth==depth)
	{
		if(head_node->success==0)		//If this is the first path
		{
			printf("success: Traceflow(%li,%d) =\n",source_entry->node_pointer->key,depth);
			head_node->success=1;
		}
		trace_print(head_node,ammount);
	}
	if(current_depth<depth)		//We haven't reached the depth we want
	{
		for(current_edge=current_entry->node_pointer->edges_leaving;current_edge!=NULL;current_edge=current_edge->next_edge)		//Search all neighbors
		{
			new_ammount=ammount+current_edge->weight;		//Add to over ammount of money
			temp_entry=search_table(mygraph->hashtable,current_edge->node_tail);
			traceflow_func(mygraph,source_entry,temp_entry,head_node,current_depth+1,depth,new_ammount);
		}
	}
	current_entry->node_pointer->visited=0;
	if(current_depth>0)		//Delete stack handles the head node
		pop(head_node);
}

void print_graph(graph *mygraph)
{
	int buck;
	hash_entry *temp_entry;
	edge *temp_edge;
	for(buck=0;buck<mygraph->hashtable->size;buck++)		//Run the whole graph
	{				//For every bucket
		for(temp_entry=mygraph->hashtable->table[buck];temp_entry!=NULL;temp_entry=temp_entry->next_entry)			//For every entry in the bucket list
		{
			printf("Vertex(%li) = ",temp_entry->node_pointer->key);		//For every vertex
			temp_edge=temp_entry->node_pointer->edges_leaving;
			while(temp_edge!=NULL)		//Print all the edges leaving
			{
				printf("(%li,%.0f)",temp_edge->node_tail,temp_edge->weight);
				if(temp_edge->next_edge!=NULL)
					printf(",");
				temp_edge=temp_edge->next_edge;
			}
			printf("\n");
		}
	}
}

void dump(graph *mygraph,char *filename)
{
	FILE *fp;
	fp=fopen(filename,"w");
	int buck;
	hash_entry *temp_entry;
	edge *temp_edge;
	//Run the whole hashtable and write a createnode command in a file for each entry in the table
	for(buck=0;buck<mygraph->hashtable->size;buck++)
	{
		for(temp_entry=mygraph->hashtable->table[buck];temp_entry!=NULL;temp_entry=temp_entry->next_entry)
		{
			fprintf(fp,"createnodes %li\n",temp_entry->node_pointer->key);
		}
	}
	//Then in each entry
	for(buck=0;buck<mygraph->hashtable->size;buck++)
	{
		for(temp_entry=mygraph->hashtable->table[buck];temp_entry!=NULL;temp_entry=temp_entry->next_entry)
		{
			//Write a addtran command for each edge in the edges leaving list
			temp_edge=temp_entry->node_pointer->edges_leaving;
			while(temp_edge!=NULL)
			{
				fprintf(fp,"addtran %li %li %.0f\n",temp_edge->node_head,temp_edge->node_tail,temp_edge->weight);
				temp_edge=temp_edge->next_edge;
			}
		}
	}
	printf("Created file\n");
	fclose(fp);
}
