#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node structure for the dancing links matrix
typedef struct Node {
    struct Node *left, *right, *up, *down, *column;
    int row_id;     // Row identifier
    int col_id;     // Column identifier
} Node;

// Column header structure
typedef struct {
    Node node;      // Embedded node for linking
    int size;       // Number of 1s in this column
    int id;         // Column identifier
} Column;

// Function to initialize the matrix
Node* create_matrix(int cols) {
    Node* header = (Node*)malloc(sizeof(Node));
    header->left = header->right = header->up = header->down = header;
    header->row_id = -1;
    header->col_id = -1;
    
    // Create column headers
    Node* prev = header;
    for (int i = 0; i < cols; i++) {
        Column* col = (Column*)malloc(sizeof(Column));
        col->node.left = prev;
        col->node.right = header;
        col->node.up = &col->node;
        col->node.down = &col->node;
        col->node.column = &col->node;
        col->size = 0;
        col->id = i;
        
        prev->right = &col->node;
        header->left = &col->node;
        prev = &col->node;
    }
    return header;
}

// Function to add a row to the matrix
void add_row(Node* header, int row_id, int* cols, int num_cols) {
    Node* first = NULL;
    Node* prev = NULL;
    
    for (int i = 0; i < num_cols; i++) {
        // Find the column header
        Node* col = header->right;
        while (col != header && col->col_id != cols[i]) {
            col = col->right;
        }
        if (col == header) continue; // Column not found
        
        // Create new node
        Node* node = (Node*)malloc(sizeof(Node));
        node->row_id = row_id;
        node->col_id = cols[i];
        node->column = col;
        
        // Link vertically
        node->down = col->down;
        node->up = col;
        col->down->up = node;
        col->up = node;
        
        // Link horizontally
        if (first == NULL) {
            first = node;
            node->left = node;
            node->right = node;
        } else {
            node->right = first;
            node->left = first->left;
            first->left->right = node;
            first->left = node;
        }
        
        ((Column*)col)->size++;
    }
}

// Cover a column
void cover(Node* col) {
    col->right->left = col->left;
    col->left->right = col->right;
    
    for (Node* i = col->down; i != col; i = i->down) {
        for (Node* j = i->right; j != i; j = j->right) {
            j->down->up = j->up;
            j->up->down = j->down;
            ((Column*)j->column)->size--;
        }
    }
}

// Uncover a column
void uncover(Node* col) {
    for (Node* i = col->up; i != col; i = i->up) {
        for (Node* j = i->left; j != i; j = j->left) {
            ((Column*)j->column)->size++;
            j->down->up = j;
            j->up->down = j;
        }
    }
    col->right->left = col;
    col->left->right = col;
}

// Dancing Links algorithm
void search(Node* header, int* solution, int k, int* solutions, int* num_solutions, int max_solutions) {
    if (*num_solutions >= max_solutions) return;
    
    if (header->right == header) {
        // Solution found
        for (int i = 0; i < k; i++) {
            solutions[(*num_solutions) * max_solutions + i] = solution[i];
        }
        (*num_solutions)++;
        return;
    }
    
    // Choose column with minimum number of 1s
    Node* col = header->right;
    Node* min_col = col;
    int min_size = ((Column*)col)->size;
    
    for (Node* c = col->right; c != header; c = c->right) {
        if (((Column*)c)->size < min_size) {
            min_col = c;
            min_size = ((Column*)c)->size;
        }
    }
    
    if (min_size == 0) return; // No solution possible
    
    cover(min_col);
    
    for (Node* r = min_col->down; r != min_col; r = r->down) {
        solution[k] = r->row_id;
        
        // Cover columns
        for (Node* j = r->right; j != r; j = j->right) {
            cover(j->column);
        }
        
        search(header, solution, k + 1, solutions, num_solutions, max_solutions);
        
        // Uncover columns
        for (Node* j = r->left; j != r; j = j->left) {
            uncover(j->column);
        }
    }
    
    uncover(min_col);
}

