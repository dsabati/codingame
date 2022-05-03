//--------------------------- CONSTANTS ---------------------------------
const NB_PLAYER = 2;                // Number or players
const NB_DIR = 6;                   // Number of possible directions
const NB_CELLS = 37;                // Number of cells on board
const MAX_DAY = 24;
const SIZE_MAX = 3;                 // Max size of a tree

/// ACTION RESTRICTIONS
const MAX_TREE_2 =                  4;
const MAX_TREE_1 =                  3;

const COMPLETE_COST =               4;
const COMPLETE_NB_TREE_MIN =        4;
const COMPLETE_NB_TREE_MIN_START =  6;
const COMPLETE_MIN_DAY_START =      12;
const COMPLETE_LAST_ROUND =         3;
const COMPLETE_DISABLED_IF_SHADOW = 1;

const GROW_COSTS =                  [1, 3, 7];
const GROW_DISABLED_IF_COMPLETE =   1;
const GROW_LAST_DAY_SIZE_2 =        1;
const GROW_LAST_DAY_SIZE_1 =        2;
const GROW_LAST_DAY_SIZE_0 =        3;

const SEED_MIN_SIZE =               2;
const SEED_ONLY_IF_FREE =           1;
const SEED_FIRST_AVAILABLE_TREE =   0;
const SEED_ONLY_BEST =              1;
const SEED_DISABLED_IF_COMPLETE =   0;
const SEED_DISABLED_IF_GROW =       1;
const SEED_LAST_DAY =               20;
const SEED_ALLOW_LAST_DAY_IF_FREE = 1;

//ACTION EVALUATION
const COMPLETE_DAY_MAX =            21;
const COMPLETE_VALUES =             [0, 2, 4];
const COMPLETE_FACTOR =             100;
const COMPLETE_RICH_FACTOR =        2;
const COMPLETE_BONUS =              1000;
const COMPLETE_END_BONUS =          1000;

const GROW_SIZE_FACTOR =            10;
const GROW_RICH_FACTOR =            2;
const GROW_BONUS =                  500;

const SEED_RICH_FACTOR =            50;
const SEED_BONUS =                  200;
const SEED_OPPONENT_SHADOW_BONUS =  10;
const SEED_FRIENDLY_SHADOW_MALUS =  -10;
const SEED_ALREADY_PLANTED_MALUS =  -100;
const SEED_DAY_MAX_MALUS =          -500;

// SIMULATION
const SIMULATION =                  1;
const MAX_TIME_ROUND =              99;         // Max time for decision process
const MAX_TIME_FIRST_ROUND =        999;        // Max time for decision process
const MONTE_CARLO_DAY =             21;
const USE_RATIO_BEST_MOVE =         1;
const ALL_WIN_PLAY_BEST =           1;
const RANDOM_OPPONENT_ACTIONS =     0;
const BEST_OPPONENT_ACTIONS =       1;

// STATE EVALUATION
const EVAL_ENABLED =                0;
const EVAL_SIZE_0 =                 1;          // EVALUATE SEEDS
const EVAL_SIZE_1 =                 2;          // EVALUATE TREES SIZE 1
const EVAL_SIZE_2 =                 4;          // EVALUATE TREES SIZE 2
const EVAL_SIZE_3 =                 8;          // EVALUATE TREES SIZE 3
const EVAL_RICHNESS_FACTOR =        1;
const EVAL_RICHNESS_POWER =         2;          // 1 * R ^ 2
const EVAL_PLAYER_SCORE_FACTOR =    0;
const EVAL_PLAYER_SUN_FACTOR =      100;

//--------------------------- ENUMS -----------------------------------

enum PLAYERS {
    ME = 0,
    OPPONENT = 1
}

enum ACTION_TYPES {
    WAIT = 'WAIT',
    SEED = 'SEED',
    GROW = 'GROW',
    COMPLETE = 'COMPLETE'
}

enum DIRECTIONS {
    RIGHT = 0,
    TOP_RIGHT = 1,
    TOP_LEFT = 2,
    LEFT = 3,
    BOTTOM_LEFT = 4,
    BOTTOM_RIGHT = 5
}

