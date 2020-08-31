#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_ALPHA(c) (('A' <= (c)) && ((c) <= 'Z') || ('a' <= (c)) && ((c) <= 'z'))

/*!
 * Data structure for text lines. Contains a pointer to a null-terminated string and its length
 */
struct line {
    char *s;
    int len;
};

/*!
 * Data structure defining the node of a binary search tree. Contains a pointer to line and two
 * pointers to child nodes
 */
struct node {
    struct line *l;
    struct node *left;
    struct node *right;
};

void free_line(struct line *l);
void free_array_of_lines(struct line **lines, int n_lines);

struct line **read_lines_from_file(FILE *input, int *n_lines);
int count_chars_and_lines_in_file(FILE *input, int *n_lines);
char *read_file_to_buf(FILE *input, int *n_lines);

void q_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                int (*line_cmp)(const void *, const void *),
                                char *file_name);

void tree_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                   int (*line_cmp)(const void *, const void *),
                                   char *file_name);
struct node *generate_bst(const struct line *const *lines, int n_lines,
                          int (*line_cmp)(const void *, const void *));
struct node *insert_node_into_bst(struct node *parent, const struct line *l,
                                  int (*line_cmp)(const void *, const void *));
void write_bst_to_file(const struct node *current, FILE *output);
void delete_bst(struct node *current);

int line_ptr_cmp_direct(const void *arg1, const void *arg2);
int line_ptr_cmp_reverse(const void *arg1, const void *arg2);

int main(int argc, char *argv[])
{
    printf("Eugene Onegin sort\n\n"
           "Poem lines from input file (mandatory first command line argument) will be sorted and\n"
           "written to output file \"output.txt\". The order in which 2 lines are processed\n"
           "during comparison is direct (default) or reverse (set by optional command line\n"
           "argument \'--r\')\n\n");

    if (argc >= 2 && argc <= 3) {
        char *input_file_name = argv[1], *output_file_name = "output.txt";
        char sort_mode = 'd';

        if (argc == 3) {
            if ((strcmp(argv[2], "--r") == 0)) {
                sort_mode = argv[2][2];
            } else {
                printf("ERROR: invalid optional command line argument (must be \'--r\')\n");
                return 3;
            }
        }

        FILE *input = NULL;
        if ((input = fopen(input_file_name, "r")) != NULL) {
            int n_lines = 0;
            struct line **lines = NULL;

            if (((lines = read_lines_from_file(input, &n_lines)) != NULL) || (n_lines == 0)) {
                fclose(input);

                if (n_lines > 0) {
                    tree_sort_with_output_to_file(lines, n_lines,
                                                  (sort_mode == 'd') ? line_ptr_cmp_direct
                                                                     : line_ptr_cmp_reverse,
                                                  output_file_name);
                    free_array_of_lines(lines, n_lines);
                } else {
                    printf("Input file was empty, so output file wasn't created\n");
                }

                return 0;
            } else {
                fclose(input);
                printf("ERROR: get_lines_from_file returned NULL in main\n");
                return 2;
            }
        } else {
            printf("ERROR: invalid file name passed to fopen in main\n");
            return 1;
        }
    } else if (argc > 3) {
        printf("ERROR: invalid command line arguments - first must be the input file name,\n"
               "the second one is optional and must be equal to \'--r\' (sets the order in\n"
               "which 2 lines will be processed during comparison to reverse)\n");
        return 3;
    } else {
        printf("ERROR: run the program with command line arguments\n");
        return 3;
    }
}

/*!
 * Frees a line
 *
 * @param [in, out] l pointer to line to be freed
 */
void free_line(struct line *l)
{
    free(l->s);
    free(l);
}

/*!
 * Frees an array of pointers to line and each pointer to line it consists of
 *
 * @param [in, out] lines array of pointers to line to be freed
 * @param [in] n_lines array size
 */
void free_array_of_lines(struct line **lines, int n_lines)
{
    int i = 0;
    for (i = 0; i < n_lines; ++i) {
        free_line(lines[i]);
    }

    free(lines);
}

/*!
 * Reads lines from file input and returns an array containing them
 *
 * @param [in] input pointer to input file
 * @param [out] n_lines number of lines read
 *
 * @return pointer to an array of read lines
 *
 * @note Returns NULL in case of failure
 */
struct line **read_lines_from_file(FILE *input, int *n_lines)
{
    assert(input != NULL);
    assert(n_lines != NULL);

