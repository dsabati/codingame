#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

/* ---------- CONSTANTS ---------- */

// Player related
#define NB_PLAYER                           2
#define PLAYER_ME                           0
#define PLAYER_OP                           1
#define PLAYER_NONE                         2

// Board related
#define GAME_MAX_X                          17630
#define GAME_MAX_Y                          9000
#define GAME_CENTER_X                       GAME_MAX_X / 2
#define GAME_CENTER_Y                       GAME_MAX_Y / 2
#define GAME_TOP_LEFT                       0
#define GAME_BOTTOM_RIGHT                   1
#define GRID_WIDTH                          27
#define GRID_HEIGHT                         15
#define GRID_CENTER_X                       GRID_WIDTH / 2
#define GRID_CENTER_Y                       GRID_HEIGHT / 2
#define CELL_WIDTH                          GAME_MAX_X / GRID_WIDTH
#define CELL_HEIGHT                         GAME_MAX_Y / GRID_HEIGHT

// Base related
#define BASE_DETECT_RADIUS                  5000
#define BASE_DEF_RADIUS_FACTOR              1.6
#define BASE_KILL_RADIUS                    300
#define BASE_FOG_RADIUS                     6000

// Entity related
#define NB_ENTITY_MAX                       300
#define NB_ENTITY_PACKED_MAX                20
#define ENTITY_MONSTER                      0
#define ENTITY_HERO_ME                      1
#define ENTITY_HERO_OP                      2

// Heroes related
#define NB_HEROES                           3
#define HERO_SPEED                          800
#define MY_HERO_1                           0
#define MY_HERO_2                           1
#define MY_HERO_3                           2
#define HERO_FOG_RADIUS                     2200

// Monster related
#define MONSTER_SPEED                       400
#define MONSTER_NO_THREAT                   0
#define MONSTER_NEAR_BASE                   1
#define MONSTER_THREAT_ME                   1
#define MONSTER_THREAT_OP                   2
#define STATE_COMPUTED                      2
#define STATE_IN_VIEW                       1
#define STATE_UNTOUCHED                     0
#define STATE_DEACTIVATED                   -1
#define STATE_DEAD                          -2

// Actions
#define A_WAIT                              0
#define A_MOVE                              1
#define A_WIND                              2
#define A_SHIELD                            3
#define A_CONTROL                           4

#define ATTACK_RADIUS                       800
#define ATTACK_DAMAGE                       2
#define WIND_RADIUS                         1280
#define WIND_PUSH_FORCE                     2200
#define SHIELD_RADIUS                       2200
#define SHIELD_MAX_TIME                     12
#define CONTROL_RADIUS                      2200
#define SPELL_COST                          10

// Utils
#define INF_MIN                             -999999
#define INF_MAX                             999999

/* ------------- DEBUG ------------ */

#define DEBUG                               1
#define DEBUG_ENTITIES                      0
#define DEBUG_METHOD_NAME                   1
#define DEBUG_STRATEGY_MODE                 1
#define DEBUG_MULTI_TARGET                  0
#define DEBUG_EVAL_TARGET                   0
#define DEBUG_SHIELD_ME                     0
#define DEBUG_BREST_GRID                    0
#define DEBUG_EXPLORATION_MAP_INIT          1
#define DEBUG_EXPLORATION_MAP               0
    #define DEBUG_GLOBAL_MAP                1
    #define DEBUG_FARM_MAP                  0
    #define DEBUG_ATTACK_MAP                0
    #define DEBUG_DEFENSE_MAP               0
    #define DEBUG_RUSH_MAP                  0
#define DEBUG_MOVE                          0
#define DEBUG_ACTION_TEXT                   0

/* ---------- METHODS ---------- */

// Round initialization
#define METHOD_0                            0
#define M0_COPY_STATE                       1
#define M0_REFRESH_OUT_MONSTERS             1
#define M0_COMPUTE_SYMMETRY                 1

// Exploration
#define METHOD_1                            1
#define M1_MAX_AGE                          10
#define M1_AGE_PRIORITY                     1
#define M1_RUSH_RADIUS_FACTOR               1.05
#define M1_RUSH_RADIUS_FACTOR_IN            0.95
#define M1_ATTACK_RADIUS_FACTOR             1.4
#define M1_ATTACK_RADIUS_FACTOR_IN          0.9
#define M1_FARM_RADIUS_FACTOR               1.6
#define M1_DEFENSE_RADIUS_FACTOR            1.2
#define M1_INVASION_RADIUS_FACTOR           1.0
#define M1_ATTACKER_DONT_EXPLORE            0

#define M1_DEFENSE_GRID                     1
#define M1_ATTACK_GRID                      1
#define M1_FARM_GRID                        1
#define M1_RUSH_GRID                        0

#define M1_DEFENSE_GRID_CENTER_TOP          0

// Attack entity
#define METHOD_2                            1
#define M2_ATTACK_NB_THREAT_MAX             2
#define M2_ATTACK_NB_NEAR_BASE_MAX          1
#define M2_LEAVE_TARGET_TO_NEAREST          1
#define M2_IGNORE_IF_OUT                    1

#define M2_MULTI_TARGET                     1
#define M2_MULTI_TARGET_MEMORY_SIZE         5030
#define M2_MULTI_TARGET_STEP                20
#define M2_MULTI_TARGET_STEP_MAX            1       // Number of step max to target to check for multi target
#define M2_NB_INTERSTEP_MAX                 20

#define M2_URGENT_DEFENSE                   1
#define M2_MULTI_TARGET_DEFENSE             0
#define M2_URGENT_RADIUS_FACTOR             1.0
#define M2_STAY_NEAR_ENEMY                  1
#define M2_NEAR_ENEMY_DIST_MAX              2200
#define M2_URGENT_KEEP_MANA_MIN             0
#define M2_URGENT_PUSH_STEP_MIN             1
#define M2_URGENT_ALWAYS_PUSH_OUT           1

#define M2_RESTRICT_DEFENDER_ATTACK         1

#define M2_EVAL_POS_MONSTER_VALUE           100.0
#define M2_EVAL_POS_MONSTER_DIST_POW        1.2
#define M2_EVAL_POS_OTHER_VALUE             100.0
#define M2_EVAL_POS_OTHER_DIST_POW          1.2

// Wind spell protect
#define METHOD_3                            3
#define M3_ALWAYS_PUSH_OUT                  1
#define M3_PUSH_OUT_HEALTH_MIN              6
#define M3_NB_SPIDER_BASE_MAX               2
#define M3_KEEP_MANA_MIN                    0

// Control spell
#define METHOD_4                            1
#define M4_MONSTER_CONTROL_ATTACK           1
#define M4_ATTACK_LIGHT_HEALTH_MIN          12
#define M4_ATTACK_HEALTH_MIN                20
#define M4_MONSTER_CONTROL_ATTRACT          0
#define M4_ATTRACT_HEALTH_MAX               15

// Shield hero spell
#define METHOD_5                            0
#define M5_PROTECT_FROM_ALL_ENEMIES         0
#define M5_THREAT_RADIUS_FACTOR             1.5
#define M5_CHASE_ENEMY_IN_BASE              1
#define M5_SHIELD_ME_NEAR_ENEMY             1
#define M5_PUSH_ENEMY_OUT                   0

// Shield monster spell
#define METHOD_6                            1

// Wind spell attack
#define METHOD_7                            1
#define M7_HEALTH_MIN_PUSH                  8

// Strategies
#define METHOD_8                            1
#define M8_ATTACK_RADIUS_FACTOR             1.2
#define M8_ALWAYS_PUSH_IN_BASE              1

#define M8_ATTACK_ROUND_MIN                 150         // Force attack in round
#define M8_ATTACK_MANA_MIN                  999        // Force attack in round
#define M8_ATTACK_CANCEL_MANA_MIN           10

#define M8_ATTACK_IF_LOSING                 0
#define M8_DEFENSE_IF_WINNING               0

#define M8_RUSH_ROUND_MIN                   999
#define M8_RUSH_MANA_MIN                    150
#define M8_RUSH_CANCEL_MANA_MIN             10

#define M8_NB_ATTACKER                      1
#define M8_NB_ENEMY_FARMER                  1
#define M8_FARM_ENEMY_ROUND_MIN             0         // Force farm of enemy zone

#define M8_INNER_ATTACK_RADIUS_FACTOR       1.0

#define M8_AVOID_KILLING_SPIDERS            1

// Strategies role distribution
#define METHOD_9                            1

#define M9_FARM_NB_FARMER                   2
#define M9_FARM_NB_DEFENDER                 1
#define M9_FARM_NB_ATTACKER                 0
#define M9_FARM_NB_RUSHER                   0

#define M9_DEFENSE_NB_FARMER                1
#define M9_DEFENSE_NB_DEFENDER              2
#define M9_DEFENSE_NB_ATTACKER              0
#define M9_DEFENSE_NB_RUSHER                0

#define M9_ATTACK_NB_FARMER                 1
#define M9_ATTACK_NB_DEFENDER               1
#define M9_ATTACK_NB_ATTACKER               1
#define M9_ATTACK_NB_RUSHER                 0

#define M9_RUSH_NB_FARMER                   0
#define M9_RUSH_NB_DEFENDER                 1
#define M9_RUSH_NB_ATTACKER                 0
#define M9_RUSH_NB_RUSHER                   2

/* ---------- STRATEGIES ---------- */
// Strategy modes
#define NB_MODES                            4
#define M_FARM                              0
#define M_ATTACK                            1
#define M_DEFENSE                           2
#define M_RUSH                              3

#define M_START                             M_FARM

// Rush strategy
#define S_RUSH                              1
#define S_RUSH_CONTROL_KEEP_MANA_MIN        80
#define S_RUSH_HORIZONTAL_SPIDER_COLLECT    1
#define S_RUSH_ABORT_CONDITIONS             1
#define S_RUSH_ENEMY_POINT_MANA_COST        10
#define S_RUSH_AVOID_KILLING_SPIDERS        1
#define S_RUSH_MOBILE                       0

// Farm strategy
#define S_FARM                              0

/* ---------- MACROS ---------- */

// Logs
#define log(args...) fprintf(stderr, args)

