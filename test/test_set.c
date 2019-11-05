#include "../src/co_set.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// 绘制二叉树
// printing tree in ascii

typedef struct asciinode_struct asciinode;

struct asciinode_struct {
    asciinode *left, *right;

    // length of the edge from this node to its children
    int edge_length;

    int height;

    int lablen;

    //-1=I am left, 0=I am root, 1=right
    int parent_dir;

    // max supported unit32 in dec, 10 digits max
    char label[11];
};

#define MAX_HEIGHT 1000
int lprofile[MAX_HEIGHT];
int rprofile[MAX_HEIGHT];
#define INFINITY (1 << 20)

// adjust gap between left and right nodes
int gap = 3;

// used for printing next node in the same level,
// this is the x coordinate of the next char printed
int print_next;

int MIN(int X, int Y) { return ((X) < (Y)) ? (X) : (Y); }

int MAX(int X, int Y) { return ((X) > (Y)) ? (X) : (Y); }

asciinode *build_ascii_tree_recursive(coset_t *tree, coset_node_t *t) {
    asciinode *node;

    if (t == coset_nil(tree)) return NULL;

    node = malloc(sizeof(asciinode));
    node->left = build_ascii_tree_recursive(tree, t->left);
    node->right = build_ascii_tree_recursive(tree, t->right);

    if (node->left != NULL) {
        node->left->parent_dir = -1;
    }

    if (node->right != NULL) {
        node->right->parent_dir = 1;
    }

    sprintf(node->label, "%d(%d)", coset_data(t, int), t->color);
    node->lablen = strlen(node->label);

    return node;
}

// Copy the tree into the ascii node structre
asciinode *build_ascii_tree(coset_t *tree, coset_node_t *t) {
    asciinode *node;
    if (t == NULL) return NULL;
    node = build_ascii_tree_recursive(tree, t);
    node->parent_dir = 0;
    return node;
}

// Free all the nodes of the given tree
void free_ascii_tree(asciinode *node) {
    if (node == NULL) return;
    free_ascii_tree(node->left);
    free_ascii_tree(node->right);
    free(node);
}

// The following function fills in the lprofile array for the given tree.
// It assumes that the center of the label of the root of this tree
// is located at a position (x,y).  It assumes that the edge_length
// fields have been computed for this tree.
void compute_lprofile(asciinode *node, int x, int y) {
    int i, isleft;
    if (node == NULL) return;
    isleft = (node->parent_dir == -1);
    lprofile[y] = MIN(lprofile[y], x - ((node->lablen - isleft) / 2));
    if (node->left != NULL) {
        for (i = 1; i <= node->edge_length && y + i < MAX_HEIGHT; i++) {
            lprofile[y + i] = MIN(lprofile[y + i], x - i);
        }
    }
    compute_lprofile(node->left, x - node->edge_length - 1,
                     y + node->edge_length + 1);
    compute_lprofile(node->right, x + node->edge_length + 1,
                     y + node->edge_length + 1);
}

void compute_rprofile(asciinode *node, int x, int y) {
    int i, notleft;
    if (node == NULL) return;
    notleft = (node->parent_dir != -1);
    rprofile[y] = MAX(rprofile[y], x + ((node->lablen - notleft) / 2));
    if (node->right != NULL) {
        for (i = 1; i <= node->edge_length && y + i < MAX_HEIGHT; i++) {
            rprofile[y + i] = MAX(rprofile[y + i], x + i);
        }
    }
    compute_rprofile(node->left, x - node->edge_length - 1,
                     y + node->edge_length + 1);
    compute_rprofile(node->right, x + node->edge_length + 1,
                     y + node->edge_length + 1);
}

// This function fills in the edge_length and
// height fields of the specified tree
void compute_edge_lengths(asciinode *node) {
    int h, hmin, i, delta;
    if (node == NULL) return;
    compute_edge_lengths(node->left);
    compute_edge_lengths(node->right);

    /* first fill in the edge_length of node */
    if (node->right == NULL && node->left == NULL) {
        node->edge_length = 0;
    } else {
        if (node->left != NULL) {
            for (i = 0; i < node->left->height && i < MAX_HEIGHT; i++) {
                rprofile[i] = -INFINITY;
            }
            compute_rprofile(node->left, 0, 0);
            hmin = node->left->height;
        } else {
            hmin = 0;
        }
        if (node->right != NULL) {
            for (i = 0; i < node->right->height && i < MAX_HEIGHT; i++) {
                lprofile[i] = INFINITY;
            }
            compute_lprofile(node->right, 0, 0);
            hmin = MIN(node->right->height, hmin);
        } else {
            hmin = 0;
        }
        delta = 4;
        for (i = 0; i < hmin; i++) {
            delta = MAX(delta, gap + 1 + rprofile[i] - lprofile[i]);
        }

        // If the node has two children of height 1, then we allow the
        // two leaves to be within 1, instead of 2
        if (((node->left != NULL && node->left->height == 1) ||
             (node->right != NULL && node->right->height == 1)) &&
            delta > 4) {
            delta--;
        }

        node->edge_length = ((delta + 1) / 2) - 1;
    }

    // now fill in the height of node
    h = 1;
    if (node->left != NULL) {
        h = MAX(node->left->height + node->edge_length + 1, h);
    }
    if (node->right != NULL) {
        h = MAX(node->right->height + node->edge_length + 1, h);
    }
    node->height = h;
}