enum GAME_RESULT {
    UNKNOWN = -1,
    LOST = 0,
    WON = 1,
    DRAW = 2
}

//--------------------------- CLASS DEFINITION ------------------------

interface Position {
    readonly x: number;
    readonly y: number;
    readonly z: number;
}

interface BoardCell {
    readonly index: number;
    readonly richness: number;
    readonly neighbors: number[];
    position?: Position;
}

interface Cell {
    readonly index: number;
    tree: Tree;
    shadowSize: number[][];
}

interface Tree {
    readonly cellIndex: number;
    readonly player: PLAYERS;
    size: number;
    isDormant: boolean;
    spooky: boolean;
}

interface Action {
    readonly type: ACTION_TYPES;
    readonly targetCellIdx: number;
    readonly sourceCellIdx: number;
    cost: number;
    value: number;
}

interface Player {
    readonly type: PLAYERS;
    sun: number;
    score: number;
    income: number;
    isWaiting: boolean;
    nbTreesBySize: number[];
    bestAction: Action;
}

interface State {
    day: number;
    round: number;
    nutrients: number;
    players: [Player, Player];
    cells: Cell[];
    trees: Tree[];
    isSimulation: boolean;
    result: GAME_RESULT;
}

abstract class Board {
    static cells: BoardCell[];
    static cellsDistance: number[][];

    static initCells() {
        if (this.cells) return;

        this.cells = [];
        const numberOfCells = parseInt(readline());
        for (let i = 0; i < numberOfCells; i++) {
            var inputs: string[] = readline().split(' ');

            const index: number = parseInt(inputs[0]);      // 0 is the center cell, the next cells spiral outwards
            const richness: number = parseInt(inputs[1]);   // 0 if the cell is unusable, 1-3 for usable cells

            // retrieve neighbors
            const neigh: number[] = [];
            for (let i = 0; i < NB_DIR; i++)
                neigh.push(parseInt(inputs[i + 2]));
            this.cells.push(createBoardCell(index, richness, neigh));
        }

        this.computeCellsGrid();
    }

    static getCells(): Cell[] {
        return this.cells.map(cell => createCell(cell.index));
    }

    static getCellsDistance(cell1: Cell, cell2: Cell) {
        return this.getCellsDistanceByIndex(cell1.index, cell2.index);
    }

    static getCellsDistanceByIndex(cell1: number, cell2: number) {
        return this.cellsDistance[cell1][cell2];
    }

    private static computeCellsGrid(): void {
        let firstCell: BoardCell = this.cells[0];
        let nextCell: BoardCell;

        firstCell.position = createPosition(0, 0, 0);    // First cell is the center cell

        for (let c = 0; c < NB_CELLS; c++) {
            const cell = this.cells[c];
            for (let dir = 0; dir < NB_DIR; dir++) {
                nextCell = this.cells[cell.neighbors[dir]];
                if (!nextCell) continue;
                if (nextCell.position) continue;

                const offset = this.computeDistanceOffset(dir);
                nextCell.position = createPosition(cell.position.x + offset.x, cell.position.y + offset.y, cell.position.z + offset.z);
            }
        }

        this.cellsDistance = [];
        for (let c1 = 0; c1 < NB_CELLS; c1++) {
            const cellMatrix = [];

            for (let c2 = 0; c2 < NB_CELLS; c2++) {
                const distance = this.computeDistanceCells(c1, c2);
                cellMatrix.push(distance);
            }

            this.cellsDistance.push(cellMatrix)
        }
    }

    private static computeDistanceOffset(direction: DIRECTIONS): Position {
        switch (direction) {
            case DIRECTIONS.RIGHT: return createPosition(+1, -1, 0);
            case DIRECTIONS.TOP_RIGHT: return createPosition(+1, 0, -1);
            case DIRECTIONS.TOP_LEFT: return createPosition(0, +1, -1);
            case DIRECTIONS.LEFT: return createPosition(-1, +1, 0);
            case DIRECTIONS.BOTTOM_LEFT: return createPosition(-1, 0, +1);
            case DIRECTIONS.BOTTOM_RIGHT: return createPosition(0, -1, +1);
        }
    }

