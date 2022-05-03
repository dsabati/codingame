#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// MACROS
#define max(a,b) ((a) > (b) ? (a) : (b))

// Utils
#define INF_MIN                             -999999
#define INF_MAX                             999999

// CONSTANTS
#define NB_PEOPLE_MAX                       10000

typedef struct person {
    int id;
    int nbInfluenced;
    int nbInfluencedBy;
    struct person* influenced[NB_PEOPLE_MAX];
    struct person* influencedBy[NB_PEOPLE_MAX];
} Person;

// VARIABLES
int nbRelationship;                                 // the number of relationships of influence
int lastPerson;
int firstPerson;
Person persons[NB_PEOPLE_MAX];

//FUNCTIONS
int nbChainFromPerson(Person* person, int depth) {
    int chain = depth;
    for (int i = 0; i < person->nbInfluenced; i++) {
        chain = max(nbChainFromPerson(person->influenced[i], depth + 1), depth);
    }

    return chain;
}

int nbChainToPerson(Person* person, int depth) {
    int chain = depth;
    for (int i = 0; i < person->nbInfluencedBy; i++) {
        chain = max(nbChainToPerson(person->influencedBy[i], depth + 1), depth);
    }

    return chain;
}

//MAIN
int main() {
    // Init persons
    for (int i = 0; i < NB_PEOPLE_MAX; i++) {
        Person* p = &persons[i];
        p->id = i;
        p->nbInfluenced = 0;
        p->nbInfluencedBy = 0;
    }

    firstPerson = INF_MAX;
    lastPerson = INF_MIN;
    scanf("%d", &nbRelationship);
    for (int i = 0; i < nbRelationship; i++) {
        // a relationship of influence between two people (x influences y)
        int x;
        int y;
        scanf("%d%d", &x, &y);
    
        Person* px = &persons[x];
        Person* py = &persons[y];

        px->influenced[px->nbInfluenced++] = py;
        py->influencedBy[py->nbInfluencedBy++] = px;

        if (x > lastPerson) lastPerson = x;
        if (y > lastPerson) lastPerson = y;
        if (x < firstPerson) firstPerson = x;
        if (y < firstPerson) firstPerson = y;
    }

    int maxChain = 0;
    for (int i = firstPerson; i < lastPerson; i++) {
        Person* p = &persons[i];
        maxChain = max(nbChainToPerson(p, 0) + nbChainFromPerson(p, 1), maxChain);
    }

    // The number of people involved in the longest succession of influences
    printf("%d\n", maxChain);

    return 0;
}