// This function prints the given level of the given tree, assuming
// that the node has the given x cordinate.
void print_level(asciinode *node, int x, int level) {
    int i, isleft;
    if (node == NULL) return;
    isleft = (node->parent_dir == -1);
    if (level == 0) {
        for (i = 0; i < (x - print_next - ((node->lablen - isleft) / 2)); i++) {
            printf(" ");
        }
        print_next += i;
        printf("%s", node->label);
        print_next += node->lablen;
    } else if (node->edge_length >= level) {
        if (node->left != NULL) {
            for (i = 0; i < (x - print_next - (level)); i++) {
                printf(" ");
            }
            print_next += i;
            printf("/");
            print_next++;
        }
        if (node->right != NULL) {
            for (i = 0; i < (x - print_next + (level)); i++) {
                printf(" ");
            }
            print_next += i;
            printf("\\");
            print_next++;
        }
    } else {
        print_level(node->left, x - node->edge_length - 1,
                    level - node->edge_length - 1);
        print_level(node->right, x + node->edge_length + 1,
                    level - node->edge_length - 1);
    }
}

// prints ascii tree for given coset_node_t structure
void print_ascii_tree(coset_t *tree, coset_node_t *t) {
    asciinode *proot;
    int xmin, i;
    if (t == coset_nil(tree)) return;
    proot = build_ascii_tree(tree, t);
    compute_edge_lengths(proot);
    for (i = 0; i < proot->height && i < MAX_HEIGHT; i++) {
        lprofile[i] = INFINITY;
    }
    compute_lprofile(proot, 0, 0);
    xmin = 0;
    for (i = 0; i < proot->height && i < MAX_HEIGHT; i++) {
        xmin = MIN(xmin, lprofile[i]);
    }
    for (i = 0; i < proot->height; i++) {
        print_next = 0;
        print_level(proot, -xmin, i);
        printf("\n");
    }
    if (proot->height >= MAX_HEIGHT) {
        printf("(This tree is taller than %d, and may be drawn incorrectly.)\n",
               MAX_HEIGHT);
    }
    free_ascii_tree(proot);
}

// 绘制二叉树
void draw_set(coset_t *tree) { 
    print_ascii_tree(tree, tree->root); 
    printf("\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////

static int compare1(void *ud, const void *item1, const void *item2) {
    int v1 = *((int*)item1);
    int v2 = *((int*)item2); 
    return v1 - v2;
}

void print_set(coset_t *set) {
    coset_node_t *node = coset_end(set);
    while (node) {
        printf("%d ", coset_data(node, int));
        node = coset_prev(set, node);
    }
    printf("\n");
}

void test1() {
    // 随机增加，和随机删除
    coset_t *set = coset_new(sizeof(int), compare1, NULL);
    int i;
    for (i = 0; i < 20; ++i) {
        int v = co_randrange(0, 100);
        coset_add(set, &v);
    }


    for (i = 0; i < 20; ++i) {
        int v = co_randrange(0, 100);
        coset_delete(set, &v);
    }
    draw_set(set);
    print_set(set);
    printf("set size=%d\n", coset_size(set));
    coset_free(set);
}

void test2() {
    // 集合操作
    coset_t *set = coset_new(sizeof(int), compare1, NULL);
    coset_t *set2 = coset_new(sizeof(int), compare1, NULL);

    coset_add_tp(set, 35, int);
    coset_add_tp(set, 48, int);
    coset_add_tp(set, 21, int);
    coset_add_tp(set, 17, int);
    coset_add_tp(set, 89, int);
    coset_add_tp(set, 50, int);
    draw_set(set);
    coset_add_tp(set2, 35, int);
    coset_add_tp(set2, 49, int);
    coset_add_tp(set2, 20, int);
    coset_add_tp(set2, 17, int);
    coset_add_tp(set2, 56, int);
    coset_add_tp(set2, 52, int);
    draw_set(set2);

    coset_intersect(set, set2);
    draw_set(set);

    coset_union(set, set2);
    draw_set(set);

    coset_minus(set, set2);
    draw_set(set);

    coset_add_tp(set, 35, int);
    coset_add_tp(set, 48, int);
    coset_add_tp(set, 21, int);
    coset_add_tp(set, 17, int);
    coset_add_tp(set, 89, int);
    coset_add_tp(set, 50, int);
    draw_set(set);
    
    printf("set size=%d\n", coset_size(set));
    int v = 20;
    printf("data=%d, exist=%d\n", v, coset_exist(set, &v));
    v = 21;
    printf("data=%d, exist=%d\n", v, coset_exist(set, &v));
    v = 50;
    printf("data=%d, exist=%d\n", v, coset_exist(set, &v));
    v = 30;
    printf("data=%d, exist=%d\n", v, coset_exist(set, &v));

    coset_free(set);
    coset_free(set2);
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    test1();
    test2();
    return 0;
}