    private static computeDistanceCells(cell1Index: number, cell2Index: number): number {
        const cell1 = this.cells[cell1Index];
        const cell2 = this.cells[cell2Index];
        return Math.max(
            Math.abs(cell1.position.x - cell2.position.x),
            Math.abs(cell1.position.y - cell2.position.y),
            Math.abs(cell1.position.z - cell2.position.z)
        );
    }
}

interface SNode {
    index: number;
    state: State;
    parent: SNode;
    children: SNode[];
    depth: number;
    bestAction: Action;
}

// --------------------------- UTILS FUNCTIONS ------------------------

// LOGS

function log(message: string): void {
    console.error(message);
}

function logCell(cell: Cell): void {
    log(`CELL #${cell.index} => richness: ${Board.cells[cell.index].richness}, neighbors: ${JSON.stringify(Board.cells[cell.index].neighbors)}, shadowSize: ${JSON.stringify(cell.shadowSize)}`);
}

function logTree(tree: Tree): void {
    log(`TREE => cell: ${tree.cellIndex}, size: ${tree.size}, player: ${tree.player}, spooky: ${tree.spooky}`);
}

function logAction(action: Action): void {
    log(`===> ${action.type} ${action.sourceCellIdx ?? ''} ${action.targetCellIdx ?? ''} cost: ${action.cost}, value: ${action.value}`)
}

function logPlayer(player: Player): void {
    log(`PLAYER ${player.type}: ${player.sun} / ${player.score} / ${player.isWaiting}`);
}

function logState(state: State): void {
    const boardCells: number[] = [
        25, 24, 23, 22,
        26, 11, 10, 9, 21,
        27, 12, 3, 2, 8, 20,
        28, 13, 4, 0, 1, 7, 19,
        29, 14, 5, 6, 18, 36,
        30, 15, 16, 17, 35,
        31, 32, 33, 34,
    ];

    const me: Player = state.players[PLAYERS.ME];
    const opponent: Player = state.players[PLAYERS.OPPONENT];
    const sunDir: DIRECTIONS = state.day % NB_DIR;

    log(`-------------------- STATE DAY ${state.day + 1} ROUND ${state.round} ----------------------`);
    log(`ME (${me.score} / ${me.sun}) :: Opponent (${opponent.score} / ${opponent.sun})`);

    let board: string = "";

    for (let c = 0; c < NB_CELLS; c++) {
        const cell: Cell = state.cells[boardCells[c]];
        const tree: Tree = cell.tree;
        const treeIsMine = tree?.player === PLAYERS.ME;

        if (c === 0) board += "           ";
        else if (c === 4) board += "\n        ";
        else if (c === 9) board += "\n     ";
        else if (c === 15) board += "\n  ";
        else if (c === 22) board += "\n     ";
        else if (c === 28) board += "\n        ";
        else if (c === 33) board += "\n           ";


        board += ` ${tree ? (treeIsMine ? 'o' : 'x') : '.'}`;
        board += `${tree?.size ?? '.'}`;
        board += `(${cell.shadowSize[sunDir][PLAYERS.ME] ? 1 : 0}${cell.shadowSize[sunDir][PLAYERS.OPPONENT] ? 1 : 0})`;
    }

    log(board);

    if (state.isSimulation) return;

    // log(`----------------- My actions ----------------: `)
    // me.possibleActions.forEach(action => logAction(action));
    // log(`------------ Oppponent's actions ------------: `)
    // opponent.possibleActions.forEach(action => logAction(action));
}

// COPY
function copy<T>(obj: T): T {
    const copy = JSON.parse(JSON.stringify(obj));
    return copy;
}

function deepCopy<T>(obj: any) {
    if (obj === null || obj === undefined) {
        return obj;
    } else if (Array.isArray(obj)) {
        const array: T[] = [];
        let len = obj.length;
        while (len--) {
            const item = obj[len];
            array.push(deepCopy<typeof item>(item));
        }
        return array as T[];
    } else {
        const c: T = Object.assign({}, obj);
        const fields: string[] = Object.getOwnPropertyNames(obj);
        let len = fields.length;
        while (len--) {
            const field = obj[fields[len]];
            if (typeof field === 'object') {
                c[fields[len]] = deepCopy<typeof field>(field);
            }
        }
        return c;
    }
}

