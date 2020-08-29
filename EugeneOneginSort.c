#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BUF_SIZE 100

#define IS_ALPHA(c) (('A' <= (c)) && ((c) <= 'Z') || ('a' <= (c)) && ((c) <= 'z'))

int buf_size = DEFAULT_BUF_SIZE;

char *buf = NULL;

struct line {
    char *s;
    int len;
};

struct node {
    struct line *l;
    struct node *left;
    struct node *right;
};

struct line **generate_array_of_lines(int n_lines);
void free_line(struct line *l);
void free_array_of_lines(struct line **lines, int n_lines);

int count_lines_in_file(FILE *input);
struct line *read_line(FILE *input, struct line *l);
int read_line_to_buf(FILE *input, int buf_pos);

void q_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                int (*line_cmp)(const void *l1, const void *l2),
                                char *file_name);

void tree_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                   int (*line_cmp)(const void *l1, const void *l2),
                                   char *file_name);
struct node *generate_bst(const struct line *const *lines, int n_lines,
                          int (*line_cmp)(const void *l1, const void *l2));
struct node *add_node(struct node *parent, const struct line *l,
                      int (*line_cmp)(const void *l1, const void *l2));
void write_bst_to_file(struct node *current, FILE *output);
void delete_bst(struct node *current);

int line_cmp_direct(const void *arg1, const void *arg2);
int line_cmp_reverse(const void *arg1, const void *arg2);

int main(int argc, char *argv[])
{
    printf("Eugene Onegin sort\n\n"
           "Poem lines from input file (first command line argument) will be sorted and written\n"
           "to output file (second command line argument) in direct order (default) or in reverse\n"
           "order (set by the the third optional command line argument \'r\')\n\n");

    if (argc >= 3 && argc <= 4) {
        char *input_file_name = argv[1], *output_file_name = argv[2];
        char sort_mode = 'd';

        if (argc == 4) {
            if ((strcmp(argv[3], "--r") == 0)) {
                sort_mode = argv[3][2];
            } else {
                printf("ERROR: invalid optional third command line argument (must be \'r\')\n");
                return 3;
            }
        }

        FILE *input = NULL;
        if ((input = fopen(input_file_name, "r")) != NULL) {
            int n_lines = count_lines_in_file(input);
            struct line **lines = NULL;

            if ((lines = generate_array_of_lines(n_lines)) != NULL) {
                int i = 0;
                for (i = 0; i < n_lines; ++i) {
                    read_line(input, lines[i]);
                }
                fclose(input);
                free(buf);

                tree_sort_with_output_to_file(lines, n_lines,
                                              (sort_mode == 'd') ? line_cmp_direct :
                                              line_cmp_reverse, output_file_name);
                free_array_of_lines(lines, n_lines);

                return 0;
            } else {
                printf("ERROR: generate_array_of_lines returned NULL in main\n");
                return 2;
            }
        } else {
            printf("ERROR: invalid file name passed to fopen in main\n");
            return 1;
        }
    } else {
        printf("ERROR: invalid command line arguments - first must be the input file name,\n"
               "second must be the output file name, the third argument is optional and sets the\n"
               "sorting order to reverse (\'r\')\n");
        return 3;
    }
}

struct line **generate_array_of_lines(int n_lines)
{
    struct line **lines = NULL;
    if ((lines = malloc(n_lines * sizeof(struct line *))) != NULL) {
        int i = 0;
        for (i = 0; i < n_lines; ++i) {
            if ((lines[i] = malloc(sizeof(struct line))) == NULL) {
                printf("ERROR: malloc returned NULL in generate_array_of_lines\n");
                return NULL;
            }
        }

        return lines;
    } else {
        printf("ERROR: malloc returned NULL in generate_array_of_lines\n");
        return NULL;
    }
}

void free_line(struct line *l)
{
    free(l->s);
}

void free_array_of_lines(struct line **lines, int n_lines)
{
    int i = 0;
    for (i = 0; i < n_lines; ++i) {
        free_line(lines[i]);
    }

    free(lines);
}

struct line *read_line(FILE *input, struct line *l)
{
    assert(input != NULL);
    assert(l != NULL);

