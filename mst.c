#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define maxVertices 1000
#define maxEdges 1000000
int graph[maxVertices][maxVertices];
/* Input graph must be undirected,weighted and connected*/
typedef struct Edge
{
	int from,to,weight;
}Edge;
int compare(const void * x,const void * y)
{
	return (*(Edge *)x).weight - (*(Edge *)y).weight;
}
Edge E[maxEdges];
int parent[maxVertices];
void init(int vertices)
{
	int iter=0;
	for(iter=0;iter<vertices;iter++)
	{
		parent[iter]=-1;
	}

}
int Find(int vertex)
{
	if(parent[vertex]==-1)return vertex;
	return parent[vertex] = Find(parent[vertex]); /* Finding its parent as well as updating the parent 
							 of all vertices along this path */
}
void Union(int parent1,int parent2)
{
	/* This can be implemented in many other ways. This is one of them */
	parent[parent1] = parent2;
}
void Kruskal(int vertices,int edges)
{
	memset(graph,-1,sizeof(graph)); /* -1 represents the absence of edge between them */
	/* Sort the edges according to the weight */
	qsort(E,edges,sizeof(Edge),compare);
	/* Initialize parents of all vertices to be -1.*/
	init(vertices);
	int totalEdges = 0,edgePos=0,from,to,weight;
	Edge now;
	while(totalEdges < vertices -1)
	{
		if(edgePos==edges)
		{
			/* Input Graph is not connected*/
			exit(0);
		}
		now = E[edgePos++];
		from = now.from;
		to = now.to;
		weight=now.weight;
		/* See If vetices from,to are connected. If they are connected do not add this edge. */
		int parent1 = Find(from);
		int parent2 = Find(to);
		if(parent1!=parent2)
		{
			graph[from][to] = weight;
			Union(parent1,parent2);
			totalEdges++;
		}
	}

}
int main()
{
	int vertices = 4,edges = 0;
	int from,to,weight;
	int totalCost;
	int matrix[4][4];
	
	for(int i = 0;i < 4;i++){
		for(int j = 0;j < 4;j++){
				matrix[i][j] = 0;
		}
	}
	
	for(int i = 0;i < 4;i++){
		for(int j = i + 1;j < 4;j++){
			if(matrix[i][j] != 0){
				E[edges].from = i; 
				E[edges].to = j; 
				E[edges].weight = matrix[i][j];
				edges++;
			}
		}
	}
	/* Finding MST */
	Kruskal(vertices,edges);
	/* Printing the MST */
	printf("The Client has calculated a tree.The tree cost is %d\n",totalCost);
	for(int i=0;i<vertices;i++){
		for(int j=0;j<vertices;j++){
			if(graph[i][j]!=-1){
				printf("%c%c     %d\n",'A' + i,'A' + j,graph[i][j]);
			}
		}
	}
	 
	return 0;
}