//----------------------- CREATION FUNCTION ---------------------------

function createPosition(x: number, y: number, z: number): Position {
    return { x, y, z };
}

function createBoardCell(index: number, richness: number, neighbors: number[]): BoardCell {
    return {
        index,
        richness,
        neighbors,
        position: null
    };
}

function createCell(index: number, shadowSize?: number[][]): Cell {
    if (!shadowSize) {
        shadowSize = [];
        for (let dir = 0; dir < NB_DIR; dir++)
            shadowSize.push([0, 0]);
    }

    return {
        index,
        tree: null,
        shadowSize
    };
}

function createTree(cellIndex: number, size: number, player: PLAYERS, isDormant: boolean, spooky?: boolean): Tree {
    return {
        cellIndex,
        size,
        player,
        isDormant,
        spooky: spooky ?? false
    };
}

function createAction(type: ACTION_TYPES, targetCellIdx?: number, sourceCellIdx?: number): Action {
    return {
        type,
        targetCellIdx,
        sourceCellIdx,
        value: 0,
        cost: 0
    };
}

function parseAction(line: string): Action {
    const parts = line.split(' ');

    switch (parts[0]) {
        case ACTION_TYPES.WAIT:
            return createAction(ACTION_TYPES.WAIT);

        case ACTION_TYPES.SEED:
            return createAction(ACTION_TYPES.SEED, parseInt(parts[2]), parseInt(parts[1]));

        default:
            return createAction(parts[0] as ACTION_TYPES, parseInt(parts[1]));
    }
}

function createPlayer(type: PLAYERS): Player {
    return {
        type,
        sun: 0,
        score: 0,
        income: 0,
        isWaiting: false,
        nbTreesBySize: [0, 0, 0, 0],
        bestAction: waitAction
    }
}

function createState(players: [Player, Player]): State {
    const state: State = {
        day: 0,
        round: -1,
        nutrients: 0,
        trees: [],
        cells: [],
        isSimulation: false,
        players,
        result: GAME_RESULT.UNKNOWN
    };

    return state;
}

function createNode(index: number, state: State, parent?: SNode, bestAction?: Action) {
    const node: SNode = {
        index,
        state,
        parent,
        children: [],
        depth: parent?.depth + 1 || 0,
        bestAction: bestAction
    }
    parent?.children.push(node);

    return node;
}

//----------------------- CLONE FUNCTION ---------------------------

function cloneCell(cell: Cell): Cell {
    return createCell(cell.index, cell.shadowSize);
}

function cloneTree(tree: Tree): Tree {
    return createTree(tree.cellIndex, tree.size, tree.player, tree.isDormant, tree.spooky);
}

function cloneAction(action: Action): Action {
    return createAction(action.type, action.targetCellIdx, action.sourceCellIdx);
}

function clonePlayer(player: Player): Player {
    return {
        type: player.type,
        sun: player.sun,
        score: player.score,
        income: player.income,
        isWaiting: player.isWaiting,
        nbTreesBySize: player.nbTreesBySize,
        bestAction: cloneAction(player.bestAction)
    }
}

function cloneState(state: State): State {
    const clonedPlayers: [Player, Player] = [clonePlayer(state.players[PLAYERS.ME]), clonePlayer(state.players[PLAYERS.OPPONENT])];
    const clonedState = createState(clonedPlayers);

    clonedState.day = state.day;
    clonedState.round = state.round;
    clonedState.nutrients = state.nutrients;
    clonedState.isSimulation = state.isSimulation;
    clonedState.result = state.result;

    let lenCells = state.cells.length;
    while (lenCells--) {
        const cell = cloneCell(state.cells[lenCells]);
        clonedState.cells.unshift(cell);
    }
    
    let lenTrees = state.trees.length;
    while (lenTrees--) {
        const tree = cloneTree(state.trees[lenTrees]);
        clonedState.trees.unshift(tree);
        clonedState.cells[tree.cellIndex].tree = tree;
    }

    return clonedState;
}

//-------------------------- MAIN ------------------------------------