    if ((buf == NULL) && ((buf = malloc(buf_size * sizeof(char))) == NULL)) {
        printf("ERROR: malloc returned NULL in read_line\n");
        return NULL;
    } else {
        int len = read_line_to_buf(input, 0);

        if (len > 0) {
            l->len = len;

            if ((l->s = malloc(len + 1)) != NULL) {
                strcpy(l->s, buf);
                return l;
            } else {
                printf("ERROR: malloc returned NULL in read_line\n");
                return NULL;
            }
        } else {
            return NULL;
        }
    }
}

int read_line_to_buf(FILE *input, int buf_pos)
{
    if (fgets(buf + buf_pos, buf_size - buf_pos, input) != NULL) {
        size_t len = strlen(buf);

        if (buf[len - 1] == '\n') {
            buf[--len] = '\0';

            return len;
        } else {
            if (feof(input)) {
                return len;
            } else {
                buf_size *= 2;

                if ((buf = realloc(buf, buf_size)) != NULL) {
                    return read_line_to_buf(input, len);
                } else {
                    printf("ERROR: realloc returned NULL in read_line_to_buf\n");
                    return 0;
                }
            }
        }
    } else {
        return 0;
    }
}

int count_lines_in_file(FILE *input)
{
    assert(input != NULL);

    int pre_c = EOF, c = 0, counter = 0;

    while ((c = fgetc(input)) != EOF) {
        if (c == '\n') {
            ++counter;
        }

        pre_c = c;
    }

    if ((pre_c != EOF) && (pre_c != '\n')) {
        ++counter;
    }

    clearerr(input);
    rewind(input);

    return counter;
}

void q_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                int (*line_cmp)(const void *l1, const void *l2),
                                char *file_name)
{
    struct line **lines_cp = NULL;
    if ((lines_cp = generate_array_of_lines(n_lines)) != NULL) {
        int i = 0;
        for (i = 0; i < n_lines; ++i) {
            memcpy(lines_cp[i], lines[i], sizeof(struct line));
        }

        qsort(lines_cp, n_lines, sizeof(struct line *), line_cmp);

        FILE *f = NULL;

        if ((f = fopen(file_name, "w")) != NULL) {
            for (i = 0; i < n_lines; ++i) {
                fprintf(f, "%s\n", lines_cp[i]->s);
            }
            fclose(f);
            free_array_of_lines(lines_cp, n_lines);
        } else {
            printf("ERROR: fopen returned NULL in q_sort_with_output_to_file\n");
        }
    } else {
        printf("ERROR: generate_array_of_lines returned NULL in q_sort_with_output_to_file\n");
    }
}

void tree_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                   int (*line_cmp)(const void *l1, const void *l2),
                                   char *file_name)
{
    struct node *root = NULL;
    if ((root = generate_bst(lines, n_lines, line_cmp)) != NULL) {
        FILE *f = NULL;

        if ((f = fopen(file_name, "w")) != NULL) {
            write_bst_to_file(root, f);
            fclose(f);
            delete_bst(root);
        } else {
            printf("ERROR: fopen return NULL in tree_sort_with_output_to_file\n");
            delete_bst(root);
        }
    } else {
        printf("ERROR: generate_bst returned NULL in main\n");
    }
}

struct node *generate_bst(const struct line *const *lines, int n_lines,
                          int (*line_cmp)(const void *l1, const void *l2))
{
    assert(lines != NULL);

    assert(n_lines > 0);

    struct node *root = malloc(sizeof(struct node));

    if ((root->l = malloc(sizeof(struct line))) != NULL) {
        memcpy(root->l, lines[0], sizeof(struct line));

        root->left = NULL;
        root->right = NULL;

        int i = 1;
        for (i = 1; i < n_lines; ++i) {
            if (add_node(root, lines[i], line_cmp) == NULL) {
                printf("ERROR: add_node returned NULL in generate_bst");
                return NULL;
            }
        }

        return root;
    } else {
        printf("ERROR: malloc returned NULL in generate_bst");

        return NULL;
    }
}

