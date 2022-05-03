#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

//---------------CONSTANTS-----------------
// Game related
#define GAME_WIDTH          16000
#define GAME_HEIGHT         9000
#define GAME_CENTER_X       GAME_WIDTH / 2
#define GAME_CENTER_Y       GAME_HEIGHT / 2

// Ash related
#define ASH_SPEED           1000
#define ASH_RADIUS          2000

// Humans related
#define NB_HUMANS_MAX       100

//Zombies related
#define NB_ZOMBIES_MAX      100
#define ZOMBIE_SPEED        400
#define ZOMBIE_RADIUS       400

//Targeting
#define NB_INTERSTEP_MAX    20

// Utils
#define INF_MAX             999999

//---------------STRUCTS-----------------

typedef struct point {
    int x;
    int y;
    float dist;
} Point;

typedef struct human {
    int id;
    Point position;
    float distFromAsh;
} Human;

typedef struct zombie {
    int id;
    Point position;
    Point next;
    Point trajectory;
    float distFromAsh;
    float distFromHumans[NB_HUMANS_MAX];
} Zombie;

typedef struct data {
    Point ash;
    int nbHumans;
    Human humans[NB_HUMANS_MAX];
    int nbZombies;
    Zombie zombies[NB_ZOMBIES_MAX];
} Data;

//---------------FUNCTION DEFINITIONS-----------------

float distance(Point, Point);
int intercept(Point*, Zombie*, Point);

//---------------VARIABLES-----------------

Data data;
Point centerPos = { GAME_CENTER_X, GAME_CENTER_Y };

//---------------MAIN-----------------

int main()
{
    while (1) {
        Point bestMove = centerPos;

        scanf("%d%d", &data.ash.x, &data.ash.y);

        scanf("%d", &data.nbHumans);
        for (int i = 0; i < data.nbHumans; i++) {
            Human* h = &data.humans[i];
            scanf("%d%d%d", &h->id, &h->position.x, &h->position.y);

            h->distFromAsh = distance(h->position, data.ash);
        }

        scanf("%d", &data.nbZombies);
        for (int i = 0; i < data.nbZombies; i++) {
            Zombie* z = &data.zombies[i];
            scanf("%d%d%d%d%d", &z->id, &z->position.x, &z->position.y, &z->next.x, &z->next.y);

            z->distFromAsh = distance(z->position, data.ash);
            z->trajectory.x = z->next.x - z->position.x;
            z->trajectory.y = z->next.y - z->position.y;

            for (int i = 0; i < data.nbHumans; i++) {
                Human* h = &data.humans[i];
                z->distFromHumans[h->id] = distance(z->position, h->position);
            }
        }

        Zombie* bestZombie = NULL;
        float distMin = INF_MAX;
        for (int i = 0; i < data.nbZombies; i++) {
            Zombie* z = &data.zombies[i];
            for (int j = 0; j < data.nbHumans; j++) {
                Human h = data.humans[j];

                float distAshZombie = z->distFromAsh;
                float distZombieHuman = z->distFromHumans[h.id];

                float stepZombieHuman = distZombieHuman / ZOMBIE_SPEED;
                float stepAshHuman = (h.distFromAsh - ASH_RADIUS + 1)/ ASH_SPEED;

                // If human cannot be reached before zombie, abandon him
                if (stepZombieHuman < stepAshHuman) continue;
                if (distZombieHuman < distMin) {
                    distMin = z->distFromHumans[h.id];
                    bestZombie = z;
                }
            }
        }

        if (bestZombie) {
            bestMove = bestZombie->position;

            // Compute interception position
            if (bestZombie->distFromAsh > ASH_RADIUS) {
                Point inter = { .x = bestZombie->position.x, .y = bestZombie->position.y, .dist = 0 };
                int interStep = intercept(&inter, bestZombie, data.ash);
                bestMove = inter;
            }
        }

        printf("%d %d\n", bestMove.x, bestMove.y); // Your destination coordinates
    }

    return 0;
}

//---------------FUNCTIONS-----------------

int intercept(Point* inter, Zombie* target, Point ash) {
    Point next = { .x = target->position.x, .y = target->position.y };
    for (char s = 0; s < NB_INTERSTEP_MAX; s++) {
        next.x += target->trajectory.x;
        next.y += target->trajectory.y;

        if (next.x < 0 || next.y < 0) return s;

        float dist = distance(ash, next);
        int step = dist / ASH_SPEED;
        if (step <= s) {
            inter->x = next.x;
            inter->y = next.y;
            return step;
        }
    }
    return NB_INTERSTEP_MAX;
}

float distance(Point p1, Point p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return sqrt((dx * dx) + (dy * dy));
}