// Display the matrix
void display_matrix(Node* header) {
    printf("\nCurrent Matrix (Column sizes):\n");
    for (Node* col = header->right; col != header; col = col->right) {
        printf("Col %d: %d nodes\n", col->col_id, ((Column*)col)->size);
    }
    printf("\n");
}

// Free the matrix memory
void free_matrix(Node* header) {
    Node* col = header->right;
    while (col != header) {
        Node* next_col = col->right;
        Node* row = col->down;
        while (row != col) {
            Node* next_row = row->down;
            free(row);
            row = next_row;
        }
        free(col);
        col = next_col;
    }
    free(header);
}

int main() {
    Node* header = NULL;
    int cols = 0, row_id = 0;
    int* solution = NULL;
    int* solutions = NULL;
    int num_solutions = 0;
    int max_solutions = 100;
    int max_rows = 100;
    
    while (1) {
        printf("\nDancing Links Operations:\n");
        printf("1. Create Matrix\n");
        printf("2. Add Row\n");
        printf("3. Display Matrix\n");
        printf("4. Run Dancing Links Algorithm\n");
        printf("5. Display Solutions\n");
        printf("6. Exit\n");
        printf("Enter choice: ");
        
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // Create Matrix
                if (header) {
                    free_matrix(header);
                    free(solution);
                    free(solutions);
                }
                printf("Enter number of columns: ");
                scanf("%d", &cols);
                header = create_matrix(cols);
                solution = (int*)malloc(max_rows * sizeof(int));
                solutions = (int*)malloc(max_solutions * max_rows * sizeof(int));
                num_solutions = 0;
                row_id = 0;
                printf("Matrix created with %d columns\n", cols);
                break;
                
            case 2: // Add Row
                if (!header) {
                    printf("Create matrix first!\n");
                    break;
                }
                printf("Enter number of 1s in row %d: ", row_id);
                int num_cols;
                scanf("%d", &num_cols);
                if (num_cols > cols) {
                    printf("Invalid: Too many columns\n");
                    break;
                }
                int* row_cols = (int*)malloc(num_cols * sizeof(int));
                printf("Enter column indices (0 to %d): ", cols-1);
                for (int i = 0; i < num_cols; i++) {
                    scanf("%d", &row_cols[i]);
                    if (row_cols[i] >= cols) {
                        printf("Invalid column index\n");
                        free(row_cols);
                        break;
                    }
                }
                add_row(header, row_id++, row_cols, num_cols);
                free(row_cols);
                printf("Row added\n");
                break;
                
            case 3: // Display Matrix
                if (!header) {
                    printf("Create matrix first!\n");
                    break;
                }
                display_matrix(header);
                break;
                
            case 4: // Run Algorithm
                if (!header) {
                    printf("Create matrix first!\n");
                    break;
                }
                num_solutions = 0;
                search(header, solution, 0, solutions, &num_solutions, max_solutions);
                printf("Algorithm completed. Found %d solutions\n", num_solutions);
                break;
                
            case 5: // Display Solutions
                if (!header) {
                    printf("Create matrix first!\n");
                    break;
                }
                if (num_solutions == 0) {
                    printf("No solutions found or algorithm not run\n");
                    break;
                }
                printf("\nSolutions found (%d):\n", num_solutions);
                for (int i = 0; i < num_solutions; i++) {
                    printf("Solution %d: Rows [", i + 1);
                    for (int j = 0; j < max_rows && solutions[i * max_rows + j] != 0; j++) {
                        printf("%d", solutions[i * max_rows + j]);
                        if (solutions[i * max_rows + j + 1] != 0) printf(", ");
                    }
                    printf("]\n");
                }
                break;
                
            case 6: // Exit
                if (header) {
                    free_matrix(header);
                    free(solution);
                    free(solutions);
                }
                printf("Exiting...\n");
                return 0;
                
            default:
                printf("Invalid choice\n");
        }
    }
    
    return 0;
}