struct node *add_node(struct node *parent, const struct line *l,
                      int (*line_cmp)(const void *l1, const void *l2))
{
    if (line_cmp(&parent->l, &l) > 0) {
        if (parent->left == NULL) {
            if ((parent->left = malloc(sizeof(struct node))) != NULL) {
                if ((parent->left->l = malloc(sizeof(struct line))) != NULL) {
                    memcpy(parent->left->l, l, sizeof(struct line));
                    parent->left->left = NULL;
                    parent->left->right = NULL;

                    return parent->left;
                } else {
                    printf("ERROR: malloc returned NULL in add_node\n");
                    return NULL;
                }
            } else {
                printf("ERROR: malloc returned NULL in add_node\n");
                return NULL;
            }
        } else {
            return add_node(parent->left, l, line_cmp);
        }
    } else if (line_cmp(&parent->l, &l) < 0) {
        if (parent->right == NULL) {
            if ((parent->right = malloc(sizeof(struct node))) != NULL) {
                if ((parent->right->l = malloc(sizeof(struct line))) != NULL) {
                    memcpy(parent->right->l, l, sizeof(struct line));
                    parent->right->left = NULL;
                    parent->right->right = NULL;

                    return parent->right;
                } else {
                    printf("ERROR: malloc returned NULL in add_node\n");
                    return NULL;
                }
            } else {
                printf("ERROR: malloc returned NULL in add_node\n");
                return NULL;
            }
        } else {
            return add_node(parent->right, l, line_cmp);
        }
    }
}

void write_bst_to_file(struct node *current, FILE *output)
{
    assert(current != NULL);
    assert(output != NULL);

    if (current->left != NULL) {
        write_bst_to_file(current->left, output);
    }

    fprintf(output, "%s\n", current->l->s);

    if (current->right != NULL) {
        write_bst_to_file(current->right, output);
    }
}

void delete_bst(struct node *current)
{
    assert(current != NULL);

    if (current->left != NULL) {
        delete_bst(current->left);
    }

    if (current->right != NULL) {
        delete_bst(current->right);
    }

    free(current);
}

int line_cmp_direct(const void *arg1, const void *arg2)
{
    const struct line *const *l1 = arg1, *const *l2 = arg2;

    char *s1 = (*l1)->s, *s2 = (*l2)->s;

    while (*s1 != '\0' && *s2 != '\0') {
        if (*s1 == *s2) {
            ++s1;
            ++s2;
        } else {
            if (IS_ALPHA(*s1) && IS_ALPHA(*s2)) {
                break;
            }

            if (!IS_ALPHA(*s1)) {
                ++s1;
            }

            if (!IS_ALPHA(*s2)) {
                ++s2;
            }
        }
    }

    while ((*s1 != '\0') && !IS_ALPHA(*s1)) {
        ++s1;
    }
    while ((*s2 != '\0') && !IS_ALPHA(*s2)) {
        ++s2;
    }

    if ((*s1 == *s2) && (*s1 == '\0')) {
        return 0;
    } else {
        if (*s1 == '\0') {
            return -1;
        } else if (*s2 == '\0') {
            return 1;
        } else {
            return (*s1 > *s2) ? 1 : -1;
        }
    }
}

int line_cmp_reverse(const void *arg1, const void *arg2)
{
    const struct line *const *l1 = arg1, *const *l2 = arg2;
    char *s1 = (*l1)->s + (*l1)->len - 1, *s2 = (*l2)->s + (*l2)->len - 1;

    while (s1 != (*l1)->s && s2 != (*l2)->s) {
        if (*s1 == *s2) {
            --s1;
            --s2;
        } else {
            if (IS_ALPHA(*s1) && IS_ALPHA(*s2)) {
                break;
            }

            if (!IS_ALPHA(*s1)) {
                --s1;
            }

            if (!IS_ALPHA(*s2)) {
                --s2;
            }
        }
    }

    while ((s1 != (*l1)->s) && !IS_ALPHA(*s1)) {
        --s1;
    }
    while ((s2 != (*l2)->s) && !IS_ALPHA(*s2)) {
        --s2;
    }

    if ((s1 == (*l1)->s) == (s2 == (*l2)->s)) {
        if (IS_ALPHA(*s1) && IS_ALPHA(*s2)) {
            if (*s1 == *s2) {
                return 0;
            } else {
                return (*s1 > *s2) ? 1 : -1;
            }
        } else if (!IS_ALPHA(*s1) && !IS_ALPHA(*s2)) {
            return 0;
        } else if (!IS_ALPHA(*s1)) {
            return -1;
        } else {
            return 1;
        }
    } else {
        if (s1 == (*l1)->s) {
            return -1;
        } else {
            return 1;
        }
    }
}