function updateState(state: State) {
    state.day = parseInt(readline());
    state.nutrients = parseInt(readline());
    state.round++;

    for (let p = 0; p < NB_PLAYER; p++) {
        const player: Player = state.players[p];
        const inputs = readline().split(' ');
        const sunBefore = player.sun;

        player.sun = parseInt(inputs[0]);
        player.score = parseInt(inputs[1]);
        if (inputs[2]) player.isWaiting = inputs[2] !== '0';

        player.income = player.sun - sunBefore;
        player.nbTreesBySize = [0, 0, 0, 0];
    }

    for (let c = 0; c < NB_CELLS; c++) {
        const cell = state.cells[c];
        cell.tree = null;
        cell.shadowSize = [];

        for (let dir = 0; dir < NB_DIR; dir++)
            cell.shadowSize.push([0, 0]);
    }

    state.trees = [];
    const numberOfTrees = parseInt(readline());
    for (let i = 0; i < numberOfTrees; i++) {
        var inputs = readline().split(' ');
        const cellIndex = parseInt(inputs[0]);
        const size = parseInt(inputs[1]);
        const player: PLAYERS = inputs[2] !== '0' ? PLAYERS.ME : PLAYERS.OPPONENT;
        const isDormant = inputs[3] !== '0';

        const tree = createTree(cellIndex, size, player, isDormant);

        state.cells[cellIndex].tree = tree;
        state.trees.push(tree);

        // Compute trees by size
        state.players[player].nbTreesBySize[size]++;
    }

    const numberOfPossibleAction = parseInt(readline());
    for (let i = 0; i < numberOfPossibleAction; i++) {
        const possibleAction = readline();
        //state.players[PLAYERS.ME].possibleActions.push(parseAction(possibleAction));
    }
}

function evaluateShadows(state: State) {
    const sunDir: DIRECTIONS = state.day % NB_DIR;
    const nb_trees = state.trees.length;
    // Compute shadows from tree size
    for (let t = 0; t < nb_trees; t++) {
        const tree: Tree = state.trees[t];
        for (let dir = 0; dir < NB_DIR; dir++) {
            let cell: Cell = state.cells[tree.cellIndex];
            let tmpCell: Cell = cell;
            for (let i = 0, size = tree.size; i < size; i++) {
                tmpCell = state.cells[Board.cells[tmpCell.index].neighbors[dir]];
                if (!tmpCell) break;
                if (size <= tmpCell.shadowSize[dir][tree.player]) continue;

                tmpCell.shadowSize[dir][tree.player] = size;
            }
        }
    }

    // Check threat for each tree
    for (let t = 0; t < nb_trees; t++) {
        const tree: Tree = state.trees[t];
        const cell: Cell = state.cells[tree.cellIndex];
        tree.spooky = cell.shadowSize[sunDir][PLAYERS.OPPONENT] >= tree.size || cell.shadowSize[sunDir][PLAYERS.ME] >= tree.size;
    }
}

