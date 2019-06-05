#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"

//Creates a hash table
hash_table *create_hash()
{
	int i;
	hash_table *new_hashtable;
	if(HASHSIZE<1)
	{
		printf("failure: Can't create hash table with negative or zero size\n");
		exit(EXIT_FAILURE);
	}
	new_hashtable=malloc(sizeof(hash_table));					//Allocate the necessary memory
	if(new_hashtable==NULL)
		return NULL;
	new_hashtable->table=malloc(HASHSIZE*sizeof(hash_entry));
	if(new_hashtable->table==NULL)
		return NULL;
	for(i=0;i<HASHSIZE;i++)
		new_hashtable->table[i]=NULL;
	new_hashtable->size=HASHSIZE;
	return new_hashtable;
}

//Configures the hash value of a key
unsigned int hash_func(hash_table *hashtable,char hash_name[100])
{
	unsigned long hash_value=0;
	int i;
	while(i=*hash_name++)
		hash_value=i+(hash_value<<6)+(hash_value<<16)-hash_value;
	hash_value=hash_value%HASHSIZE;
	return (unsigned int)hash_value;
}

//Returns the entry in the hash table with the specific key
hash_entry *search_table(hash_table *hashtable,char hash_name[100])
{
	hash_entry *entry;
	int hash_value;
	hash_value=hash_func(hashtable,hash_name);					//Search only the bucket with this key's hash value
	for(entry=hashtable->table[hash_value];entry!=NULL;entry=entry->next_entry)
	{
		if(strcmp(hash_name,entry->node_pointer->account_name)==0)		//Returns pointer to the entry in the table
			return entry;
	}
	return NULL;									//If there is no such node in the table
}

//Inserts a new entry to the hash table
int insert_to_table(hash_table *hashtable,char hash_name[100],int init_balance)
{
	hash_entry *entry,*new_entry;
	int hash_value;
	entry=search_table(hashtable,hash_name);					//Checking if there is already a node with the key
	if(entry!=NULL)
	{
		printf("There is already an account with the name %s\n",hash_name);
		return 0;
	}
	else										//There isn't already a node, so we create a new one
	{
		hash_value=hash_func(hashtable,hash_name);				//Which bucket we will insert the node to
		new_entry=malloc(sizeof(hash_entry));
		new_entry->node_pointer=malloc(sizeof(node));
		strcpy(new_entry->node_pointer->account_name,hash_name);
		new_entry->node_pointer->current_balance=init_balance;
		new_entry->node_pointer->transfers=NULL;
		new_entry->next_entry=hashtable->table[hash_value];			//Place it at the beginning of the bucket list
		printf("Added account with name:%s and initial balance:%d\n",hash_name,init_balance);
		hashtable->table[hash_value]=new_entry;
		return 1;
	}
}

int add_transfer_to_list(hash_table *hashtable,char name_send[100],char name_receive[100],int new_ammount)
{
	if(new_ammount<1)
	{
		printf("Invalid ammount\n");
		return 0;
	}
	hash_entry *entry,*entry2;
	transfer *temp_transfer;
	entry2=search_table(hashtable,name_receive);					//Checking that both accounts exist
	if(entry2==NULL)
	{
		printf("There isn't an account with the name %s\n",name_receive);
		return 0;
	}
	entry=search_table(hashtable,name_send);					//Find account that is sending the money
	if(entry==NULL)
	{
		printf("There isn't an account with the name %s\n",name_send);
		return 0;
	}
	if(entry->node_pointer->current_balance>=new_ammount)				//Make sure it has enough money
		entry->node_pointer->current_balance=entry->node_pointer->current_balance-new_ammount;		//And remove it from his balance
	else
	{
		printf("Account %s doesn't have enough money\n",name_send);
		return 0;
	}
	entry2->node_pointer->current_balance=entry2->node_pointer->current_balance+new_ammount;		//Add the ammount to the receiving account
	temp_transfer=entry2->node_pointer->transfers;
	while(temp_transfer!=NULL)							//Search the list of accounts that have sent money to this account
	{
		if(strcmp(temp_transfer->account_from,name_send)==0)			//If this isn't the first time receiving money from the account sending the money
		{
			temp_transfer->ammount=temp_transfer->ammount+new_ammount;
			printf("Added ammount to previously existing transaction\n");
			return 1;
		}
		if(temp_transfer->next_transfer==NULL)
			break;
		temp_transfer=temp_transfer->next_transfer;
	}
	//Creating a new transaction since it doesn't already exist and adding it to the list
	transfer *new_transfer;
	new_transfer=malloc(sizeof(transfer));
	strcpy(new_transfer->account_from,name_send);
	new_transfer->ammount=new_ammount;
	new_transfer->next_transfer=NULL;
	if(entry2->node_pointer->transfers==NULL)					//It is the first time this account has received money
		entry2->node_pointer->transfers=new_transfer;
	else
		temp_transfer->next_transfer=new_transfer;
	printf("Successful new transaction from %s to %s\n",name_send,name_receive);
	return 1;
}

void delete_table(hash_table *hashtable)
{
	int buck;
	hash_entry *entry,*temp_entry;
	transfer *transfer,*temp_transfer;
	if(hashtable==NULL)
		return;
	for(buck=0;buck<hashtable->size;buck++)						//For every bucket
	{
		entry=hashtable->table[buck];
		while(entry!=NULL)							//For every entry in a bucket
		{
			transfer=entry->node_pointer->transfers;
			while(transfer!=NULL)						//Free the list of edges leaving
			{
				temp_transfer=transfer;
				transfer=transfer->next_transfer;
				free(temp_transfer);
			}
			temp_entry=entry;
			entry=entry->next_entry;
			free(temp_entry->node_pointer);					//Free the pointer to the node
			free(temp_entry);						//Free the entry
		}
	}
	free(hashtable->table);
	free(hashtable);								//Free the hashtable
	return;
}

void print_table(hash_table *hashtable)
{
	int buck;
	hash_entry *temp_entry;
	transfer *temp_transfer;
	for(buck=0;buck<hashtable->size;buck++)						//Run the whole graph
	{										//For every bucket
		for(temp_entry=hashtable->table[buck];temp_entry!=NULL;temp_entry=temp_entry->next_entry)					//For every entry in the bucket list
		{
			printf("Account: %s, %d = ",temp_entry->node_pointer->account_name,temp_entry->node_pointer->current_balance);		//For every vertex
			temp_transfer=temp_entry->node_pointer->transfers;
			while(temp_transfer!=NULL)												//Print all the edges leaving
			{
				printf("(%s,%d)",temp_transfer->account_from,temp_transfer->ammount);
				if(temp_transfer->next_transfer!=NULL)
					printf(",");
				temp_transfer=temp_transfer->next_transfer;
			}
			printf("\n");
		}
	}
}
