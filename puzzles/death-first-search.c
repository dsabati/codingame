#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Utils
#define INF_MIN 	-999999

// CONSTANTS
#define NB_NODE_MIN 	2
#define NB_NODE_MAX 	500

#define NB_LINK_MIN 	1
#define NB_LINK_MAX 	1000

#define NB_EXIT_MIN 	1
#define NB_EXIT_MAX 	20

#define EVAL_EXIT 	100
#define MAX_ITERATION 	5

// STRUCTS
typedef struct node
{
	int id;
	char isExit;
	int nbNeighbors;
	struct node *neighbors[NB_NODE_MAX];
} Node;

// VARIABLES
int nbNodes;
int nbLinks;
int nbExits;

Node nodes[NB_NODE_MAX];

// FUNCTIONS
void removeNeighbor(Node *node, int id)
{
	int index = NB_NODE_MAX;
	for (int i = 0; i < node->nbNeighbors - 1; i++)
	{
		if (node->neighbors[i]->id == id)
			index = i;
		if (index <= i)
			node->neighbors[i] = node->neighbors[i + 1];
	}
	node->nbNeighbors--;
}

int evalNode(Node *node, int depth)
{
	if (depth == MAX_ITERATION)
		return INF_MIN;
	if (node->isExit)
		return EVAL_EXIT / depth;

	float bestEval = INF_MIN;
	for (int i = 0; i < node->nbNeighbors; i++)
	{
		Node *neighbor = node->neighbors[i];
		float eval = evalNode(neighbor, depth + 1);
		if (eval > bestEval)
			bestEval = eval;
	}

	return bestEval / depth;
}

// MAIN
int main()
{
	scanf("%d%d%d", &nbNodes, &nbLinks, &nbExits);

	for (int i = 0; i < nbNodes; i++)
	{
		Node *n = &nodes[i];
		n->id = i;
		n->nbNeighbors = 0;
		n->isExit = false;
	}

	for (int i = 0; i < nbLinks; i++)
	{
		int N1;
		int N2;
		scanf("%d%d", &N1, &N2);

		Node *n1 = &nodes[N1];
		Node *n2 = &nodes[N2];

		n1->neighbors[n1->nbNeighbors++] = n2;
		n2->neighbors[n2->nbNeighbors++] = n1;
	}

	for (int i = 0; i < nbExits; i++)
	{
		int EI;
		scanf("%d", &EI);

		nodes[EI].isExit = true;
	}

	while (1)
	{
		int bobnetNodeId;
		scanf("%d", &bobnetNodeId);

		Node *bobnetNode = &nodes[bobnetNodeId];

		float bestEval = INF_MIN;
		Node *bestNode = NULL;
		for (int i = 0; i < bobnetNode->nbNeighbors; i++)
		{
			Node *node = bobnetNode->neighbors[i];
			float eval = evalNode(node, 1);

			if (eval > bestEval)
			{
				bestEval = eval;
				bestNode = node;
			}
		}

		printf("%d %d\n", bobnetNode->id, bestNode->id);
		removeNeighbor(bobnetNode, bestNode->id);
		removeNeighbor(bestNode, bobnetNode->id);
	}

	return 0;
}