function findActions(state: State, player: Player): Action[] {
    const actions: Action[] = [createAction(ACTION_TYPES.WAIT)];
    const day = state.day;
    const sun = player.sun;
    let disableSeed = false;

    if (!player.isWaiting) {

        let nbTrees = state.trees.length;
        // Find complete and grow actions
        for (let t = 0; t < nbTrees; t++ ) {
            const tree = state.trees[t];
            if (tree.isDormant) continue;               // Iterate if tree already had an action this day
            if (tree.player !== player.type) continue;  // Iterate if tree current belong to player

            let completeAllowed = true;
            let growAllowed = true;
            let seedAllowed = true;
            let completeMin = 0;
            const treeSize = tree.size;
            const treeCell: Cell = state.cells[tree.cellIndex];

            // COMPLETE
            // Tree is max size && player has enough sun to complete it
            if (day < COMPLETE_MIN_DAY_START) completeMin = COMPLETE_NB_TREE_MIN_START;
            else if (day < MAX_DAY - COMPLETE_LAST_ROUND) completeMin = COMPLETE_NB_TREE_MIN;
            if (player.nbTreesBySize[SIZE_MAX] < completeMin) completeAllowed = false;
            else if (tree.size < SIZE_MAX) completeAllowed = false;
            else if (sun < COMPLETE_COST) completeAllowed = false;

            if (completeAllowed) {
                const action = createAction(ACTION_TYPES.COMPLETE, tree.cellIndex);
                action.cost = COMPLETE_COST;
                action.value = evaluateAction(state, player, action);
                actions.push(action);

                if (GROW_DISABLED_IF_COMPLETE) {
                    if (player.nbTreesBySize[SIZE_MAX] >= completeMin - 1
                        && player.nbTreesBySize[SIZE_MAX-1] > 0
                        && sun > COMPLETE_COST + GROW_COSTS[2] - 1)
                        growAllowed = false;
                }

                if (SEED_DISABLED_IF_COMPLETE)
                    seedAllowed = false;
            }
            
            // GROW
            const costGrow = (GROW_COSTS[tree.size] + player.nbTreesBySize[tree.size + 1]);
            if (treeSize >= SIZE_MAX) growAllowed = false;
            else if (sun < costGrow) growAllowed = false;
            else if (treeSize < 2 && player.nbTreesBySize[2] >= MAX_TREE_2) growAllowed = false;
            else if (treeSize < 1 && player.nbTreesBySize[1] >= MAX_TREE_1) growAllowed = false;
            else if (day > MAX_DAY - GROW_LAST_DAY_SIZE_2 && treeSize < 3) growAllowed = false;
            else if (day > MAX_DAY - GROW_LAST_DAY_SIZE_1 && treeSize < 2) growAllowed = false;
            else if (day > MAX_DAY - GROW_LAST_DAY_SIZE_0 && treeSize < 1) growAllowed = false;

            if (growAllowed) {
                const action = createAction(ACTION_TYPES.GROW, tree.cellIndex);
                action.cost = costGrow;
                action.value = evaluateAction(state, player, action);
                actions.push(action);

                if (SEED_DISABLED_IF_GROW)
                    seedAllowed = false;
            }

            // SEED
            // Player has enough sun to plant a seed
            const costSeed = player.nbTreesBySize[0];
            if (seedAllowed && day >= SEED_LAST_DAY && !(SEED_ALLOW_LAST_DAY_IF_FREE && costSeed == 0)) seedAllowed = false;
            else if (seedAllowed && sun < costSeed) seedAllowed = false;

            if (seedAllowed && !disableSeed) {
                if (SEED_ONLY_IF_FREE && costSeed !== 0) continue
                if (tree.size < SEED_MIN_SIZE) continue;

                // Iterate on all cells to find possible cells to plant seed
                for (let c = 0; c < NB_CELLS; c++) {
                    const cell: Cell = state.cells[c];
                    if (cell.tree) continue;                                            // Iterate because we don't like other trees
                    if (Board.cells[cell.index].richness === 0) continue;               // Iterate if cell is communist
                    const dist = Board.getCellsDistance(treeCell, cell);
                    if (dist > tree.size || dist === 1) continue;                      // Iterate because cell too far

                    const action = createAction(ACTION_TYPES.SEED, cell.index, tree.cellIndex);
                    action.cost = costSeed;
                    action.value = evaluateAction(state, player, action);
                    actions.push(action);

                    if (SEED_FIRST_AVAILABLE_TREE)
                        disableSeed = true;
                }
            }
        }
    }

    return actions;
}

function findBestAction(actions: Action[], player: Player, randomize: boolean = false): Action {
    let actionLen = actions.length;
    let bestAction: Action = actions[actionLen - 1];
    if (randomize && ( player.type === PLAYERS.ME || ( player.type == PLAYERS.OPPONENT && RANDOM_OPPONENT_ACTIONS))) {
        bestAction = actions[Math.floor(Math.random() * actionLen)];
    }
    else {
        while (actionLen--) {
            const action = actions[actionLen];

            if (action.value > bestAction.value) 
                bestAction = action;
        }
    }

    return bestAction;
}

