
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Maximum nodes for Dancing Links (729 rows + 324 columns + root)
#define MAX_NODES 10000
#define GRID_SIZE 9
#define BOX_SIZE 3
#define MAX_ROWS (GRID_SIZE * GRID_SIZE * GRID_SIZE) // 729
#define MAX_COLS (GRID_SIZE * GRID_SIZE * 4) // 324

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
    int rows, cols;
    int solutions[MAX_ROWS][GRID_SIZE];
    int solution_count;
    int grid[GRID_SIZE][GRID_SIZE]; // Sudoku grid
} DLX;

// Function prototypes
void initialize_dlx(DLX *dlx);
void insert_node(DLX *dlx, int row, int col);
void cover_column(Node *col);
void uncover_column(Node *col);
void search(DLX *dlx, int k);
void setup_sudoku_matrix(DLX *dlx);
void input_sudoku(DLX *dlx);
void print_sudoku(DLX *dlx);
void print_solutions(DLX *dlx);

// Initialize the Dancing Links matrix for Sudoku (729 rows, 324 columns)
void initialize_dlx(DLX *dlx) {
    dlx->rows = GRID_SIZE * GRID_SIZE * GRID_SIZE; // 729 possible placements
    dlx->cols = GRID_SIZE * GRID_SIZE * 4; // 324 constraints
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

    Node *col = dlx->root->right;
    cover_column(col);

    for (Node *r = col->down; r != col; r = r->down) {
        dlx->solutions[dlx->solution_count][k] = r->row_id;
        for (Node *j = r->right; j != r; j = j->right) {
            cover_column(j->column);
        }
        search(dlx, k + 1);
        if (dlx->solution_count >= 1) return; // Stop after first solution for Sudoku
        for (Node *j = r->left; j != r; j = j->left) {
            uncover_column(j->column);
        }
    }
    uncover_column(col);
}

// Setup the Dancing Links matrix for a blank Sudoku
void setup_sudoku_matrix(DLX *dlx) {
    initialize_dlx(dlx);
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            for (int n = 1; n <= GRID_SIZE; n++) {
                int row_id = r * GRID_SIZE * GRID_SIZE + c * GRID_SIZE + (n - 1);
                // Cell constraint: (r, c) has one number
                insert_node(dlx, row_id, r * GRID_SIZE + c);
                // Row constraint: number n in row r
                insert_node(dlx, row_id, GRID_SIZE * GRID_SIZE + r * GRID_SIZE + (n - 1));
                // Column constraint: number n in column c
                insert_node(dlx, row_id, 2 * GRID_SIZE * GRID_SIZE + c * GRID_SIZE + (n - 1));
                // Box constraint: number n in box
                int box = (r / BOX_SIZE) * BOX_SIZE + (c / BOX_SIZE);
                insert_node(dlx, row_id, 3 * GRID_SIZE * GRID_SIZE + box * GRID_SIZE + (n - 1));
            }
        }
    }
}

// Input a Sudoku puzzle
void input_sudoku(DLX *dlx) {
    printf("Enter the 9x9 Sudoku grid (0 for empty cells, 1-9 for filled cells):\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            scanf("%d", &dlx->grid[i][j]);
            if (dlx->grid[i][j] < 0 || dlx->grid[i][j] > 9) {
                printf("Invalid input! Use 0-9 only.\n");
                dlx->grid[i][j] = 0;
            }
        }
    }
    setup_sudoku_matrix(dlx);
    // Cover columns for pre-filled cells
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (dlx->grid[r][c] != 0) {
                int n = dlx->grid[r][c];
                int row_id = r * GRID_SIZE * GRID_SIZE + c * GRID_SIZE + (n - 1);
                for (Node *node = &dlx->nodes[0]; node < &dlx->nodes[dlx->node_count]; node++) {
                    if (node->row_id == row_id) {
                        for (Node *j = node; j != node->right; j = j->right) {
                            cover_column(j->column);
                        }
                        break;
                    }
                }
            }
        }
    }
    printf("Sudoku input complete.\n");
}

// Print the Sudoku grid
void print_sudoku(DLX *dlx) {
    printf("\nCurrent Sudoku Grid:\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        if (i % 3 == 0) printf("---------------------\n");
        for (int j = 0; j < GRID_SIZE; j++) {
            if (j % 3 == 0) printf("| ");
            printf("%d ", dlx->grid[i][j]);
        }
        printf("|\n");
    }
    printf("---------------------\n");
}

// Print solutions
void print_solutions(DLX *dlx) {
    if (dlx->solution_count == 0) {
        printf("\nNo solutions found.\n");
        return;
    }
    printf("\nFound %d solution(s):\n", dlx->solution_count);
    for (int s = 0; s < dlx->solution_count; s++) {
        printf("Solution %d:\n", s + 1);
        int grid[GRID_SIZE][GRID_SIZE] = {0};
        // Copy initial grid
        for (int i = 0; i < GRID_SIZE; i++)
            for (int j = 0; j < GRID_SIZE; j++)
                grid[i][j] = dlx->grid[i][j];
        // Fill solution
        for (int k = 0; k < GRID_SIZE * GRID_SIZE && dlx->solutions[s][k] != 0; k++) {
            int row_id = dlx->solutions[s][k];
            int r = row_id / (GRID_SIZE * GRID_SIZE);
            int c = (row_id / GRID_SIZE) % GRID_SIZE;
            int n = (row_id % GRID_SIZE) + 1;
            grid[r][c] = n;
        }
        // Print solution grid
        for (int i = 0; i < GRID_SIZE; i++) {
            if (i % 3 == 0) printf("---------------------\n");
            for (int j = 0; j < GRID_SIZE; j++) {
                if (j % 3 == 0) printf("| ");
                printf("%d ", grid[i][j]);
            }
            printf("|\n");
        }
        printf("---------------------\n");
    }
}

int main() {
    DLX dlx = {0};
    int choice;

    printf("Sudoku Solver Using Dancing Links (Algorithm X)\n");
    while (true) {
        printf("\nMenu:\n");
        printf("1. Input a new Sudoku puzzle\n");
        printf("2. Solve the Sudoku puzzle\n");
        printf("3. Display current Sudoku grid\n");
        printf("4. Display solution(s)\n");
        printf("5. Exit\n");
        printf("Enter your choice (1-5): ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                input_sudoku(&dlx);
                break;
            case 2:
                dlx.solution_count = 0;
                search(&dlx, 0);
                printf("Solving complete. Found %d solution(s).\n", dlx.solution_count);
                break;
            case 3:
                print_sudoku(&dlx);
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
