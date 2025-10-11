#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Maximum board size and nodes (adjust as needed)
#define MAX_N 20
#define MAX_ROWS (MAX_N * MAX_N) // Max possible queen placements
#define MAX_COLS (4 * MAX_N - 2) // Max constraints: rows + cols + diagonals
#define MAX_NODES (MAX_ROWS * 4) // Approx nodes (each row has ~4 1s)

// Node structure for Dancing Links
typedef struct Node {
    struct Node *left, *right, *up, *down, *column;
    int row_id, col_id, node_count;
} Node;

// Structure to hold Dancing Links matrix and solutions
typedef struct {
    Node *root;
    Node *headers[MAX_COLS];
    Node nodes[MAX_NODES];
    int node_count;
    int rows, cols, N;
    int solutions[MAX_ROWS][MAX_N];
    int solution_count;
    int board[MAX_N][MAX_N]; // Current board state
} DLX;

// Function prototypes
void initialize_dlx(DLX *dlx, int N);
void insert_node(DLX *dlx, int row, int col);
void cover_column(Node *col);
void uncover_column(Node *col);
void search(DLX *dlx, int k);
void setup_nqueens_matrix(DLX *dlx);
void input_board_size(DLX *dlx);
void print_board(DLX *dlx);
void print_solutions(DLX *dlx);

// Initialize the Dancing Links matrix for N-Queens
void initialize_dlx(DLX *dlx, int N) {
    dlx->N = N;
    dlx->rows = N * N; // One row per possible queen placement
    dlx->cols = 4 * N - 2; // Row, col, main diag, anti-diag constraints
    dlx->node_count = 0;
    dlx->solution_count = 0;
    dlx->root = &dlx->nodes[dlx->node_count++];
    dlx->root->left = dlx->root->right = dlx->root;
    dlx->root->up = dlx->root->down = dlx->root;

    // Initialize column headers
    for (int i = 0; i < dlx->cols; i++) {
        dlx->headers[i] = &dlx->nodes[dlx->node_count++];
        dlx->headers[i]->node_count = 0;
        dlx->headers[i]->col_id = i;
        dlx->headers[i]->left = dlx->root->left;
        dlx->headers[i]->right = dlx->root;
        dlx->root->left->right = dlx->headers[i];
        dlx->root->left = dlx->headers[i];
        dlx->headers[i]->up = dlx->headers[i];
        dlx->headers[i]->down = dlx->headers[i];
    }

    // Initialize board
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            dlx->board[i][j] = 0;
}

// Insert a node at (row, col) in the matrix
void insert_node(DLX *dlx, int row, int col) {
    Node *node = &dlx->nodes[dlx->node_count++];
    node->row_id = row;
    node->col_id = col;
    node->column = dlx->headers[col];

    // Link vertically
    node->down = dlx->headers[col];
    node->up = dlx->headers[col]->up;
    dlx->headers[col]->up->down = node;
    dlx->headers[col]->up = node;

    // Link horizontally
    Node *row_start = node;
    if (dlx->node_count > 1 && dlx->nodes[dlx->node_count-2].row_id == row) {
        row_start = &dlx->nodes[dlx->node_count-2];
        node->left = row_start;
        node->right = row_start->right;
        row_start->right->left = node;
        row_start->right = node;
    } else {
        node->left = node;
        node->right = node;
    }

    dlx->headers[col]->node_count++;
}

// Cover a column
void cover_column(Node *col) {
    col->right->left = col->left;
    col->left->right = col->right;
    for (Node *i = col->down; i != col; i = i->down) {
        for (Node *j = i->right; j != i; j = j->right) {
            j->down->up = j->up;
            j->up->down = j->down;
            j->column->node_count--;
        }
    }
}

// Uncover a column
void uncover_column(Node *col) {
    for (Node *i = col->up; i != col; i = i->up) {
        for (Node *j = i->left; j != i; j = j->left) {
            j->column->node_count++;
            j->down->up = j;
            j->up->down = j;
        }
    }
    col->right->left = col;
    col->left->right = col;
}