// Evaluate given action depending on game state
function evaluateAction(state: State, player: Player, action: Action): number {
    let value: number = 0;

    if (action.type === ACTION_TYPES.WAIT) return value;

    const sunDir = state.day % NB_DIR;
    const cell = state.cells[action.targetCellIdx];
    const boardCell = Board.cells[cell.index];

    if (action.type === ACTION_TYPES.COMPLETE) {
        value += COMPLETE_BONUS;
        value += boardCell.richness * COMPLETE_RICH_FACTOR;
    }
    else if (action.type === ACTION_TYPES.GROW) {
        value += GROW_BONUS;
        value += cell.tree.size * GROW_SIZE_FACTOR;
        value += boardCell.richness * GROW_RICH_FACTOR;
    }
    else if (action.type === ACTION_TYPES.SEED) {
        value += cell.shadowSize[sunDir][PLAYERS.ME] * SEED_FRIENDLY_SHADOW_MALUS;
        value += cell.shadowSize[sunDir][PLAYERS.OPPONENT] * SEED_OPPONENT_SHADOW_BONUS;
        value += boardCell.richness * SEED_RICH_FACTOR;
    }

    return value;
}

function simulate(state: State, actions: Action[]): Action {
    let elapsedTime: number = 0;
    let total: number = 0;
    let bestAction: Action = state.players[PLAYERS.ME].bestAction;
    const won = [];
    const lost = [];
    const draw = [];
    const limitTime = state.round > 0 ? MAX_TIME_ROUND : MAX_TIME_FIRST_ROUND;

    let actionIndex = 0;
    const nbActions = actions.length;
    for (let i = 0; i < nbActions; i++) {
        won.push(0);
        lost.push(0);
        draw.push(0);
    }

    while (elapsedTime < limitTime) {
        // Create a copy of current state
        const simulatedState: State = cloneState(state);
        const me = simulatedState.players[PLAYERS.ME];
        const opponent = simulatedState.players[PLAYERS.OPPONENT];
        simulatedState.isSimulation = true;

        me.bestAction = actions[actionIndex];
        opponent.bestAction = findBestAction(findActions(state, opponent), opponent);

        while (simulatedState.result === GAME_RESULT.UNKNOWN) {
            simulatedState.players.forEach(player => {
                simulateAction(simulatedState, player, player.bestAction);
            });

            // End of day
            if (me.isWaiting && opponent.isWaiting) {
                simulatedState.day++;

                // End Game
                if (simulatedState.day === MAX_DAY) {
                    simulatedState.players.forEach(player => player.score += player.sun % 3);
                    if (me.score === opponent.score)
                        simulatedState.players.forEach(player => player.score += player.nbTreesBySize.reduce((acc, next, index) => index > 0 ? acc + next : 0));

                    simulatedState.result = me.score === opponent.score ? GAME_RESULT.DRAW : me.score > opponent.score ? GAME_RESULT.WON : GAME_RESULT.LOST;
                }
                // Start new day
                else {
                    evaluateShadows(simulatedState);
                    simulatedState.players.forEach(player => {
                        player.isWaiting = false;
                        player.income = 0;
                    });
                    for (let t = 0, len = simulatedState.trees.length; t < len; t++) {
                        const tree = simulatedState.trees[t];
                        tree.isDormant = false;
                        if (tree.spooky) continue;

                        simulatedState.players[tree.player].income += tree.size;
                        simulatedState.players[tree.player].sun += tree.size;
                    }
                }
            }

            simulatedState.round++;

            simulatedState.players.forEach(player => {
                const actions = findActions(simulatedState, player);
                player.bestAction = findBestAction(actions, player, true);
            });
        }
        
        const end = Date.now();
        elapsedTime = end - startRound;

        total++;
        if (simulatedState.result === GAME_RESULT.WON) {
            actions[actionIndex].value += 1;
            won[actionIndex]++;
        } else if (simulatedState.result === GAME_RESULT.LOST) {
            actions[actionIndex].value += -1;
            lost[actionIndex]++;
        } else
            draw[actionIndex]++;

        actionIndex++;
        if (actionIndex === nbActions) actionIndex = 0;

        //log('==================== SIMULATED END STATE ======================');
        //logState(simulatedState);
    }
    let lastRatio = 0;
    let nbWon = 0;
    log(`============== ACTIONS RESULTS: =================`);
    log(`Total games played: ${total}`)
    for (let i = 0; i < nbActions; i++) {
        const ratio = won[i] / (won[i] + lost[i] + draw[i]) * 100;
        log(`${actions[i].type} ${actions[i].sourceCellIdx ?? ''} ${actions[i].targetCellIdx ?? ''}: ${won[i]} / ${draw[i]} / ${lost[i]} ${ratio}%`)

        if (ratio === 100) nbWon++;
        if (lastRatio <= ratio && USE_RATIO_BEST_MOVE) {
            bestAction = actions[i];
            lastRatio = ratio;
        }
    }
    if (total < nbActions || (nbWon === nbActions && ALL_WIN_PLAY_BEST && !USE_RATIO_BEST_MOVE))
        bestAction = findBestAction(actions, me);
    log(`================ BEST ACTION: ===================`);
    logAction(bestAction)
    log(`=================================================`);

    return bestAction;
}

