#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "hash.h"

int main (int argc, char* argv[])
{
	if(argc!=5 && argc!=3)			//Must have exactly 3 or 5 arguments
	{
		printf("failure: Need 3 or 5 arguments \n");
		exit(EXIT_FAILURE);
	}
	int hash_table_size;
	FILE *fp;
	char *token;
	const char s[3]=" \n";		//To use in strtok
	char line[256];
	int word=0;			//To count number of words in a line

	if(argc==3 && argv[1][0]=='-' && argv[1][1]=='b')
		hash_table_size=atoi(argv[2]);
	else if(argc==5 && argv[1][0]=='-' && argv[1][1]=='b' && argv[3][0]=='-' && argv[3][1]=='o')
	{
		hash_table_size=atoi(argv[2]);
		fp=fopen(argv[4],"r");		//File is argv[4]
		if(fp==NULL)
		{
			printf("There is no file with the name %s\n",argv[4]);
			exit(EXIT_FAILURE);
		}
	}
	else if(argc==5 && argv[1][0]=='-' && argv[1][1]=='o' && argv[3][0]=='-' && argv[3][1]=='b')
	{
		hash_table_size=atoi(argv[4]);
		fp=fopen(argv[2],"r");		//File is argv[2]
		if(fp==NULL)
		{
			printf("There is no file with the name %s\n",argv[2]);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("failure: Incorrect arguments \n");
		exit(EXIT_FAILURE);
	}

	int graph_exists=1;		//Flag to know if we have a graph
	graph *mygraph;
	mygraph=create_graph(hash_table_size);
	if(argc==5)			//If we have to read first from a file
	{
		while(fgets(line,256,fp)!=NULL)		//Read every line
		{
			token=strtok(line,s);			//Strtok use is similar to the example at tutorialspoint.com
			if(graph_exists==0)		//Create a graph, if the previous one was freed
			{
				graph *mygraph;
				mygraph=create_graph(hash_table_size);
				graph_exists=1;
			}
			if(strcmp(token,"createnodes")==0)	//And compare the first word with the available commands
			{
				unsigned long id;
				while(token!=NULL)
				{
					word++;
					if(word>1)		//Create a node for all numbers
					{
						if(strlen(token)!=7)
							printf("failure:Key must be 7 digit number\n");
						else
						{
							sscanf(token,"%li",&id);
							create_node(mygraph->hashtable,id);
						}
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"delnodes")==0)
			{
				unsigned long id;
				while(token!=NULL)
				{
					word++;
					if(word>1)
					{
						sscanf(token,"%li",&id);
						delete_node(mygraph->hashtable,id);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"addtran")==0)
			{
				unsigned long id1;
				unsigned long id2;
				double ammount;
				while(token!=NULL)
				{
					word++;
					if(word==2)
						sscanf(token,"%li",&id1);
					else if(word==3)
						sscanf(token,"%li",&id2);
					else if(word==4)
					{
						sscanf(token,"%lf",&ammount);
						add_tran(mygraph->hashtable,id1,id2,ammount);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"deltran")==0)
			{
				unsigned long id1;
				unsigned long id2;
				while(token!=NULL)
				{
					word++;
					if(word==2)
						sscanf(token,"%li",&id1);
					else if(word==3)
					{
						sscanf(token,"%li",&id2);
						del_tran(mygraph->hashtable,id1,id2);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"lookup")==0)
			{
				int mode;
				unsigned long id;
				while(token!=NULL)
				{
					word++;
					if(word==2)
					{
						if(strcmp(token,"in")==0)
							mode=0;
						else if(strcmp(token,"out")==0)
							mode=1;
						else if(strcmp(token,"sum")==0)
							mode=2;
					}
					else if(word==3)
					{
						sscanf(token,"%li",&id);
						lookup(mygraph,mode,id);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"triangle")==0)
			{
				double ammount;
				unsigned long id;
				while(token!=NULL)
				{
					word++;
					if(word==2)
						sscanf(token,"%li",&id);
					else if(word==3)
					{
						sscanf(token,"%lf",&ammount);
						triangle(mygraph,id,ammount);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"conn")==0)
			{
				unsigned long id1;
				unsigned long id2;
				while(token!=NULL)
				{
					word++;
					if(word==2)
						sscanf(token,"%li",&id1);
					else if(word==3)
					{
						sscanf(token,"%li",&id2);
						connected_nodes(mygraph,id1,id2);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"allcycles")==0)
			{
				unsigned long id;
				while(token!=NULL)
				{
					word++;
					if(word==2)
					{
						sscanf(token,"%li",&id);
						allcycles(mygraph,id);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"traceflow")==0)
			{
				int depth;
				unsigned long id;
				while(token!=NULL)
				{
					word++;
					if(word==2)
						sscanf(token,"%li",&id);
					else if(word==3)
					{
						sscanf(token,"%d",&depth);
						traceflow(mygraph,id,depth);
					}
					token=strtok(NULL,s);
				}
			}
			else if(strcmp(token,"bye")==0)
			{
				if(graph_exists==1)
				{
					delete_graph(mygraph);
					graph_exists=0;
				}
			}
			else if(strcmp(token,"print")==0)
				print_graph(mygraph);
			else if(strcmp(token,"dump")==0)
			{
				while(token!=NULL)
				{
					word++;
					if(word==2)
						dump(mygraph,token);
					token=strtok(NULL,s);
				}
			}
			else
				printf("There is no %s command\n",token);
			word=0;
		}
		fclose(fp);		//Close the file used to read
	}

	//Read whatever the user inputs
	while(fgets(line,256,stdin)!=NULL)
	{
		token=strtok(line,s);
		if(graph_exists==0)		//Create a graph, if the previous one was freed
		{
			graph *mygraph;
			mygraph=create_graph(hash_table_size);
			graph_exists=1;
		}
		if(strcmp(token,"createnodes")==0)	//And compare the first word with the available commands
		{
			unsigned long id;
			while(token!=NULL)
			{
				word++;
				if(word>1)
				{
					if(strlen(token)!=7)
						printf("failure:Key must be 7 digit number\n");
					else
					{
						sscanf(token,"%li",&id);
						create_node(mygraph->hashtable,id);
					}
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"delnodes")==0)
		{
			unsigned long id;
			while(token!=NULL)
			{
				word++;
				if(word>1)
				{
					sscanf(token,"%li",&id);
					delete_node(mygraph->hashtable,id);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"addtran")==0)
		{
			unsigned long id1;
			unsigned long id2;
			double ammount;
			while(token!=NULL)
			{
				word++;
				if(word==2)
					sscanf(token,"%li",&id1);
				else if(word==3)
					sscanf(token,"%li",&id2);
				else if(word==4)
				{
					sscanf(token,"%lf",&ammount);
					add_tran(mygraph->hashtable,id1,id2,ammount);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"deltran")==0)
		{
			unsigned long id1;
			unsigned long id2;
			while(token!=NULL)
			{
				word++;
				if(word==2)
					sscanf(token,"%li",&id1);
				else if(word==3)
				{
					sscanf(token,"%li",&id2);
					del_tran(mygraph->hashtable,id1,id2);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"lookup")==0)
		{
			int mode;
			unsigned long id;
			while(token!=NULL)
			{
				word++;
				if(word==2)
				{
					if(strcmp(token,"in")==0)
						mode=0;
					else if(strcmp(token,"out")==0)
						mode=1;
					else if(strcmp(token,"sum")==0)
						mode=2;
				}
				else if(word==3)
				{
					sscanf(token,"%li",&id);
					lookup(mygraph,mode,id);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"triangle")==0)
		{
			double ammount;
			unsigned long id;
			while(token!=NULL)
			{
				word++;
				if(word==2)
					sscanf(token,"%li",&id);
				else if(word==3)
				{
					sscanf(token,"%lf",&ammount);
					triangle(mygraph,id,ammount);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"conn")==0)
		{
			unsigned long id1;
			unsigned long id2;
			while(token!=NULL)
			{
				word++;
				if(word==2)
					sscanf(token,"%li",&id1);
				else if(word==3)
				{
					sscanf(token,"%li",&id2);
					connected_nodes(mygraph,id1,id2);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"allcycles")==0)
		{
			unsigned long id;
			while(token!=NULL)
			{
				word++;
				if(word==2)
				{
					sscanf(token,"%li",&id);
					allcycles(mygraph,id);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"traceflow")==0)
		{
			int depth;
			unsigned long id;
			while(token!=NULL)
			{
				word++;
				if(word==2)
					sscanf(token,"%li",&id);
				else if(word==3)
				{
					sscanf(token,"%d",&depth);
					traceflow(mygraph,id,depth);
				}
				token=strtok(NULL,s);
			}
		}
		else if(strcmp(token,"bye")==0)
		{
			if(graph_exists==1)
			{
				delete_graph(mygraph);
				graph_exists=0;
			}
		}
		else if(strcmp(token,"print")==0)
			print_graph(mygraph);
		else if(strcmp(token,"dump")==0)
		{
			while(token!=NULL)
			{
				word++;
				if(word==2)
					dump(mygraph,token);
				token=strtok(NULL,s);
			}
		}
		else
			printf("There is no %s command\n",token);
		word=0;
	}

	if(graph_exists==1)	//Delete the graph if we didnt delete it ourselves
		delete_graph(mygraph);
}