// Search for solutions
void search(DLX *dlx, int k) {
    if (dlx->root->right == dlx->root) {
        dlx->solution_count++;
        return;
    }

    // Choose column with fewest nodes
    Node *col = dlx->root->right;
    for (Node *j = col->right; j != dlx->root; j = j->right) {
        if (j->node_count < col->node_count && j->node_count > 0) {
            col = j;
        }
    }
    if (col->node_count == 0) return; // No solution possible

    cover_column(col);

    for (Node *r = col->down; r != col; r = r->down) {
        dlx->solutions[dlx->solution_count][k] = r->row_id;
        for (Node *j = r->right; j != r; j = j->right) {
            cover_column(j->column);
        }
        search(dlx, k + 1);
        for (Node *j = r->left; j != r; j = j->left) {
            uncover_column(j->column);
        }
    }
    uncover_column(col);
}

// Setup the Dancing Links matrix for N-Queens
void setup_nqueens_matrix(DLX *dlx) {
    initialize_dlx(dlx, dlx->N);
    for (int r = 0; r < dlx->N; r++) {
        for (int c = 0; c < dlx->N; c++) {
            int row_id = r * dlx->N + c;
            // Row constraint
            insert_node(dlx, row_id, r);
            // Column constraint
            insert_node(dlx, row_id, dlx->N + c);
            // Main diagonal (r - c + N - 1)
            int diag = r - c + dlx->N - 1;
            insert_node(dlx, row_id, 2 * dlx->N + diag);
            // Anti-diagonal (r + c)
            int anti_diag = r + c;
            insert_node(dlx, row_id, 2 * dlx->N + (2 * dlx->N - 1) + anti_diag);
        }
    }
}

// Input board size
void input_board_size(DLX *dlx) {
    printf("Enter board size N (1 to %d): ", MAX_N);
    scanf("%d", &dlx->N);
    if (dlx->N < 1 || dlx->N > MAX_N) {
        printf("Invalid board size! Must be between 1 and %d.\n", MAX_N);
        dlx->N = 0;
        return;
    }
    setup_nqueens_matrix(dlx);
    printf("Board size %dx%d set.\n", dlx->N, dlx->N);
}

// Print the current board
void print_board(DLX *dlx) {
    if (dlx->N == 0) {
        printf("\nNo board size set.\n");
        return;
    }
    printf("\nCurrent %dx%d Board:\n", dlx->N, dlx->N);
    for (int i = 0; i < dlx->N; i++) {
        for (int j = 0; j < dlx->N; j++) {
            printf("%c ", dlx->board[i][j] ? 'Q' : '.');
        }
        printf("\n");
    }
}

// Print all solutions
void print_solutions(DLX *dlx) {
    if (dlx->N == 0) {
        printf("\nNo board size set.\n");
        return;
    }
    printf("\nFound %d solution(s) for %dx%d N-Queens:\n", dlx->solution_count, dlx->N, dlx->N);
    for (int s = 0; s < dlx->solution_count; s++) {
        printf("Solution %d:\n", s + 1);
        int board[MAX_N][MAX_N] = {0};
        for (int k = 0; k < dlx->N && dlx->solutions[s][k] != 0; k++) {
            int row_id = dlx->solutions[s][k];
            int r = row_id / dlx->N;
            int c = row_id % dlx->N;
            board[r][c] = 1;
        }
        for (int i = 0; i < dlx->N; i++) {
            for (int j = 0; j < dlx->N; j++) {
                printf("%c ", board[i][j] ? 'Q' : '.');
            }
            printf("\n");
        }
        printf("\n");
    }
}

int main() {
    DLX dlx = {0};
    int choice;

    printf("N-Queens Solver Using Dancing Links (Algorithm X)\n");
    while (true) {
        printf("\nMenu:\n");
        printf("1. Input board size (N)\n");
        printf("2. Solve N-Queens problem\n");
        printf("3. Display current board\n");
        printf("4. Display all solutions\n");
        printf("5. Exit\n");
        printf("Enter your choice (1-5): ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                input_board_size(&dlx);
                break;
            case 2:
                if (dlx.N == 0) {
                    printf("Please set board size first (Option 1).\n");
                    break;
                }
                dlx.solution_count = 0;
                search(&dlx, 0);
                printf("Solving complete. Found %d solution(s).\n", dlx.solution_count);
                break;
            case 3:
                print_board(&dlx);
                break;
            case 4:
                print_solutions(&dlx);
                break;
            case 5:
                printf("Exiting program.\n");
                return 0;
            default:
                printf("Invalid choice! Please enter 1-5.\n");
        }
    }
    return 0;
}