    char *buf = NULL;

    if ((buf = read_file_to_buf(input, n_lines)) != NULL && (*n_lines > 0)) {
        struct line **lines = NULL;

        if ((lines = calloc(*n_lines, sizeof(struct line *))) != NULL) {
            if ((*lines = calloc(1, sizeof(struct line))) != NULL) {
                char *tmp = strtok(buf, "\n");
                (*lines)->len = strlen(tmp);

                if (((*lines)->s = calloc((*lines)->len + 1, sizeof(char))) != NULL) {
                    strcpy((*lines)->s, tmp);

                    int i = 1;
                    for (i = 1; i < *n_lines; ++i) {
                        if ((lines[i] = calloc(1, sizeof(struct line))) != NULL) {
                            if ((tmp = strtok(NULL, "\n")) != NULL) {
                                lines[i]->len = strlen(tmp);

                                if ((lines[i]->s = calloc(lines[i]->len + 1, sizeof(char))) !=
                                NULL) {
                                    strcpy(lines[i]->s, tmp);
                                } else {
                                    printf("ERROR: calloc returned NULL in read_lines_from_file\n");
                                    return NULL;
                                }
                            } else {
                                printf("ERROR: strtok returned NULL unexpectedly in "
                                       "read_lines_from_file\n");
                                return NULL;
                            }
                        } else {
                            printf("ERROR: calloc returned NULL in read_lines_from_file\n");
                            return NULL;
                        }
                    }

                    return lines;
                } else {
                    printf("ERROR: calloc returned NULL in read_lines_from_file\n");
                    return NULL;
                }
            } else {
                printf("ERROR: calloc returned NULL in read_lines_from_file\n");
                return NULL;
            }
        } else {
            printf("ERROR: calloc returned NULL in read_lines_from_file\n");
            return NULL;
        }
    } else {
        if (*n_lines != 0) {
            printf("ERROR: read_file_to_buf returned NULL and n_lines is not equal to zero\n");
        }

        return NULL;
    }
}

/*!
 * Reads file input to a char buffer and counts the number of lines in it
 *
 * @param [in] input pointer to file
 * @param [out] n_lines number of lines in input
 *
 * @return pointer to the char buffer
 *
 * @note Returns NULL in case of failure or if the input file was empty
 */
char *read_file_to_buf(FILE *input, int *n_lines)
{
    assert(input != NULL);
    assert(n_lines != NULL);

    int n_chars = count_chars_and_lines_in_file(input, n_lines);
    char *buf = NULL;

    if (n_chars > 0) {
        if ((buf = calloc(n_chars + 1, sizeof(char))) != NULL) {
            if (fread(buf, sizeof(char), n_chars, input) == n_chars) {
                return buf;
            } else {
                printf("ERROR: fread read less chars than expected in read_file_to_buf\n");
                return NULL;
            }
        } else {
            printf("ERROR: calloc returned NULL in read_file_to_buf\n");
            return NULL;
        }
    } else {
        return NULL;
    }
}

/*!
 * Counts the number of characters and lines in file input, clears error flags of input and
 * rewinds it
 *
 * @param [in] input pointer to input file
 *
 * @return the number of lines
 */
int count_chars_and_lines_in_file(FILE *input, int *n_lines)
{
    assert(input != NULL);
    assert(n_lines != NULL);

    int pre_c = EOF, c = 0, n_chars = 0;
    *n_lines = 0;

    while ((c = fgetc(input)) != EOF) {
        if ((c == '\n') && (pre_c != '\n') && (pre_c != EOF)) {
            ++*n_lines;
        }

        pre_c = c;
        ++n_chars;
    }

    if ((pre_c != EOF) && (pre_c != '\n')) {
        ++*n_lines;
    }

    clearerr(input);
    rewind(input);

    return n_chars;
}

/*!
 * Converts an alpha to lower
 *
 * @param [in] c character
 *
 * @return Lowercase version of c or unmodified c, if it's not an alpha
 */
int to_lower(int c)
{
    if (IS_ALPHA(c)) {
        return tolower(c);
    } else {
        return c;
    }
}

/*!
 * Sorts lines using quick sort algorithm and writes the sorted lines to file_name
 *
 * @param [in] lines pointer to array of pointer to line
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to pointer to line comparator function
 * @param [in] file_name name of file to which the result of sorting is written
 *
 * @note Filename is opened in "w" mode
 */