// Math utils
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define randInt(a,b) (int)(((rand() / (double)RAND_MAX) * ((b) - (a) + 1)) + (a))
#define randDouble(a,b) (double)(((rand() / (double)RAND_MAX) * ((b) - (a) + 1)) + (a))
#define round(n) (int)(n > 0 ? ((n) + 0.5) : ((n) - 0.5))

// Time
#define GET_TIME (gettimeofday(&ttt, NULL))
#define START_TIME (startTime = TOP_TIME)
#define TOP_TIME (ttt.tv_sec * 1000.0 + ttt.tv_usec / 1000.0)
#define ELAPSED_TIME (TOP_TIME - startTime)
#define SHOW_TIME(txt) (log("%s...CPU: %.6f ms\n", txt, TOP_TIME - startTime))
#define SHOW_MAX_TIME (log("MAX Time: %0.6f ms\n", maxTime))
struct timeval ttt;
double startTime;
double maxTime;

#define TIME_LIMIT_LOOP                 49
#define TIME_LIMIT_INIT                 999

/* ---------- STRUCTURES ---------- */

typedef struct point {
    int x;
    int y;
    float dist;
} Point;

typedef struct move {
    char action;        // WAIT MOVE WIND SHIELD CONTROL
    int target;
    Point pos;
} Move;

typedef struct entity {
    int id;             // Unique identifier
    int rank;           // Hero rank
    int health;         // Remaining health of this monster
    int healthMax;      // Remaining health of this monster
    int shield;         // Count down until shield spell fades
    int type;           // 0=monster, 1=your hero, 2=opponent hero
    char isControlled;  // Equals 1 when this entity is under a control spell
    char nearBase;      // 0=monster with no target yet, 1=monster targeting a base
    char threatFor;     // Given this monster's trajectory, is it a threat to 1=your base, 2=your opponent's base, 0=neither
    char nbAttacker;    // Number of heroes attacking this entity
    char target;        // Id of target for entity
    char visible;       // Is entity visible
    char mode;
    char modeRank;
    char end;
    Point dest;      // Trajectory of entity
    Point pos;       // Point of entity
} Entity;

typedef struct player {
    int health;
    int mana;
    int id;
    Point basePos;
    Move bestMove[NB_HEROES];
} Player;

typedef struct data {
    char strategy;
    char nbAttacker;
    char nbFarmer;
    char nbDefender;
    char nbRusher;
    char nbHero;
    Entity* heroes[NB_HEROES];
    Entity* orderedHeroes[NB_HEROES];
    char nbEnemy;
    Entity* enemies[NB_HEROES];
    Entity* enemyInBase;
    char nbMonster;
    Entity* monsters[NB_ENTITY_MAX];
    float distHeroEntity[NB_HEROES][NB_ENTITY_MAX];
    float distBaseEntity[NB_PLAYER][NB_ENTITY_MAX];
    float distBaseHero[NB_PLAYER][NB_HEROES];
    float distEnemyEntity[NB_HEROES][NB_ENTITY_MAX];
    int nbStepBeforeOut[NB_ENTITY_MAX];
    int nbMonsterInBase[NB_PLAYER];
    int countRushMonsters;
} Data;

typedef struct grid {
    float eval[GRID_HEIGHT][GRID_WIDTH];
    int fog[GRID_HEIGHT][GRID_WIDTH];
    int attack[GRID_HEIGHT][GRID_WIDTH];
    int defense[GRID_HEIGHT][GRID_WIDTH];
    int farm[GRID_HEIGHT][GRID_WIDTH];
    int rush[GRID_HEIGHT][GRID_WIDTH];
} Grid;

typedef struct node {
    int round;
    int depth;
    Player players[NB_PLAYER];
    int nbEntities;
    Entity entities[NB_ENTITY_MAX];
} Node;

typedef struct game {
    int round;
    char orientation;
} Game;

/* ---------- PROTOTYPES ---------- */

// Core
void initGame(Game*);
void readInputs(Node*);
void refreshState(Node*, Node*);
void computeSymmetry(Node*);
void setMove(Move*, char, int, Point);
void playMoves(Game*, Node*);

// Sub
Entity* getEntity(Node*, int);
Entity* getEnemyInBase(Player*);
Entity* findClosestHeroFromEntity(Entity*);
Entity* findClosestFreeHeroFromEntity(Entity*);
Entity* findClosestMonsterFromBase(Player*);
Entity* findBestMonsterToControl(Entity*, int);
Move* heroBestMove(Entity*);
char hasEnoughMana(Player*);
char isInBase(Player*, Entity*);
char isInGame(Point);
char isInView(Node* n, Entity*);
char isCastable(Entity*);
char canCastSpell(Player*, Entity*, Entity*, int);
char shouldCastWind(Player*, Entity*, Entity*, int);
char canKillThisRound(Entity*, Entity*);
char heroRank(Entity*);
int interception(Point*, Entity*, Entity*);
Point orientedPosition(Point);

// Moves
void wait(Player*, Entity*);
void move(Player*, Entity*, Point);
void attack(Player*, Entity*, Entity*);
void castWind(Player*, Entity*, Entity*, Point);
void castControl(Player*, Entity*, Entity*, Point);
void castShield(Player*, Entity*, Entity*);
int optimizeMove(Point*, Entity*, Entity*, int);

// Logs
void logEntity(Entity*);
void logMove(Move*);
void logBestTarget(Entity*, Entity*, int);
void logExplorationState(Grid*);

// Utils
float distance(Point, Point);
float distance2(Point, Point);
float distanceEntity(Entity*, Entity*);
char isRectangleInCircle(Point, int, Point, Point);
char isPointInCircle(Point, int, Point);
float getAngle(Point, Point);
Point computeAnglePosition(Point, int, float);

/* ---------- MAIN ---------- */
// Core instances
Game game;
Node state;
Node memo;
Data data;
Grid grid;

// Entity state
char entityState[NB_ENTITY_MAX];

// Messages
char mes[NB_HEROES][50] = { "", "", "" };
char mode[NB_MODES][2] = { "F", "A", "D", "R" };

// Flags
char enemyUseControl = false;

// Core positions
Point zeroPos = { .x = 0, .y = 0 };
Point centerPos = { .x = GAME_CENTER_X, .y = GAME_CENTER_Y };
Point farmPos[2] = {{8000, 8500}, {4200, 8500, 0}};
Point rushPos[2] = {{12930, 8600, 0}, {12630, 8000, 0}};
Point controlTo = { 12500, 8550 };

// Step grid
int nbMoveGrid;
Point moveGrid[M2_MULTI_TARGET_MEMORY_SIZE];


