#ifndef HASH_H_
#define HASH_H_

#define HASHSIZE 151

typedef struct Node node;
typedef struct Transfer transfer;
typedef struct Hash_entry hash_entry;
typedef struct Hash_table hash_table;

struct Node
{
	char account_name[100];			//Account name
	int current_balance;			//Account balance
	struct Transfer *transfers;		//Pointer to the first transfer
};

struct Transfer
{
	int ammount;				//Ammount that was transfered
	char account_from[100];			//Name from which money was transfered
	struct Transfer *next_transfer;		//Pointer to the next transfer in the list
};

struct Hash_entry
{
	struct Node *node_pointer;		//Pointer to the node of the hash entry
	struct Hash_entry *next_entry;		//Pointer to next hash entry in a bucket
};

struct Hash_table
{
	int size;				//Size of array
	struct Hash_entry **table;		//Array of pointers that point to hash entries
};

hash_table *create_hash();
unsigned int hash_func(hash_table *hashtable,char hash_name[100]);
hash_entry *search_table(hash_table *hashtable,char hash_name[100]);
int insert_to_table(hash_table *hashtable,char hash_name[100],int init_balance);
int add_transfer_to_list(hash_table *hashtable,char name_send[100],char name_receive[100],int new_ammount);
void delete_table(hash_table *hashtable);
void print_table(hash_table *hashtable);

#endif