void q_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                int (*line_cmp)(const void *, const void *),
                                char *file_name)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);
    assert(file_name != NULL);

    assert(n_lines > 0);

    struct line **lines_cp = NULL;
    if ((lines_cp = calloc(n_lines, sizeof(struct line *))) != NULL) {
        int i = 0;
        for (i = 0; i < n_lines; ++i) {
            if ((lines_cp[i] = calloc(1, sizeof(struct line))) != NULL) {
                lines_cp[i]->len = lines[i]->len;

                if ((lines_cp[i]->s = calloc(lines_cp[i]->len + 1, sizeof(char))) != NULL) {
                    strcpy(lines_cp[i]->s, lines[i]->s);
                } else {
                    printf("ERROR: calloc returned NULL in q_sort_with_output_to_file\n");
                    return;
                }
            } else {
                printf("ERROR: calloc returned NULL in q_sort_with_output_to_file\n");
                return;
            }
        }

        qsort(lines_cp, n_lines, sizeof(struct line *), line_cmp);

        FILE *f = NULL;

        if ((f = fopen(file_name, "w")) != NULL) {
            for (i = 0; i < n_lines; ++i) {
                if (lines_cp[i]->len != 0) {
                    fprintf(f, "%s\n", lines_cp[i]->s);
                }
            }
            fclose(f);
            free_array_of_lines(lines_cp, n_lines);
        } else {
            printf("ERROR: fopen returned NULL in q_sort_with_output_to_file\n");
        }
    } else {
        printf("ERROR: allocate_array_of_lines returned NULL in q_sort_with_output_to_file\n");
    }
}

/*!
 * Sorts lines using tree sort algorithm and writes the sorted lines to file_name
 *
 * @param [in] lines pointer to array of pointers to line
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to pointer to line comparator function
 * @param [in] file_name name of file to which the result of sorting is written
 *
 * @note Filename is opened in "w" mode
 */
void tree_sort_with_output_to_file(const struct line *const *lines, int n_lines,
                                   int (*line_cmp)(const void *, const void *),
                                   char *file_name)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);
    assert(file_name != NULL);

    assert(n_lines > 0);

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

/*!
 * Generates a binary search tree (BST) consisting of pointers to line from lines
 *
 * @param [in] lines pointer to array of pointers to line
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to pointer to line comparator function
 *
 * @return Pointer to root node
 *
 * @note Returns NULL in case of failure
 */
struct node *generate_bst(const struct line *const *lines, int n_lines,
                          int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    struct node *root = NULL;
    if ((root = calloc(1, sizeof(struct node))) != NULL) {
        if ((root->l = calloc(1, sizeof(struct line))) != NULL) {
            memcpy(root->l, lines[0], sizeof(struct line));

            root->left = NULL;
            root->right = NULL;

            int i = 1;
            for (i = 1; i < n_lines; ++i) {
                if (insert_node_into_bst(root, lines[i], line_cmp) == NULL) {
                    printf("ERROR: insert_node_into_bst returned NULL in generate_bst");
                    return NULL;
                }
            }

            return root;
        } else {
            printf("ERROR: calloc returned NULL in generate_bst");
            return NULL;
        }
    } else {
        printf("ERROR: calloc returned NULL in generate_bst");
        return NULL;
    }
}

/*!
 * Inserts node into BST
 *
 * @param [in, out] parent pointer to parent node
 * @param [in] l pointer to line
 * @param [in] line_cmp pointer to pointer to line comparator function
 *
 * @return Pointer to inserted node
 *
 * @note Returns NULL in case of failure
 */