int main() {
    initGame(&game);
    maxTime = 0.0;

    nbMoveGrid = 0;
    for (int dy = -HERO_SPEED; dy <= HERO_SPEED; dy += M2_MULTI_TARGET_STEP) {
        for (int dx = -HERO_SPEED; dx <= HERO_SPEED; dx += M2_MULTI_TARGET_STEP) {
            Point dPos = { .x = dx, .y = dy };
            float dist = distance(zeroPos, dPos);
            if ( dist > HERO_SPEED) continue;

            Point* p = &moveGrid[nbMoveGrid++];
            p->x = dx;
            p->y = dy;
            p->dist = dist;
        }
    }

    // game loop
    while (1) {
        Node* n = &state;

        Player* me = &n->players[PLAYER_ME];
        Player* op = &n->players[PLAYER_OP];

        Entity* backHero = NULL;
        Entity* midHero = NULL;
        Entity* frontHero = NULL;

        //--------------- INIT ROUND ----------------

        {
            n->round = game.round;

            // Read current round infos
            readInputs(n);

            GET_TIME;
            START_TIME;
            SHOW_TIME("Début du calcul");

            #if METHOD_0
                #if M0_COPY_STATE
                    // Refresh infos from previous round
                    refreshState(n, &memo);
                #endif
                #if M0_COMPUTE_SYMMETRY
                    // Figure infos by symmetry
                    computeSymmetry(n);
                #endif
            #endif

        }
    
        //--------------- PRE-COMPUTATION ----------------

        {
            // Init list of heroes and monsters
            data.nbHero = 0;
            data.nbEnemy = 0;
            data.nbMonster = 0;
            for (int e = 0; e < n->nbEntities; e++) {
                Entity* en = &n->entities[e];
                if (en->type == ENTITY_MONSTER) data.monsters[data.nbMonster++] = en;
                else if (en->type == ENTITY_HERO_ME) data.heroes[data.nbHero++] = en;
                else if (en->type == ENTITY_HERO_OP) data.enemies[data.nbEnemy++] = en;

                #if DEBUG_ENTITIES
                    logEntity(en);
                #endif
            }
            // log("NB HEROES: %d / NB ENEMIES: %d / NB MONSTERS: %d\n", data.nbHero, data.nbEnemy, data.nbMonster);

            // Init heroes data
            for (int e1 = 0; e1 < data.nbHero; e1++) {
                Entity* h = data.heroes[e1];
                h->rank = heroRank(h);
                h->mode = M_START;
                h->end = false;

                if (h->isControlled) enemyUseControl = true;

                // log("HERO(%d) Rank ==> %d\n", h->id, h->rank);

                // Distance bases <-> hero
                data.distBaseHero[PLAYER_ME][h->rank] = distance(me->basePos, h->pos);
                data.distBaseHero[PLAYER_OP][h->rank] = distance(op->basePos, h->pos);

                // Distance hero <-> enemy
                for (int e2 = 0; e2 < data.nbEnemy; e2++) {
                    Entity* eh = data.enemies[e2];
                    data.distHeroEntity[h->rank][eh->id] = distanceEntity(h, eh);
                }

                // Distance hero <-> monster
                for (int e2 = 0; e2 < data.nbMonster; e2++) {
                    Entity* m = data.monsters[e2];
                    data.distHeroEntity[h->rank][m->id] = distanceEntity(h, m);
                }

                // Set default move
                wait(me, h);
            }

            // Init enemies data
            for (int e = 0; e < data.nbEnemy; e++) {
                Entity* eh = data.enemies[e];
                eh->rank = heroRank(eh);

                // Distance with bases
                data.distBaseEntity[PLAYER_ME][eh->id] = distance(me->basePos, eh->pos);
                data.distBaseEntity[PLAYER_OP][eh->id] = distance(op->basePos, eh->pos);

                // Distance enemy <-> monster
                for (int e2 = 0; e2 < data.nbMonster; e2++) {
                    Entity* m = data.monsters[e2];
                    data.distEnemyEntity[eh->rank][m->id] = distanceEntity(eh, m);
                }
            }

            // Init monsters data
            data.nbMonsterInBase[PLAYER_ME] = 0;
            data.nbMonsterInBase[PLAYER_OP] = 0;
            for (int e = 0; e < data.nbMonster; e++) {
                Entity* m = data.monsters[e];

                // Distance with bases
                data.distBaseEntity[PLAYER_ME][m->id] = distance(me->basePos, m->pos);
                data.distBaseEntity[PLAYER_OP][m->id] = distance(op->basePos, m->pos);

                // Nb monsters in bases
                if (data.distBaseEntity[PLAYER_ME][m->id] < BASE_DETECT_RADIUS) data.nbMonsterInBase[PLAYER_ME]++;
                if (data.distBaseEntity[PLAYER_OP][m->id] < BASE_DETECT_RADIUS) data.nbMonsterInBase[PLAYER_OP]++;

                // Nb steps before monster is out of game
                int nbStep = 0;
                Point nextPos = { .x = m->pos.x, .y = m->pos.y };
                for (int i = 0; i < 20; i++) {
                    nextPos.x+= m->dest.x;
                    nextPos.y+= m->dest.y;
                    if (!isInGame(nextPos)) break;

                    nbStep++;
                }

                data.nbStepBeforeOut[m->id] = nbStep;
            }

            data.enemyInBase = getEnemyInBase(me);
        }

        //--------------- STATEGIES DEFINITION ----------------

        {
            #if METHOD_8
                #if DEBUG && DEBUG_METHOD_NAME
                    log("M8\n");
                #endif 

                if (data.strategy != M_RUSH && (game.round >= M8_ATTACK_ROUND_MIN || me->mana >= M8_ATTACK_MANA_MIN)) data.strategy = M_ATTACK;
                if (data.strategy == M_ATTACK && me->mana < M8_ATTACK_CANCEL_MANA_MIN) data.strategy = M_FARM;

                #if S_RUSH
                    if (game.round >= M8_RUSH_ROUND_MIN || me->mana >= M8_RUSH_MANA_MIN) {
                        if (data.strategy != M_RUSH) data.countRushMonsters = 0;
                        data.strategy = M_RUSH;
                    }
                    if (data.strategy == M_RUSH && me->mana < M8_RUSH_CANCEL_MANA_MIN) data.strategy = M_FARM;
                #endif

                #if M8_ATTACK_IF_LOSING
                    if (me->health < op->health && data.strategy != M_RUSH) data.strategy = M_ATTACK;
                #endif

                #if M8_DEFENSE_IF_WINNING
                    if (me->health > op->health) data.strategy = M_DEFENSE;
                #endif
            #endif

            #if METHOD_9
                #if DEBUG && DEBUG_METHOD_NAME
                    log("M9\n");
                #endif

                if (data.strategy == M_FARM) {
                    data.nbAttacker = M9_FARM_NB_ATTACKER;
                    data.nbDefender = M9_FARM_NB_DEFENDER;
                    data.nbFarmer = M9_FARM_NB_FARMER;
                    data.nbRusher = M9_FARM_NB_RUSHER;
                }
                else if (data.strategy == M_ATTACK) {
                    data.nbAttacker = M9_ATTACK_NB_ATTACKER;
                    data.nbDefender = M9_ATTACK_NB_DEFENDER;
                    data.nbFarmer = M9_ATTACK_NB_FARMER;
                    data.nbRusher = M9_ATTACK_NB_RUSHER;
                }
                else if (data.strategy == M_DEFENSE) {
                    data.nbAttacker = M9_DEFENSE_NB_ATTACKER;
                    data.nbDefender = M9_DEFENSE_NB_DEFENDER;
                    data.nbFarmer = M9_DEFENSE_NB_FARMER;
                    data.nbRusher = M9_DEFENSE_NB_RUSHER;
                }
                else if (data.strategy == M_RUSH) {
                    data.nbAttacker = M9_RUSH_NB_ATTACKER;
                    data.nbDefender = M9_RUSH_NB_DEFENDER;
                    data.nbFarmer = M9_RUSH_NB_FARMER;
                    data.nbRusher = M9_RUSH_NB_RUSHER;
                }

                // Find back hero
                float distMin = INF_MAX;
                for (char i = 0; i < data.nbHero; i++) {
                    Entity* h = data.heroes[i];
                    float dist = data.distBaseHero[PLAYER_ME][h->rank];

                    if (dist < distMin) {
                        distMin = dist;
                        backHero = h;
                    }
                }

                // Find front hero
                distMin = INF_MAX;
                for (char i = 0; i < data.nbHero; i++) {
                    Entity* h = data.heroes[i];
                    if (h == backHero) continue;

                    float dist = data.distBaseHero[PLAYER_OP][h->rank];

                    if (dist < distMin) {
                        distMin = dist;
                        frontHero = h;
                    }
                }

                // Find middle hero
                for (char i = 0; i < data.nbHero; i++) {
                    Entity* h = data.heroes[i];
                    if (h == backHero) continue;
                    if (h == frontHero) continue;

                    midHero = h;
                }

                if (data.nbFarmer > 0) midHero->mode = M_FARM;
                if (data.nbFarmer > 1) frontHero->mode = M_FARM;
                if (data.nbFarmer > 2) backHero->mode = M_FARM;
                if (data.nbAttacker > 0) frontHero->mode = M_ATTACK;
                if (data.nbAttacker > 1) midHero->mode = M_ATTACK;
                if (data.nbAttacker > 2) backHero->mode = M_ATTACK;
                if (data.nbDefender > 0) backHero->mode = M_DEFENSE;
                if (data.nbDefender > 1) midHero->mode = M_DEFENSE;
                if (data.nbDefender > 2) frontHero->mode = M_DEFENSE;

                if (data.nbFarmer == 2) {
                    frontHero->modeRank = 0;
                    midHero->modeRank = 1;
                }

                if (data.nbRusher == 2) {
                    frontHero->mode = M_RUSH;
                    frontHero->modeRank = 0;
                    midHero->mode = M_RUSH;
                    midHero->modeRank = 1;
                }

                data.orderedHeroes[0] = frontHero;
                data.orderedHeroes[1] = midHero;
                data.orderedHeroes[2] = backHero;

                #if DEBUG_STRATEGY_MODE
                    log("[STRATEGY MODE] = %s\n", mode[data.strategy]);
                    for (int i = 0; i < data.nbHero; i++) {
                        Entity* h = data.heroes[i];
                        log("Hero[%d]: %s%d\n", h->rank, mode[h->mode], h->modeRank);
                    }
                #endif

            #endif

        }

        //--------------- STATEGIES APPLICATION ----------------

        {
            #if S_RUSH
                if (data.strategy == M_RUSH) {

                    Entity* enemy = getEnemyInBase(op);

                    if (enemy && me->mana >= op->health * S_RUSH_ENEMY_POINT_MANA_COST) {
                        float distFrontHeroOPBase = data.distBaseHero[PLAYER_OP][frontHero->rank];

                        if (distFrontHeroOPBase <= BASE_FOG_RADIUS) {
                            if (canCastSpell(me, frontHero, enemy, CONTROL_RADIUS)) {
                                Point controlPoint = {.x = GAME_CENTER_X, .y = GAME_MAX_Y - enemy->pos.y };
                                castControl(me, frontHero, enemy, controlPoint);
                                frontHero->end = true;
                            } else {
                                move(me, frontHero, enemy->pos);
                                frontHero->end = true;
                            }
                        }
                    }

                    // Sure kill
                    if (me->mana >= 2 * SPELL_COST) {
                        for (int i = 0; i < data.nbMonster; i++) {
                            Entity* m = data.monsters[i];
                            if (data.distHeroEntity[frontHero->rank][m->id] > WIND_RADIUS) continue;
                            if (data.distHeroEntity[midHero->rank][m->id] > WIND_RADIUS) continue;
                            if (data.distBaseEntity[PLAYER_OP][m->id] > BASE_KILL_RADIUS + 2 * WIND_PUSH_FORCE + MONSTER_SPEED) continue;

                            castWind(me, frontHero, m, op->basePos);
                            castWind(me, midHero, m, op->basePos);

                            frontHero->end = true;
                            midHero->end = true;
                            break;
                        }
                    }

                    // Prepare sure kill
                    if (me->mana >= 3 * SPELL_COST) {
                        for (int i = 0; i < data.nbMonster; i++) {
                            Entity* m = data.monsters[i];

                            float distOpBaseMonster = data.distBaseEntity[PLAYER_OP][m->id];
                            if (distOpBaseMonster > BASE_KILL_RADIUS + 3 * WIND_PUSH_FORCE + MONSTER_SPEED) continue;

                            Point mNextPos = {
                                .x = m->pos.x + WIND_PUSH_FORCE * (op->basePos.x - m->pos.x) / distOpBaseMonster,
                                .y = m->pos.y + WIND_PUSH_FORCE * (op->basePos.y - m->pos.y) / distOpBaseMonster
                            };
                            float cast0 = data.distHeroEntity[frontHero->rank][m->id];
                            float cast1 = data.distHeroEntity[midHero->rank][m->id];
                            float dist0 = distance(mNextPos, frontHero->pos);
                            float dist1 = distance(mNextPos, midHero->pos);
                            if (cast0 < WIND_RADIUS && dist0 < WIND_RADIUS && dist1 < WIND_RADIUS + HERO_SPEED) {
                                if (dist1 > WIND_RADIUS) move(me, midHero, mNextPos);
                                castWind(me, frontHero, m, op->basePos);
                                frontHero->end = true;
                                midHero->end = true;
                            } else if (cast1 < WIND_RADIUS && dist1 < WIND_RADIUS && dist0 < WIND_RADIUS + HERO_SPEED) {
                                if (dist0 > WIND_RADIUS) move(me, frontHero, mNextPos);
                                castWind(me, midHero, m, op->basePos);
                                frontHero->end = true;
                                midHero->end = true;
                            }
                        }
                    }

                    // Control monsters to prepare for rush
                    for (char i = 0; i < data.nbHero; i++) {
                        Entity* h = data.heroes[i];
                        if (h->mode != M_RUSH) continue;
                        if (h->modeRank == 0) continue;
                        if (h->end) continue;

                        Entity* m = findBestMonsterToControl(h, CONTROL_RADIUS);
                        if (m && me->mana >= S_RUSH_CONTROL_KEEP_MANA_MIN + SPELL_COST) {
                            if (canCastSpell(me, h, m, CONTROL_RADIUS)) {
                                // Point opBase = { .x = op->basePos.x + (game.orientation == GAME_TOP_LEFT ? -4000 : 4000), .y = op->basePos.y };
                                castControl(me, h, m, orientedPosition(controlTo));
                                data.countRushMonsters++;
                                h->end = true;
                            }
                        }
                    }

                    // Move to rush position
                    // if (data.countRushMonsters >= 3) {
                        for (char i = 0; i < data.nbHero; i++) {
                            Entity* h = data.heroes[i];
                            if (h->mode != M_RUSH) continue;
                            if (h->end) continue;
                            Point rush = orientedPosition(rushPos[h->modeRank]);
                            if (h->pos.x == rush.x && h->pos.y == rush.y) continue;

                            #if S_RUSH_HORIZONTAL_SPIDER_COLLECT
                                if (h->modeRank == 1 && abs(h->pos.y - rush.y) > 1000) rush.x = h->pos.x;
                            #endif

                            optimizeMove(&rush, h, NULL, 20);
                            move(me, h, rush);
                            h->end = true;
                        }
                    // }

                    #if S_RUSH_ABORT_CONDITIONS
                        if (me->mana < op->health * S_RUSH_ENEMY_POINT_MANA_COST) {
                            data.strategy = M_FARM;
                            frontHero->mode = M_FARM;
                            midHero->mode = M_FARM;
                        }
                    #endif

                    #if 0
                        for (char i = 0; i < data.nbHero; i++) {
                            Entity* h = data.heroes[i];
                            if (h->end) continue;
                            if (h->mode != M_RUSH) continue;

                            float distRush = data.distBaseHero[PLAYER_OP][h->rank];
                            for (char j = 0; j < data.nbMonster; j++) {
                                Entity* m = data.monsters[j];
                                if (data.distHeroEntity[h->rank][m->id] > WIND_RADIUS) continue;
                                if (data.distBaseEntity[PLAYER_OP][m->id] < distRush) {
                                    castWind(me, h, m, op->basePos);
                                }
                            }

                            h->end = true;
                        }
                    #endif
                }
            #endif

            // Exploration
            #if METHOD_1
                #if DEBUG && DEBUG_METHOD_NAME
                    log("M1\n");
                #endif

                // Find best zone to explore
                for(char h = 0; h < data.nbHero; h++) {
                    Entity* hero = data.heroes[h];

                    #if S_RUSH_MOBILE
                        if(hero->mode == M_RUSH && hero->modeRank == 1) continue;
                    #else
                        if (hero->end) continue;
                    #endif

                    #if M1_ATTACKER_DONT_EXPLORE
                        if(hero->mode == M_ATTACK) continue;
                    #endif

                    float bestEval = INF_MIN;
                    Point bestGrid = { .x = -1, .y = -1 };

                    #if M1_AGE_PRIORITY
                        int ageMax = INF_MIN;
                    #endif

                    for (int j = 0; j < GRID_HEIGHT; j++) {
                        for (int i = 0; i < GRID_WIDTH; i++) {
                            if (hero->mode == M_RUSH && grid.rush[j][i] == 0) continue;
                            if (hero->mode == M_ATTACK && grid.attack[j][i] == 0) continue;
                            if (hero->mode == M_DEFENSE && grid.defense[j][i] == 0) continue;
                            if (hero->mode == M_FARM && grid.farm[j][i] == 0) continue;

                            Point cellCenter = {
                                .x = GAME_CENTER_X + (i - GRID_CENTER_X) * CELL_WIDTH,
                                .y = GAME_CENTER_Y + (j - GRID_CENTER_Y) * CELL_HEIGHT
                            };

                            #if METHOD_8
                                /* if ((data.strategy == M_ATTACK && hero->rank < M8_NB_ATTACKER) || (game.round >= M8_FARM_ENEMY_ROUND_MIN && hero->rank < M8_NB_ENEMY_FARMER)) {
                                    cellCenter.x = GAME_CENTER_X + (GRID_CENTER_X - i) * CELL_WIDTH;
                                    cellCenter.y = GAME_CENTER_Y + (GRID_CENTER_Y - j) * CELL_HEIGHT;
                                } */
                            #endif

                            Point cellCenter1 = {
                                .x = cellCenter.x - CELL_WIDTH / 2,
                                .y = cellCenter.y - CELL_HEIGHT / 2
                            };
                            Point cellCenter2 = {
                                .x = cellCenter.x + CELL_WIDTH / 2,
                                .y = cellCenter.y + CELL_HEIGHT / 2
                            };
                            if (isRectangleInCircle(hero->pos, HERO_FOG_RADIUS, cellCenter1, cellCenter2)) {
                                grid.fog[j][i] = 0;
                                grid.fog[GRID_HEIGHT - j - 1][GRID_WIDTH - i - 1] = 0;  // Visit sym cell
                                continue;
                            }

                            float dist = distance(hero->pos, cellCenter) + 1.0;

                        #if M1_AGE_PRIORITY
                            int age = grid.fog[j][i];
                            if (age < 0) age = 100;
                            float eval = age * 1000.0 / dist + grid.eval[j][i];
                            if (age > 0 && eval > bestEval) {
                        #else
                            float eval = 1000000.0 / dist + grid.Eval[j][i];
                            if (grid.Fog[j][i] < 0 && eval > bestEval) {
                        #endif
                                bestEval = eval;
                                bestGrid.x = i;
                                bestGrid.y = j;
                            }
                        }
                    }

                    if (bestGrid.x > -1) {
                        Point bestGridPosition = {
                            .x = GAME_CENTER_X + (bestGrid.x - GRID_CENTER_X) * CELL_WIDTH,
                            .y = GAME_CENTER_Y + (bestGrid.y - GRID_CENTER_Y) * CELL_HEIGHT
                        };
                        // log("BEST GRID: (%d %d)[%d, %d] => %f", bestGrid.x, bestGrid.y, bestGridPosition.x, bestGridPosition.y, bestEval);
                        move(me, hero, bestGridPosition);
                        sprintf(mes[hero->rank], "Pat [%d, %d]", bestGrid.x, bestGrid.y);
                        grid.eval[bestGrid.y][bestGrid.x] -= 1000.0;
                    }
                }

                #if DEBUG && DEBUG_EXPLORATION_MAP
                    logExplorationState(&grid);
                #endif

            #endif

            #if S_FARM
                for (char i = 0; i < data.nbHero; i++) {
                    Entity* h = data.heroes[i];
                    if (h->mode != M_FARM) continue;
                    move(me, h, orientedPosition(farmPos[h->modeRank]));
                    h->end = true;
                }
            #endif

            // Move to spider
            #if METHOD_2
                #if DEBUG && DEBUG_METHOD_NAME
                    log("M2\n");
                #endif 

                #if M2_URGENT_DEFENSE
                    Entity* bestMonster = findClosestMonsterFromBase(me);
                    if(bestMonster) {
                        // Entity* bestHero = findClosestHeroFromEntity(bestMonster);
                        Entity* bestHero = NULL;
                        float distMin = INF_MAX;
                        for (char i = 0; i < data.nbHero; i++) {
                            Entity* h = data.heroes[i];
                            if (h->end) continue;

                            float dist = data.distHeroEntity[h->rank][bestMonster->id];
                            if (dist < distMin) {
                                distMin = dist;
                                bestHero = h;
                            }
                        }

                        if (bestHero) {
                            bestHero->target = bestMonster->id;
                            bestMonster->nbAttacker++;
                            
                            float distHeroMonster = data.distHeroEntity[bestHero->rank][bestMonster->id];
                            char castOk = true;
                            if (me->mana < M2_URGENT_KEEP_MANA_MIN) castOk = false;
                            if (data.distBaseEntity[PLAYER_ME][bestMonster->id] > M2_URGENT_PUSH_STEP_MIN * MONSTER_SPEED + BASE_KILL_RADIUS) castOk = false;
                            #if M2_URGENT_ALWAYS_PUSH_OUT
                                castOk = shouldCastWind(me, bestHero, bestMonster, M3_PUSH_OUT_HEALTH_MIN);
                            #endif

                            if (
                                castOk &&
                                canCastSpell(me, bestHero, bestMonster, WIND_RADIUS) &&
                                data.distBaseEntity[PLAYER_ME][bestMonster->id] < BASE_DETECT_RADIUS
                            ) {
                                castWind(me, bestHero, bestMonster, op->basePos);
                                sprintf(mes[bestHero->rank], "Push %d", bestMonster->id);
                                // me->mana -= SPELL_COST;
                            }
                            #if 0
                            else if (distHeroMonster < MONSTER_SPEED) {
                                if (data.enemyInBase && !data.enemyInBase->isControlled) {
                                    float distEnemyMonster = data.distEnemyEntity[data.enemyInBase->rank][bestMonster->id];
                                    if (distEnemyMonster < WIND_RADIUS) {
                                        for (char i = 0; i < data.nbHero; i++) {
                                            Entity* h = data.heroes[i];
                                            if (h->mode != M_RUSH) continue;
                                            if (h->target > -1) continue;
                                            if (h->end) continue;
                                            

                                            if (canCastSpell(me, h, data.enemyInBase, CONTROL_RADIUS)) {
                                                castControl(me, h, data.enemyInBase, centerPos);
                                                h->end = true;
                                            }
                                        }
                                    }
                                }
                            }
                            #endif
                            else {
                                Point inter = { .x = bestMonster->pos.x, .y = bestMonster->pos.y, .dist = 0 };
                                int interStep = interception(&inter, bestMonster, bestHero);
                                move(me, bestHero, inter);
                                sprintf(mes[bestHero->rank], "Int %d", bestMonster->id);

                                #if M2_MULTI_TARGET_DEFENSE
                                    if (interStep <= M2_MULTI_TARGET_STEP_MAX) {
                                        optimizeMove(&inter, bestHero, bestMonster, interStep);
                                        move(me, bestHero, inter);
                                        sprintf(mes[bestHero->rank], "Int+ %d", bestMonster->id);
                                    }
                                #endif
                            }
                        }
                    }
                #endif

                #if 0 && M5_CHASE_ENEMY_IN_BASE
                    if (data.enemyInBase) data.monsters[data.nbMonster++] = data.enemyInBase;
                #endif

                // Find best attack target for each hero
                for(char e = 0; e < data.nbHero; e++) {
                    Entity* h = data.heroes[e];
                    if (h->end) continue;
                    if (h->target > -1) continue;

                    Entity* bestTarget = NULL;
                    float bestEval = INF_MIN;
                    for(int i = 0; i < data.nbMonster; i++) {
                        Entity* m = data.monsters[i];

                        float eval = 0.0;
                        if (m->nbAttacker == 0) eval = 0.1;

                        float heroMonsterDist = data.distHeroEntity[h->rank][m->id];
                        float myBaseMonsterDist = data.distBaseEntity[PLAYER_ME][m->id];
                        float opBaseMonsterDist = data.distBaseEntity[PLAYER_OP][m->id];

                        // Find another hero
                        #if M2_LEAVE_TARGET_TO_NEAREST
                            char otherOk = 0;
                            for (char e1 = 0; e1 < data.nbHero; e1++) {
                                Entity* h2 = data.heroes[e1];
                                if (h2->id == h->id) continue;

                                if (heroMonsterDist > data.distHeroEntity[h2->rank][m->id]) otherOk = 1;
                            }
                            if (otherOk) continue;
                        #endif

                        // If hero too far, continue
                        if (heroMonsterDist > WIND_RADIUS + (HERO_SPEED - MONSTER_SPEED) * myBaseMonsterDist / MONSTER_SPEED) continue;

                        #if M2_STAY_NEAR_ENEMY
                            if (h->mode == M_DEFENSE && data.enemyInBase) {
                                if (data.distEnemyEntity[data.enemyInBase->rank][m->id] > M2_NEAR_ENEMY_DIST_MAX) continue;
                            }
                        #endif

                        // If inside enemy base
                        if (opBaseMonsterDist <= M8_INNER_ATTACK_RADIUS_FACTOR * BASE_DETECT_RADIUS) continue;

                        #if M2_RESTRICT_DEFENDER_ATTACK
                            if (h->mode == M_DEFENSE) {
                                if (data.enemyInBase) {
                                    if (myBaseMonsterDist > M1_INVASION_RADIUS_FACTOR * BASE_DETECT_RADIUS) continue;
                                    if (data.distEnemyEntity[data.enemyInBase->rank][m->id] > CONTROL_RADIUS) continue;
                                } else {
                                    if (myBaseMonsterDist > M1_DEFENSE_RADIUS_FACTOR * BASE_DETECT_RADIUS) continue;
                                }
                            }
                        #endif

                        #if M2_IGNORE_IF_OUT
                            Point inter = { .x = m->pos.x, .y = m->pos.y, 0 };
                            int interStep = interception(&inter, m, h);

                            Point relPos = { .x = inter.x - m->dest.x, .y = inter.y - m->dest.y };
                            if (!isInGame(relPos)) continue;
                        #endif

                        // Attack near enemy base
                        if (h->mode == M_ATTACK) {
                            if (opBaseMonsterDist > M8_ATTACK_RADIUS_FACTOR * BASE_DETECT_RADIUS) continue;
                            if (opBaseMonsterDist < M8_INNER_ATTACK_RADIUS_FACTOR * BASE_DETECT_RADIUS) continue;
                            if (m->threatFor != MONSTER_THREAT_OP) {
                                if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 1000.0 / opBaseMonsterDist;
                            }
                            #if 0
                            if (m->nearBase == MONSTER_THREAT_OP) {
                                if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 100.0 / opBaseMonsterDist;
                            }
                            if (m->threatFor == MONSTER_THREAT_OP) {
                                if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 10.0 / opBaseMonsterDist;
                            }
                            #endif
                        }

                        #if 0
                        else if (h->mode == M_FARM) {
                            if (opBaseMonsterDist < BASE_DETECT_RADIUS) continue;
                            if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 100.0 / myBaseMonsterDist;
                        //#else
                        // Farm near enemy base
                        else if (h->rank < M8_NB_ENEMY_FARMER && game.round >= M8_FARM_ENEMY_ROUND_MIN) {
                            if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 1000.0 / opBaseMonsterDist;
                        }
                        #endif
                        // Default
                        else {
                            if (m->threatFor == MONSTER_THREAT_ME) {
                                if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 1000.0 / myBaseMonsterDist;
                            }
                            if (m->nearBase == MONSTER_THREAT_ME) {
                                if (m->nbAttacker < M2_ATTACK_NB_THREAT_MAX) eval += 10000.0 / myBaseMonsterDist;
                            }
                        }

                        if (eval > 0.0) eval = eval / ( heroMonsterDist + 1.0);

                        if (eval > bestEval) {
                            bestEval = eval;
                            bestTarget = m;
                        }
                    }

                    if (bestTarget) {
                        bestTarget->nbAttacker++;
                        attack(me, h, bestTarget);
                        sprintf(mes[h->rank], "%d", bestTarget->id);

                        // Compute intersection
                        Point inter = { .x = bestTarget->pos.x, .y = bestTarget->pos.y, .dist = 0 };
                        int interStep = 0;
                        #if 0
                        if (bestTarget->health > ATTACK_DAMAGE || data.distHeroEntity[h->rank][bestTarget->id] > ATTACK_RADIUS)
                        #endif
                        {
                            interStep = interception(&inter, bestTarget, h);
                            move(me, h, inter);
                            sprintf(mes[h->rank], "Int %d", bestTarget->id);
                        }

                        #if 1
                            if (data.distBaseHero[PLAYER_OP][h->rank] < BASE_FOG_RADIUS) {
                                float distBaseSpider = data.distBaseEntity[PLAYER_OP][bestTarget->id];
                                if (h->mode == M_ATTACK) {
                                    inter.x = bestTarget->pos.x + (WIND_RADIUS - 100) * (op->basePos.x - bestTarget->pos.x) / distBaseSpider;
                                    inter.y = bestTarget->pos.y + (WIND_RADIUS - 100) * (op->basePos.y - bestTarget->pos.y) / distBaseSpider;

                                    move(me, h, inter);
                                    sprintf(mes[h->rank], "Int %d", bestTarget->id);
                                }
                            }
                        #endif

                        #if M2_MULTI_TARGET
                            if (interStep <= M2_MULTI_TARGET_STEP_MAX && h->mode != M_ATTACK) {
                                optimizeMove(&inter, h, bestTarget, interStep);
                                move(me, h, inter);
                                sprintf(mes[h->rank], "Int+ %d", bestTarget->id);

                            }
                        #endif
                    }
                }
            #endif

            // Defense push
            #if METHOD_3
                if (hasEnoughMana(me) && me->mana >= M3_KEEP_MANA_MIN + SPELL_COST) {
                    #if DEBUG && DEBUG_METHOD_NAME
                        log("M3\n");
                    #endif
                    for (int i = 0; i < data.nbMonster; i++) {
                        Entity* m = data.monsters[i];
                        if (!isInBase(me, m)) continue;
                        if (!isCastable(m)) continue;

                        Entity* h = findClosestFreeHeroFromEntity(m);
                        if (!h) continue;

                        // If can kill monster, cancel
                        if (canKillThisRound(h, m)) continue;

                        // CHANGES: remove ?
                        if (data.distBaseEntity[PLAYER_ME][m->id] > MONSTER_SPEED + BASE_KILL_RADIUS) continue;
                        
                        #if !M3_ALWAYS_PUSH_OUT
                            if(!shouldCastWind(me, h, m, M3_PUSH_OUT_HEALTH_MIN)) continue;
                        #endif

                        // Activate spell
                        if (canCastSpell(me, h, m, WIND_RADIUS)) {
                            castWind(me, h, m, op->basePos);
                            sprintf(mes[h->rank], "Push %d", m->id);
                            if (!hasEnoughMana(me)) break;
                        }
                    }
                }
            #endif

            // Control
            #if METHOD_4
                if (hasEnoughMana(me)) {
                    #if DEBUG && DEBUG_METHOD_NAME
                    log("M4\n");
                    #endif 

                    for (int i = 0; i < data.nbMonster; i++) {
                        Entity* m = data.monsters[i];
                        if (m->nearBase > 0) continue;
                        // if (m->threatFor != MONSTER_THREAT_ME) continue;
                        if (!isCastable(m)) continue;

                        Entity* h = findClosestHeroFromEntity(m);
                        if (!h) continue;

                        // Monster killable this round
                        if (canKillThisRound(h, m)) continue;

                        if (h->mode == M_ATTACK) {
                            if (m->threatFor == MONSTER_THREAT_OP) continue;
                            if (m->health >= M4_ATTACK_LIGHT_HEALTH_MIN) continue;
                        } else {
                            if (m->threatFor != MONSTER_THREAT_ME) continue;
                            if (m->health < M4_ATTACK_HEALTH_MIN) continue;
                        }

                        #if M4_MONSTER_CONTROL_ATTACK
                            if (canCastSpell(me, h, m, CONTROL_RADIUS)) {
                                castControl(me, h, m, orientedPosition(controlTo));
                                castControl(me, h, m, op->basePos);
                                sprintf(mes[h->rank], "Ctl %d", m->id);
                                if (!hasEnoughMana(me)) break;
                            }
                        #endif

                        #if M4_SPIDER_CONTROL_ATTRACT
                            if (distHeroMonster > ATTACK_RADIUS) {
                                if (canCastSpell(me, h, m, CONTROL_RADIUS)) {
                                    castControl(n, me, h, m, h->pos);
                                    if (!hasEnoughMana(me)) break;
                                }
                            }
                        #endif
                    }
                }
                
            #endif

            // Shield hero
            #if METHOD_5
                if(enemyUseControl && hasEnoughMana(me)) {
                    #if DEBUG && DEBUG_METHOD_NAME
                        log("M5\n");
                    #endif

                    // Check if hero need to be shielded
                    for(char i = 0; i < data.nbEnemy; i++) {
                        Entity* eh = data.enemies[i];
                        if (!eh) continue;

                        #if !M5_PROTECT_FROM_ALL_ENEMIES
                            if(eh->id != data.enemyInBase->id) continue;
                        #endif

                        if (data.distBaseEntity[PLAYER_ME][eh->id] > M5_THREAT_RADIUS_FACTOR * BASE_FOG_RADIUS) continue;

                        #if M5_PUSH_ENEMY_OUT
                        for (int i = 0; i < data.nbHero; i++) {
                            Entity* h = data.heroes[i];
                            if (h->target > -1) continue;
                            if (!isCastable(h)) continue;

                            if (canCastSpell(me, h, eh, WIND_RADIUS)) {
                                castWind(me, h, eh, op->basePos);
                                h->target = eh->id;
                                if (!hasEnoughMana(me)) break;
                            }
                        }
                        #endif

                        #if M5_SHIELD_ME_NEAR_ENEMY
                            for (int i = 0; i < data.nbHero; i++) {
                                Entity* h = data.heroes[i];
                                // if (h->target > -1) continue;
                                if (!isCastable(h)) continue;

                                if (canCastSpell(op, eh, h, CONTROL_RADIUS)) {
                                    castShield(me, h, h);
                                    sprintf(mes[h->rank], "Shield");
                                    if (!hasEnoughMana(me)) break;
                                }
                            }
                        #endif
                    }
                }

                #if M5_CHASE_ENEMY_IN_BASE
                    if (data.enemyInBase && backHero->mode == M_DEFENSE && backHero->target == -1) {
                        move(me, backHero, data.enemyInBase->pos);
                    }
                #endif
            #endif

            // Shield monster
            #if METHOD_6
                if(data.strategy == M_ATTACK && hasEnoughMana(me)) {
                    #if DEBUG && DEBUG_METHOD_NAME
                        log("M6\n");
                    #endif 

                    for(int i = 0; i < data.nbMonster; i++) {
                        Entity* m = data.monsters[i];
                        if (!isCastable(m)) continue;
                        if (!isInBase(op, m)) continue;

                        Entity* h = findClosestHeroFromEntity(m);

                        if (canKillThisRound(h, m)) continue;

                        if (canCastSpell(me, h, m, SHIELD_RADIUS)) {
                            castShield(me, h, m);
                            sprintf(mes[h->rank], "Shield %d", m->id);
                            if (!hasEnoughMana(me)) break;
                        }
                    }
                }
            #endif

            // Attack wind
            #if METHOD_7
                // CHANGES : removed data.strategy == M_ATTACK
                if(hasEnoughMana(me)) {
                    #if DEBUG && DEBUG_METHOD_NAME
                        log("M7\n");
                    #endif 

                    for(int i = 0; i < data.nbMonster; i++) {
                        Entity* m = data.monsters[i];
                        if (!isCastable(m)) continue;

                        char pushOk = false;

                        float distMonsterEnemyBase = data.distBaseEntity[PLAYER_OP][m->id];

                        // Only push strong monsters unless monster can attack base if pushed
                        if (m->health < M7_HEALTH_MIN_PUSH && distMonsterEnemyBase > WIND_PUSH_FORCE + BASE_KILL_RADIUS) continue;

                        // Only attack in attack mode
                        // if (data.strategy != M_ATTACK && distMonsterEnemyBase > BASE_DETECT_RADIUS + WIND_PUSH_FORCE) continue;

                        // Attack if can attack
                        // if (distMonsterEnemyBase < M8_ATTACK_RADIUS_FACTOR * BASE_DETECT_RADIUS) pushOk = true;

                        if (data.strategy != M_ATTACK && distMonsterEnemyBase > BASE_DETECT_RADIUS) continue;

                        #if M8_ALWAYS_PUSH_IN_BASE
                            if (distMonsterEnemyBase <= BASE_DETECT_RADIUS + WIND_PUSH_FORCE) pushOk = true;
                        #endif

                        if (!pushOk) continue;

                        Entity* h = findClosestHeroFromEntity(m);

                        // Cancel if monster is killable this round
                        if (canKillThisRound(h, m)) continue;

                        if (canCastSpell(me, h, m, WIND_RADIUS)) {
                            castWind(me, h, m, op->basePos);
                            sprintf(mes[h->rank], "Push %d", m->id);
                            if (!hasEnoughMana(me)) break;
                        }
                    }
                }
            #endif
        }

        //--------------- RUN ACTIONS ----------------

        {
            for (char i = 0; i < data.nbHero; i++) {
                char message[50];
                sprintf(message, "%s %s", mode[data.heroes[i]->mode], mes[i]);
                sprintf(mes[i], "%s", message);
            }

            GET_TIME;
            SHOW_TIME("Compute time");
            maxTime = max(maxTime, TOP_TIME - startTime);
            SHOW_MAX_TIME;

            playMoves(&game, n);
        }

        //--------------- END ROUND ----------------

        {
            game.round++;
            for (char j = 0; j < GRID_HEIGHT; j++) {
                for (char i = 0; i < GRID_WIDTH; i++) {
                    if (grid.fog[j][i] > -1) grid.fog[j][i]++;
                    if (grid.fog[j][i] > M1_MAX_AGE) grid.fog[j][i] = -1;
                }
            }

            #if METHOD_0
                #if M0_COPY_STATE
                    memcpy(&memo, n, sizeof(Node));
                #endif
            #endif
        }
    }

    return 0;
}

/* ---------- FUNCTIONS ---------- */

void initGame(Game* g) {
    g->round = 0;
    Player* me = &state.players[PLAYER_ME];
    Player* op = &state.players[PLAYER_OP];

    int nbHeroes;
    scanf("%d%d", &me->basePos.x, &me->basePos.y);
    scanf("%d", &nbHeroes);

    g->orientation = me->basePos.x == 0 ? GAME_TOP_LEFT : GAME_BOTTOM_RIGHT;

    // Opponent base
    op->basePos.x = GAME_MAX_X - me->basePos.x;
    op->basePos.y = GAME_MAX_Y - me->basePos.y;

    // Player ids
    me->id = PLAYER_ME;
    op->id = PLAYER_OP;

    data.strategy = M_START;

    for (int i = 0; i < NB_ENTITY_MAX; i++) {
        entityState[i] = STATE_UNTOUCHED;
    }
    for (int j = 0; j < GRID_HEIGHT; j++) {
        for (int i = 0; i < GRID_WIDTH; i++) {
            grid.fog[j][i] = -1;
            grid.attack[j][i] = 1;
            grid.defense[j][i] = 1;
            grid.farm[j][i] = 1;
            grid.rush[j][i] = 1;
            grid.eval[j][i] = 0;

            int x = GAME_CENTER_X + (i - GRID_CENTER_X) * CELL_WIDTH;
            int y = GAME_CENTER_Y + (j - GRID_CENTER_Y) * CELL_WIDTH;
            Point cellCenter1 = {
                .x = x - CELL_WIDTH / 2,
                .y = y - CELL_HEIGHT / 2
            };
            Point cellCenter2 = {
                .x = x + CELL_WIDTH / 2,
                .y = y + CELL_HEIGHT / 2  
            };

            // Exclude grids that are inside bases
            if (isRectangleInCircle(me->basePos, BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.fog[j][i] = 0;
            if (isRectangleInCircle(op->basePos, BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.fog[j][i] = 0;

            // Define rush zones
            #if M1_RUSH_GRID
                if (isRectangleInCircle(op->basePos, M1_RUSH_RADIUS_FACTOR_IN * BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.rush[j][i] = 0;
                if (!isRectangleInCircle(op->basePos, M1_RUSH_RADIUS_FACTOR * BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.rush[j][i] = 0;
            #else
                grid.rush[j][i];
            #endif
            // Define attack zones
            if (isRectangleInCircle(op->basePos, M1_ATTACK_RADIUS_FACTOR_IN * BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.attack[j][i] = 0;
            if (!isRectangleInCircle(op->basePos, M1_ATTACK_RADIUS_FACTOR * BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.attack[j][i] = 0;

            // Define farm zones
            if (isRectangleInCircle(me->basePos, BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.farm[j][i] = 0;
            if (isRectangleInCircle(op->basePos, BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.farm[j][i] = 0;
            // if (!isRectangleInCircle(me->basePos, M1_FARM_RADIUS_FACTOR * BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.farm[j][i] = 0;
            if (g->orientation == GAME_TOP_LEFT && y < 4500) grid.farm[j][i] = 0;
            if (g->orientation == GAME_BOTTOM_RIGHT && y > 4500) grid.farm[j][i] = 0;

            // Define defense zones
            Point offsetBase = {.x = me->basePos.x, .y = me->basePos.y + (g->orientation == GAME_TOP_LEFT ? -2000 : 2000 )};
            if (isRectangleInCircle(me->basePos, BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.defense[j][i] = 0;
            if (!isRectangleInCircle(me->basePos, M1_DEFENSE_RADIUS_FACTOR * BASE_FOG_RADIUS, cellCenter1, cellCenter2)) grid.defense[j][i] = 0;
            // if (g->orientation == GAME_TOP_LEFT && y < 4500) grid.defense[j][i] = 0;
            // if (g->orientation == GAME_BOTTOM_RIGHT && y > 4500) grid.defense[j][i] = 0;
        }
    }

    #if DEBUG_EXPLORATION_MAP_INIT
        /*
        log("--------------- FOG MAP ---------------\n");
        for (char j = 0; j < GRID_HEIGHT; j++) {
            for (char i = 0; i < GRID_WIDTH; i++) {
                log("%d ", grid.fog[j][i]);
            }
            log("\n");
        }
        */
        log("----------------- FARM MAP ----------------\n");
        for (char j = 0; j < GRID_HEIGHT; j++) {
            for (char i = 0; i < GRID_WIDTH; i++) {
                log("%d ", grid.farm[j][i]);
            }
            log("\n");
        }
        log("---------------- ATTACK MAP ---------------\n");
        for (char j = 0; j < GRID_HEIGHT; j++) {
            for (char i = 0; i < GRID_WIDTH; i++) {
                log("%d ", grid.attack[j][i]);
            }
            log("\n");
        }
        log("--------------- DEFENSE MAP ---------------\n");
        for (char j = 0; j < GRID_HEIGHT; j++) {
            for (char i = 0; i < GRID_WIDTH; i++) {
                log("%d ", grid.defense[j][i]);
            }
            log("\n");
        }
        log("----------------- RUSH MAP ----------------\n");
        for (char j = 0; j < GRID_HEIGHT; j++) {
            for (char i = 0; i < GRID_WIDTH; i++) {
                log("%d ", grid.rush[j][i]);
            }
            log("\n");
        }
        log("----------------- END MAP -----------------\n");
    #endif
}

void readInputs(Node* n) {
    // Read players information
    for (char i = 0; i < NB_PLAYER; i++) {
        Player* p = &state.players[i];
        scanf("%d%d", &p->health, &p->mana);
    }

    Player* me = &n->players[PLAYER_ME];
    Player* op = &n->players[PLAYER_OP];

    // Amount of heros and monsters you can see
    scanf("%d", &n->nbEntities);

    // Read entities information
    for (int i = 0; i < n->nbEntities; i++) {
        Entity* e = &state.entities[i];
        scanf("%d%d%d%d%d%d%d%d%d%d%d", &e->id, &e->type, &e->pos.x, &e->pos.y, &e->shield, &e->isControlled, &e->health, &e->dest.x, &e->dest.y, &e->nearBase, &e->threatFor);

        if (e->type != ENTITY_MONSTER) {
            e->dest.x = 0;
            e->dest.y = 0;
            e->health = 30;
        }

        e->healthMax = e->health;
        e->nbAttacker = 0;
        e->visible = 1;
        e->target = -1;
        e->mode = -1;
        entityState[e->id] = STATE_IN_VIEW;

        if (e->nearBase == MONSTER_NEAR_BASE && abs(e->pos.x - op->basePos.x) < abs(e->pos.x - me->basePos.x)) e->nearBase = MONSTER_THREAT_OP;

        // log("--> AFTER: "); logEntity(e);
    }
}

void refreshState(Node* nDist, Node* nSrc) {
    for(int i = 0; i < nSrc->nbEntities; i++) {
        Entity* enSrc = &nSrc->entities[i];
        if (enSrc->type != ENTITY_MONSTER) continue;
        if (entityState[enSrc->id] < 0) continue;

        Entity* enDist = getEntity(nDist, enSrc->id);
        if (enDist) {
            if (isInView(nDist, enSrc) && !enDist->visible) {
                entityState[enDist->id] = STATE_DEAD;
                nDist->nbEntities--;
            }
            enDist->healthMax = max(enDist->healthMax, enSrc->healthMax);
        } else {
            enDist = &nDist->entities[nDist->nbEntities++];
            enDist->id = enSrc->id;
            enDist->type = ENTITY_MONSTER;

            #if M0_REFRESH_OUT_MONSTERS
                char wasInGame = isInGame(enSrc->pos);
            #endif

            enDist->pos.x = enSrc->pos.x + enSrc->dest.x;
            enDist->pos.x = enSrc->pos.y + enSrc->dest.y;
            enDist->shield = max(0, enSrc->shield - 1);
            enDist->isControlled = 0;
            enDist->health = enSrc->health;
            enDist->dest = enSrc->dest;
            enDist->nearBase = enSrc->nearBase;
            enDist->threatFor = enSrc->threatFor;
            enDist->nbAttacker = 0;
            enDist->visible = 0;
            entityState[enDist->id] = STATE_COMPUTED;

            #if M0_REFRESH_OUT_MONSTERS
                if (!wasInGame && !isInGame(enDist->pos)) {
            #else
                if (!isInGame(enDist->pos)) {
            #endif

                entityState[enDist->id] = STATE_DEACTIVATED;
                nDist->nbEntities--;
            }

            if (!isInView(nDist, enDist)) {
                entityState[enDist->id] = STATE_DEAD;
                nDist->nbEntities--;
            }
        }
    }
}

void computeSymmetry(Node* n) {
    for(int i = 0; i < n->nbEntities; i++) {
        Entity* en = &n->entities[i];
        if (en->type != ENTITY_MONSTER) continue;

        int symId = en->id + (en->id % 2 == 0 ? 1 : -1);
        if (entityState[symId] < 0) continue;

        if (!getEntity(n, symId)) {
            Entity* symEn = &n->entities[n->nbEntities++];
            symEn->id = symId;
            symEn->type = ENTITY_MONSTER;
            symEn->pos.x = GAME_MAX_X - en->pos.x;
            symEn->pos.y = GAME_MAX_Y - en->pos.y;
            symEn->shield = 0;
            symEn->isControlled = 0;
            symEn->health = en->healthMax;
            symEn->dest.x = -en->dest.x;
            symEn->dest.y = -en->dest.y;
            symEn->nearBase = en->nearBase;
            symEn->threatFor = en->threatFor == 0 ? 0 : 3 - en->threatFor;
            symEn->nbAttacker = 0;
            symEn->visible = 0;
            entityState[symId] = STATE_COMPUTED;
        }
    }
}

Entity* getEntity(Node* node, int id) {
    for(int i = 0; i < node->nbEntities; i++) {
        Entity* en = &node->entities[i];
        if (en->id == id) return en;
    }

    return NULL;
}

// Find closest hero
Entity* findClosestHeroFromEntity(Entity* e) {
    Entity* hero = NULL;
    float distMin = INF_MAX;

    for(int i = 0; i < data.nbHero; i++) {
        Entity* h = data.heroes[i];
        float dist = data.distHeroEntity[h->rank][e->id];

        if (dist < distMin) {
            distMin = dist;
            hero = h;
        }
    }
    return hero;
}

// Find closest hero without any target
Entity* findClosestFreeHeroFromEntity(Entity* e) {
    Entity* hero = NULL;
    float distMin = INF_MAX;

    for(char i = 0; i < data.nbHero; i++) {
        Entity* h = data.heroes[i];
        if (h->end) continue;
        if (h->target > -1 && h->target != e->id) continue;

        float dist = data.distHeroEntity[h->rank][e->id];

        if (dist < distMin) {
            distMin = dist;
            hero = h;
        }
    }
    return hero;
}

Entity* findClosestMonsterFromBase(Player* player) {
    Entity* closest = NULL;
    float distMin = INF_MAX;

    for(int i = 0; i < data.nbMonster; i++) {
        Entity* m = data.monsters[i];
        float distMonsterFromBase = data.distBaseEntity[player->id][m->id];

        char threatOk = false;
        if (isInBase(player, m)) threatOk = true;
        if (player->id == PLAYER_ME) {
            if (m->threatFor == MONSTER_THREAT_ME && distMonsterFromBase < BASE_FOG_RADIUS * M2_URGENT_RADIUS_FACTOR) threatOk = true;
        } else {
            if (m->threatFor == MONSTER_THREAT_OP && distMonsterFromBase < BASE_FOG_RADIUS * M2_URGENT_RADIUS_FACTOR) threatOk = true;
        }

        if (threatOk && distMonsterFromBase < distMin) {
            distMin = distMonsterFromBase;
            closest = m;
        }
    }
    return closest;
}

Entity* findBestMonsterToControl(Entity* h, int radius) {
    Entity* best = NULL;
    float distMin = INF_MAX;   
    for (int i = 0; i < data.nbMonster; i++) {
        Entity* m = data.monsters[i];
        if (m->nearBase > 0) continue;
        if (m->threatFor == MONSTER_THREAT_OP) continue;
        if (m->isControlled) continue;
        if (!isCastable(m)) continue;
        if (canKillThisRound(h, m)) continue;
        if (game.orientation == GAME_TOP_LEFT && m->pos.y < 4500) continue;
        if (game.orientation == GAME_BOTTOM_RIGHT && m->pos.y > 4500) continue;
        if (data.distHeroEntity[h->rank][m->id] > radius) continue;

        float distOpBaseMonster = data.distBaseEntity[PLAYER_OP][m->id];
        if (distOpBaseMonster < distMin) {
            best = m;
            distMin = distOpBaseMonster;
        }
    }

    return best;
}

Entity* getEnemyInBase(Player* p) {
    for (char e = 0; e < data.nbEnemy; e++) {
        Entity* oh = data.enemies[e];
        if (data.distBaseEntity[p->id][oh->id] > M5_THREAT_RADIUS_FACTOR * BASE_FOG_RADIUS) continue;

        return oh;
    }
    return NULL;
}

float distanceEntity(Entity* e1, Entity* e2) {
    return distance(e1->pos, e2->pos);
}

// Define an action
void setMove(Move* m, char a, int t, Point p) {
    m->action = a;
    m->target = t;
    m->pos = p;

    // log("New Move: "); logMove(m);
}

void playMoves(Game* g, Node* n) {
    for(int i = 0; i < data.nbHero; i++) {
        Entity* h = data.heroes[i];
        // log("%d", h->rank); logEntity(h);

        Move* m = heroBestMove(h);
        // logMove(m);

        #if DEBUG_ACTION_TEXT
            if (m->action == A_WAIT) {
                printf("WAIT Wait %s\n", mes[h->rank]);
            } else if (m->action == A_MOVE) {
                printf("MOVE %d %d %s\n", m->pos.x, m->pos.y, mes[h->rank]);
            } else if (m->action == A_WIND) {
                printf("SPELL WIND %d %d %s\n", m->pos.x, m->pos.y, mes[h->rank]);
            } else if (m->action == A_SHIELD) {
                printf("SPELL SHIELD %d %s\n", m->target, mes[h->rank]);
            } else if (m->action == A_CONTROL) {
                printf("SPELL CONTROL %d %d %d %s\n", m->target, m->pos.x, m->pos.y, mes[h->rank]);
            }
        #else
            if (m->action == A_WAIT) {
                printf("WAIT Wait\n");
            } else if (m->action == A_MOVE) {
                printf("MOVE %d %d\n", m->pos.x, m->pos.y);
            } else if (m->action == A_WIND) {
                printf("SPELL WIND %d %d\n", m->pos.x, m->pos.y);
            } else if (m->action == A_SHIELD) {
                printf("SPELL SHIELD %d\n", m->target);
            } else if (m->action == A_CONTROL) {
                printf("SPELL CONTROL %d %d %d\n", m->target, m->pos.x, m->pos.y);
            }
        #endif
    }
}

Move* heroBestMove(Entity* e) {
    return &state.players[PLAYER_ME].bestMove[e->rank];
}

char hasEnoughMana(Player* p) {
    return p->mana >= SPELL_COST;
}

char isCastable(Entity* e) {
    return e && e->shield <= 0;
}

char isInBase(Player* p, Entity* e) {
    if (p->id == PLAYER_ME) {
        return e->nearBase == MONSTER_THREAT_ME;
    }

    return e->nearBase == MONSTER_THREAT_OP;
}

char isInGame(Point pos) {
    if (pos.x < 0 || pos.x > GAME_MAX_X) return 0;
    if (pos.y < 0 || pos.y > GAME_MAX_Y) return 0;
    return 1;
}

char isInView(Node* n, Entity* e) {
    if (distance(n->players[PLAYER_ME].basePos, e->pos) <= BASE_FOG_RADIUS) return 1;

    for(int i = 0; i < n->nbEntities; i++) {
        Entity* h = &n->entities[i];
        if (h->type != ENTITY_HERO_ME) continue;

        float dist = distanceEntity(h, e);

        if (dist <= HERO_FOG_RADIUS) return 1;
    }

    return 0;
}

char canKillThisRound(Entity* hero, Entity* monster) {
    float distHeroMonster = data.distHeroEntity[hero->rank][monster->id];
    return (distHeroMonster <= ATTACK_RADIUS && monster->health <= ATTACK_DAMAGE);
}

char canKillBeforeOut(Entity* hero, Entity* monster) {
    float distHeroMonster = data.distHeroEntity[hero->rank][monster->id];
    return (distHeroMonster <= ATTACK_RADIUS && monster->health <= ATTACK_DAMAGE);
}

char shouldCastWind(Player* p, Entity* h, Entity* e, int minHealth) {
    if (data.enemyInBase) return 1;
    if (e->health >= minHealth) return 1;
    if (data.nbMonsterInBase[p->id] > M3_NB_SPIDER_BASE_MAX) return 1;

    // Compute monster step
    float myBaseMonsterDist = data.distBaseEntity[p->id][e->id];
    int nbStep = (0.99 + myBaseMonsterDist - BASE_KILL_RADIUS) / MONSTER_SPEED;

    Point inter = { .x = e->pos.x, .y = e->pos.y,  .dist = 0 };
    int interStep = interception(&inter, e, h);

    int round = interStep + e->health / ATTACK_DAMAGE;
    if (round > nbStep) return 1;
    return 0;
}

void wait(Player* p, Entity* h) {
    setMove(&p->bestMove[h->rank], A_MOVE, -1, zeroPos);
}

void move(Player* p, Entity* h, Point po) {
    setMove(&p->bestMove[h->rank], A_MOVE, -1, po);
}

int optimizeMove(Point* inter, Entity* hero, Entity* target, int step) {
    float bestEval = INF_MIN;
    int bestPack = 0;
    for (int d = 0; d < nbMoveGrid; d++) {
        Point* p = &moveGrid[d];
        if (p->dist > HERO_SPEED) continue;

        Point nextHeroPos = { .x = hero->pos.x + p->x, .y = hero->pos.y + p->y };

        float optDist = distance(nextHeroPos, *inter);
        char optStep = optDist / HERO_SPEED;
        if (optStep > step) continue;

        float eval = 0.0;

        char nbPackedTarget = 0;
        Entity* packedTarget[NB_ENTITY_PACKED_MAX];     // Reachable targets
        float distPackedTarget[NB_ENTITY_PACKED_MAX];

        if (target) {
            packedTarget[nbPackedTarget] = target;
            distPackedTarget[nbPackedTarget++] = optDist;
        }
        for (int k = 0; k < data.nbMonster; k++) {
            Entity* m = data.monsters[k];
            if (target && m->id == target->id) continue;

            float dist = distance(nextHeroPos, m->pos);

            #if M8_AVOID_KILLING_SPIDERS
                if (hero->mode == M_ATTACK && dist <= ATTACK_RADIUS) continue;
            #endif

            #if S_RUSH_AVOID_KILLING_SPIDERS
                if (hero->mode == M_RUSH && dist <= ATTACK_RADIUS) continue;
            #endif
                else if (dist > ATTACK_RADIUS) continue;

            packedTarget[nbPackedTarget] = m;
            distPackedTarget[nbPackedTarget++] = dist;
        }

        // log("IN LOOP %d -- ", d);

        float evalTarget = 0.0;
        char evalTargetOK = true;

        #if M8_AVOID_KILLING_SPIDERS
            if (hero->mode == M_ATTACK) evalTargetOK = false;
        #endif

        #if S_RUSH_AVOID_KILLING_SPIDERS
            if (hero->mode == M_RUSH ) evalTargetOK = false;;
        #endif

        if (evalTargetOK) {
            for (char k = 0; k < nbPackedTarget; k++) {
                Entity* pt = packedTarget[k];
                if ( pt->id == target->id) evalTarget += 100.0 / pow(distPackedTarget[k] + 1.0, 1.3);
                evalTarget += 100.0 / pow(distPackedTarget[k] + 1.0, 1.2);
            }
        }
        else {
            evalTarget += 100.0 / pow(optDist + 1.0, 1.2);
        }

        if (nbPackedTarget > bestPack) bestPack = nbPackedTarget;

        eval += evalTarget;

        #if 0
            Point distPoint = { .x = GAME_CENTER_X, .y = game.orientation == GAME_TOP_LEFT ? 0 : GAME_MAX_Y };
            float distTarget = distance(distPoint, nextHeroPos);
            eval += (nbPackedTarget) * 10.0 / pow(distTarget + 1.0, 0.7);
        #else
            eval += (nbPackedTarget - 1) * 1000.0;
        #endif

        if (eval > bestEval) {
            // log("TEST EVAL %d\n", d);
            bestEval = eval;
            inter = &nextHeroPos;
        }
    }

    return bestPack;
}

void attack(Player* p, Entity* h, Entity* m) {
    setMove(&p->bestMove[h->rank], A_MOVE, m->id, m->pos);
}

char canCastSpell(Player* p, Entity* e1, Entity* e2, int radius) {
    if (!hasEnoughMana(p)) return 0;
    if (!isCastable(e2)) return 0;
    if (p->id == PLAYER_ME && data.distHeroEntity[e1->rank][e2->id] > radius) return 0;
    if (p->id == PLAYER_OP && data.distEnemyEntity[e1->rank][e2->id] > radius) return 0;

    return 1;
}

void castWind(Player* p, Entity* h, Entity* m, Point pos) {
    Point windPos = {
        .x = pos.x - m->pos.x + h->pos. x,
        .y = pos.y - m->pos.y + h->pos.y
    };
    setMove(&p->bestMove[h->rank], A_WIND, -1, windPos);
    p->mana -= SPELL_COST;
}

void castControl(Player* p, Entity* h, Entity* e, Point po) {
    setMove(&p->bestMove[h->rank], A_CONTROL, e->id, po);
    e->isControlled = true;
    p->mana -= SPELL_COST;
}

void castShield(Player* p, Entity* h, Entity* e) {
    setMove(&p->bestMove[h->rank], A_SHIELD, e->id, zeroPos);
    p->mana -= SPELL_COST;
}

char heroRank(Entity* e) {
    return e->id % 3;
}

int interception(Point* inter, Entity* target, Entity* hero) {
    Point next = { .x = target->pos.x, .y = target->pos.y };
    for (char s = 0; s < M2_NB_INTERSTEP_MAX; s++) {
        next.x += target->dest.x;
        next.y += target->dest.y;

        float dist = distance(hero->pos, next);
        int step = dist / HERO_SPEED;
        if (step <= s) {
            inter->x = next.x;
            inter->y = next.y;
            return step;
        }
    }
    return M2_NB_INTERSTEP_MAX;
}

Point orientedPosition(Point pos) {
    if (game.orientation == GAME_TOP_LEFT) return pos;

    Point orientedPos = {
        .x = GAME_MAX_X - pos.x,
        .y = GAME_MAX_Y - pos.y,
        .dist = pos.dist
    };

    return orientedPos;
}

/* ---------- LOG FUNCTIONS -------------*/

void logEntity(Entity* e) {
    log("[%d] --> ENTITY:%d T%d((%d %d) -> (%d %d)) h:%d s:%d con:%d base:%d th:%d\n", entityState[e->id], e->id, e->type, e->pos.x, e->pos.y, e->dest.x, e->dest.y, e->health, e->shield, e->isControlled, e->nearBase, e->threatFor);
}

void logMove(Move* m) {
    log("--> MOVE action: %d target: %d(%d %d)\n", m->action, m->target, m->pos.x, m->pos.y);
}

void logBestTarget(Entity *h, Entity* e, int eval) {
    log("---> HERO:%d BEST:%d((%d %d) -> (%d %d))) eval:%d\n", h->id, e->id, e->pos.x, e->pos.y, e->dest.x, e->dest.y, eval);
}

void logExplorationState(Grid* g) {
    for (char j = 0; j < GRID_HEIGHT; j++) {
        for (char i = 0; i < GRID_WIDTH; i++) {
            int gr = g->fog[j][i];
            #if DEBUG_FARM_MAP
                gr = g->farm[j][i];
            #elif DEBUG_ATTACK_MAP
                gr = g->attack[j][i];
            #elif DEBUG_DEFENSE_MAP
                gr = g->defense[j][i];
            #elif DEBUG_RUSH_MAP
                gr = g->rush[j][i];
            #endif
            if (gr >= 0 && gr < 10)
                log("%d  ", gr);
            else
                log("%d ", gr);
        }
        log("\n");
    }
}

/* ---------- UTILS FUNCTIONS -------------*/

float distance(Point p1, Point p2) {
    return sqrt(distance2(p1, p2));
}

float distance2(Point p1, Point p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return (dx * dx) + (dy * dy);
}

char isPointInCircle(Point center, int radius, Point pos) {
    return distance2(center, pos) <= (radius * radius);
}

char isRectangleInCircle(Point center, int radius, Point pos1, Point pos2) {
    Point pos3 = { .x = pos1.x, .y = pos2.y };
    Point pos4 = { .x = pos2.x, .y = pos1.y };

    if (!isPointInCircle(center, radius, pos1)) return 0;
    if (!isPointInCircle(center, radius, pos2)) return 0;
    if (!isPointInCircle(center, radius, pos3)) return 0;
    if (!isPointInCircle(center, radius, pos4)) return 0;

    return 1;
}

float getAngle(Point p1, Point p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return atan2(dy, dx) * 180 / M_PI;
}

Point computeAnglePosition(Point p, int radius, float angle) {
    float x = radius * sin(M_PI * 2 * angle / 360);
    float y = radius * cos(M_PI * 2 * angle / 360);

    Point pa;
    pa.x = p.x + round(x);
    pa.y = p.y + round(y);

    return pa;
}