function simulateAction(state: State, player: Player, action: Action) {
    if (!action || action.type === ACTION_TYPES.WAIT) {
        player.isWaiting = true;
        return;
    }
    else if (action.type === ACTION_TYPES.COMPLETE) {
        const cell = state.cells[action.targetCellIdx];
        const boardCell = Board.cells[cell.index];

        state.trees.splice(state.trees.findIndex(t => t.cellIndex === cell.tree.cellIndex), 1);
        cell.tree = null;
        player.nbTreesBySize[SIZE_MAX]--;
        player.score += state.nutrients + COMPLETE_VALUES[boardCell.richness - 1];
        state.nutrients = Math.max(0, state.nutrients - 1);
    }
    else if (action.type === ACTION_TYPES.GROW) {
        const cell = state.cells[action.targetCellIdx];
        const tree = cell.tree;

        player.nbTreesBySize[tree.size]--;
        tree.isDormant = true;
        tree.size++;
        player.nbTreesBySize[tree.size]++;
    }
    else if (action.type === ACTION_TYPES.SEED) {
        const sourceTree = state.cells[action.sourceCellIdx].tree;
        const cell = state.cells[action.targetCellIdx];

        if (cell.tree && cell.tree.player !== player.type) {
            state.trees.splice(state.trees.findIndex(t => t.cellIndex === cell.tree.cellIndex), 1);
            state.players[cell.tree.player].nbTreesBySize[0]--;
            cell.tree = null;
        } else {
            const tree = createTree(cell.index, 0, player.type, true);
            state.trees.push(tree);
            cell.tree = tree;

            player.nbTreesBySize[0]++;
        }

        sourceTree.isDormant = true;
    }

    player.sun -= action.cost;
}

function playAction(player: Player, action: Action) {
    let stringifiedAction: string = "";

    switch (action.type) {
        case ACTION_TYPES.WAIT:
            stringifiedAction += ACTION_TYPES.WAIT;
            break;
        case ACTION_TYPES.SEED:
            stringifiedAction += `${ACTION_TYPES.SEED} ${action.sourceCellIdx} ${action.targetCellIdx}`;
            break;
        default:
            stringifiedAction += `${action.type} ${action.targetCellIdx}`;
    }

    console.log(`${stringifiedAction} ${stringifiedAction}`);
}

const waitAction: Action = createAction(ACTION_TYPES.WAIT);
const me: Player = createPlayer(PLAYERS.ME);
const opponent: Player = createPlayer(PLAYERS.OPPONENT);
const gameState: State = createState([me, opponent]);

// Init cells
Board.initCells();
gameState.cells = Board.getCells();

let startRound = 0;
while (true) {
    startRound = Date.now();

    // Update current state
    updateState(gameState);

    // Initialize shadows
    evaluateShadows(gameState);

    logState(gameState);

    const actions = findActions(gameState, me);
    me.bestAction = findBestAction(actions, me);

    // Simulate 
    if (SIMULATION) {
        const opponentActions = findActions(gameState, opponent);
        opponent.bestAction = findBestAction(actions, opponent, true);
        log(`================= MY ACTIONS ====================`);
        for (let i = 0, nbActions = actions.length; i < nbActions; i++)
            logAction(actions[i]);
        log(`============= OPPONENT'S ACTIONS ================`);
        for (let i = 0, nbActions = opponentActions.length; i < nbActions; i++)
            logAction(opponentActions[i]);

        me.bestAction = simulate(gameState, actions);
    }

    playAction(me, me.bestAction);
}