struct node *insert_node_into_bst(struct node *parent, const struct line *l,
                                  int (*line_cmp)(const void *, const void *))
{
    assert(parent != NULL);
    assert(l != NULL);
    assert(line_cmp != NULL);

    if ((*line_cmp)(&parent->l, &l) >= 0) {
        if (parent->left == NULL) {
            if ((parent->left = calloc(1, sizeof(struct node))) != NULL) {
                if ((parent->left->l = calloc(1, sizeof(struct line))) != NULL) {
                    memcpy(parent->left->l, l, sizeof(struct line));
                    parent->left->left = NULL;
                    parent->left->right = NULL;

                    return parent->left;
                } else {
                    printf("ERROR: calloc returned NULL in insert_node_into_bst\n");
                    return NULL;
                }
            } else {
                printf("ERROR: calloc returned NULL in insert_node_into_bst\n");
                return NULL;
            }
        } else {
            return insert_node_into_bst(parent->left, l, line_cmp);
        }
    } else {
        if (parent->right == NULL) {
            if ((parent->right = calloc(1, sizeof(struct node))) != NULL) {
                if ((parent->right->l = calloc(1, sizeof(struct line))) != NULL) {
                    memcpy(parent->right->l, l, sizeof(struct line));
                    parent->right->left = NULL;
                    parent->right->right = NULL;

                    return parent->right;
                } else {
                    printf("ERROR: calloc returned NULL in insert_node_into_bst\n");
                    return NULL;
                }
            } else {
                printf("ERROR: calloc returned NULL in insert_node_into_bst\n");
                return NULL;
            }
        } else {
            return insert_node_into_bst(parent->right, l, line_cmp);
        }
    }
}

/*!
 * Writes BST to file output
 *
 * @param [in] current pointer to current node
 * @param [in] output pointer to output file
 */
void write_bst_to_file(const struct node *current, FILE *output)
{
    assert(current != NULL);
    assert(output != NULL);

    if (current->left != NULL) {
        write_bst_to_file(current->left, output);
    }

    if (current->l->len != 0) {
        fprintf(output, "%s\n", current->l->s);
    }

    if (current->right != NULL) {
        write_bst_to_file(current->right, output);
    }
}

/*!
 * Deletes BST
 *
 * @param [in, out] current pointer to current node
 */
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

/*!
 * Direct pointer to line comparator function. Compares line->s of two pointers to
 * line, discarding non-alpha characters (in terms of this their length is equal to the number of
 * alpha characters), processing the string in direct order
 *
 * @param [in] arg1 first pointer to pointer to line
 * @param [in] arg2 second pointer to pointer to line
 *
 * @return 0, if *arg1 is equal to *arg2, 1 if *arg1 is greater than *arg2, -1 otherwise
 */
int line_ptr_cmp_direct(const void *arg1, const void *arg2)
{
    assert(arg1 != NULL);
    assert(arg2 != NULL);

    const struct line *const *l1 = arg1, *const *l2 = arg2;
    const char *s1 = (*l1)->s, *s2 = (*l2)->s;

    while (*s1 != '\0' && *s2 != '\0') {
        if (to_lower(*s1) == to_lower(*s2)) {
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

    if ((*s1 == '\0') && (*s2 == '\0')) {
        return 0;
    } else if ((*s1 != '\0') && (*s2 != '\0')) {
        return (to_lower(*s1) > to_lower(*s2)) ? 1 : -1;
    } else {
        return (*s1 != '\0') ? 1 : -1;
    }
}

/*!
 * Direct pointer to line comparator function. Compares line->s of two pointers to
 * line, discarding non-alpha characters (in terms of this their length is equal to the number of
 * alpha characters), processing the string in reverse order
 *
 * @param [in] arg1 first pointer to pointer to line
 * @param [in] arg2 second pointer to pointer to line
 *
 * @return 0, if *arg1 is equal to *arg2, 1 if *arg1 is greater than *arg2, -1 otherwise
 */
int line_ptr_cmp_reverse(const void *arg1, const void *arg2)
{
    const struct line *const *l1 = arg1, *const *l2 = arg2;
    const char *s1 = (*l1)->s, *s2 = (*l2)->s;
    int i1 = (*l1)->len - 1, i2 = (*l2)->len - 1;

    while ((i1 >= 0) && (i2 >= 0)) {
        if (to_lower(s1[i1]) == to_lower(s2[i2])) {
            --i1;
            --i2;
        } else {
            if (IS_ALPHA(s1[i1]) && IS_ALPHA(s2[i2])) {
                break;
            }

            if (!IS_ALPHA(s1[i1])) {
                --i1;
            }

            if (!IS_ALPHA(s2[i2])) {
                --i2;
            }
        }
    }

    while ((i1 >= 0) && !IS_ALPHA(s1[i1])) {
        --i1;
    }
    while ((i2 >= 0) && !IS_ALPHA(s2[i2])) {
        --i2;
    }

    if ((i1 == -1) && (i2 == -1)) {
        return 0;
    } else if ((i1 != -1) && (i2 != -1)) {
        return (to_lower(s1[i1]) > to_lower(s2[i2])) ? 1 : -1;
    } else {
        return (i1 != -1) ? 1 : -1;
    }
}