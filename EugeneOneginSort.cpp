#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*!
 * Global char buffer for reading text file into
 */
static char* BUFFER = NULL;

/*!
 * Data structure defining the node of a binary search tree. Contains a pointer
 * to line of chars and two pointers to child nodes
 */
struct node {
    const char* line;
    struct node* left;
    struct node* right;
};

char** read_lines_from_file(const char* file_name, int* n_lines);
int count_lines_in_buffer();
int read_file_to_buffer(const char* file_name);

void q_sort_with_output_to_file(char** lines,
    int n_lines,
    int (*line_cmp)(const void*, const void*),
    const char* file_name);
void write_lines_to_file(const char* const* lines,
    int n_lines,
    const char* file_name);

void tree_sort_with_output_to_file(const char* const* lines,
    int n_lines,
    int (*line_cmp)(const void*, const void*),
    const char* file_name);
struct node* generate_bst(const char* const* lines,
    int n_lines,
    int (*line_cmp)(const void*, const void*));
struct node* insert_node_into_bst(struct node* parent,
    const char* line,
    int (*line_cmp)(const void*, const void*));
void write_bst_to_file(const struct node* current, const char* file_name);
void delete_bst(struct node* current);

int is_alpha(int c);
int to_lower(int c);
int line_cmp_direct(const void* arg1, const void* arg2);
int line_cmp_reversed(const void* arg1, const void* arg2);

int main(int argc, const char* argv[])
{
    printf(
        "Eugene Onegin sort\n\n"
        "Poem lines from input file (mandatory first command line argument) "
        "will be sorted and\n"
        "written to output file \"output.txt\". The order in which 2 lines "
        "are processed\n"
        "during comparison is direct (default) or reversed (set by optional "
        "command line\n"
        "argument \"-reversed\" (\"--r\"))\n\n");

    if (argc >= 2 && argc <= 3) {
        const char *input_file_name = argv[1], *output_file_name = "output.txt";
        char sort_mode = 'd';

        if (argc == 3) {
            if ((strcmp(argv[2], "--r") == 0) || (strcmp(argv[2], "-reversed") == 0)) {
                sort_mode = argv[2][2];
            } else {
                printf(
                    "ERROR: invalid optional command line argument (must be "
                    "\"-reversed\""
                    "or \"--r\")\n");
                return EXIT_FAILURE;
            }
        }

        int n_lines = 0;
        char** lines = NULL;

        if (((lines = read_lines_from_file(input_file_name, &n_lines)) != NULL) || (n_lines == 0)) {
            if (n_lines > 0) {
                q_sort_with_output_to_file(
                    lines, n_lines,
                    (sort_mode == 'd') ? line_cmp_direct : line_cmp_reversed,
                    output_file_name);

                free(BUFFER);
            } else {
                printf("Input file was empty, so output file wasn't created\n");
            }

            return EXIT_SUCCESS;
        } else {
            printf("ERROR: get_lines_from_file returned NULL in main\n");

            return EXIT_FAILURE;
        }
    } else if (argc > 3) {
        printf(
            "ERROR: invalid command line arguments - first must be the input "
            "file name,\n"
            "the second one is optional and must be equal to \"-reversed\" or "
            "\"--r\" (sets\n"
            "the order in which 2 lines will be processed during comparison to "
            "reversed)\n");

        return EXIT_FAILURE;
    } else {
        printf("ERROR: run the program with command line arguments\n");

        return EXIT_FAILURE;
    }
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
 *
 * @warning Utilizes global char buffer BUFFER. BUFFER must be freed by caller
 */
char** read_lines_from_file(const char* file_name, int* n_lines)
{
    assert(file_name != NULL);
    assert(n_lines != NULL);

    int error_code = 0;

    if (!(error_code = read_file_to_buffer(file_name)) && ((*n_lines = count_lines_in_buffer()) > 0)) {
        char** lines = NULL;

        if ((lines = (char**)calloc(*n_lines, sizeof(char*))) != NULL) {
            lines[0] = strtok(BUFFER + 1, "\n");
            lines[0][-1] = '\0';

            int i = 1;
            for (i = 1; i < *n_lines; ++i) {
                lines[i] = strtok(NULL, "\n");
                lines[i][-1] = '\0';
            }

            return lines;
        } else {
            printf("ERROR: calloc returned NULL in read_lines_from_file\n");
            return NULL;
        }
    } else {
        if (*n_lines != 0) {
            printf(
                "ERROR: read_file_to_buf returned %d and n_lines is not equal to "
                "zero\n",
                error_code);
        }

        return NULL;
    }
}

/*!
 * Reads file input to a global char buffer BUFFER. BUFFER must be freed by
 * caller
 *
 * @param [in] input pointer to file
 *
 * @return 0 in case success, otherwise an error code
 *
 * @note Returns -1 in case of failure, 1 if the input file was empty. Puts '\0'
 * in the beginning of the buffer (for the purpose of dividing strings by '\0')
 */
int read_file_to_buffer(const char* file_name)
{
    assert(file_name != NULL);

    FILE* input = NULL;

    if ((input = fopen(file_name, "r")) != NULL) {
        fseek(input, 0, SEEK_END);
        long n_chars = ftell(input);
        rewind(input);

        if (n_chars > 0) {
            if ((BUFFER = (char*)calloc(n_chars + 2l, sizeof(char))) != NULL) {
                if (fread(BUFFER + 1, sizeof(char), n_chars, input) == n_chars) {
                    if (fclose(input) != 0) {
                        printf("ERROR: fclose returned non-zero value\n");

                        return -1;
                    } else {
                        return 0;
                    }
                } else {
                    if (fclose(input) != 0) {
                        printf("ERROR: fclose returned non-zero value\n");
                    }

                    printf(
                        "ERROR: fread read less chars than expected in "
                        "read_file_to_buf\n");

                    return -1;
                }
            } else {
                printf("ERROR: calloc returned NULL in read_file_to_buf\n");

                return -1;
            }
        } else {
            return 1;
        }
    } else {
        printf("ERROR: invalid file name passed to fopen\n");

        return -1;
    }
}

/*!
 * Counts the number of lines in global buffer BUFFER. After finishing, clears
 * error flags of input and rewinds it
 *
 * @param [in] input pointer to input file
 *
 * @return the number of lines
 */
int count_lines_in_buffer(void)
{
    assert(BUFFER != NULL);

    int pre_c = EOF, n_lines = 0;
    const char* reader = BUFFER + 1;

    while (*reader) {
        if ((*reader == '\n') && (pre_c != '\n') && (pre_c != EOF)) {
            ++n_lines;
        }

        pre_c = *reader++;
    }

    if ((pre_c != EOF) && (pre_c != '\n')) {
        ++n_lines;
    }

    return n_lines;
}

void write_line_to_file(const char* line) { }

/*!
 * Sorts lines using the quick sort algorithm and writes the sorted lines to
 * file_name
 *
 * @param [in] lines pointer to an array of lines
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to line comparator function
 * @param [in] file_name name of file to which the result of sorting is written
 *
 * @note Filename is opened in "w" mode
 */
void q_sort_with_output_to_file(char** lines,
    int n_lines,
    int (*line_cmp)(const void*, const void*),
    const char* file_name)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);
    assert(file_name != NULL);

    assert(n_lines > 0);

    qsort(lines, n_lines, sizeof(struct line*), line_cmp);

    write_lines_to_file(lines, n_lines, file_name);
}

/*!
 * Writes lines to file named file_name
 *
 * @param lines pointer to an array of lines
 * @param n_lines size of array
 * @param file_name name of output file
 */
void write_lines_to_file(const char* const* lines,
    int n_lines,
    const char* file_name)
{
    FILE* output = NULL;

    if ((output = fopen(file_name, "w")) != NULL) {
        int i = 0;
        for (i = 0; i < n_lines; ++i) {
            const char* line_ptr = lines[i];

            while (isspace(*line_ptr)) {
                ++line_ptr;
            }

            if (isupper(line_ptr[0]) && islower(line_ptr[1])) {
                fprintf(output, "%s\n", line_ptr);
            }
        }
        fclose(output);
    } else {
        printf("ERROR: fopen returned NULL in q_sort_with_output_to_file\n");
    }
}

/*!
 * Sorts lines using the tree sort algorithm and writes the sorted lines to
 * file_name
 *
 * @param [in] lines pointer to an array of lines
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to line comparator function
 * @param [in] file_name name of file to which the result of sorting is written
 *
 * @note Filename is opened in "w" mode
 */
void tree_sort_with_output_to_file(const char* const* lines,
    int n_lines,
    int (*line_cmp)(const void*, const void*),
    const char* file_name)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);
    assert(file_name != NULL);

    assert(n_lines > 0);

    struct node* root = NULL;
    if ((root = generate_bst(lines, n_lines, line_cmp)) != NULL) {
        write_bst_to_file(root, file_name);
        delete_bst(root);
    } else {
        printf("ERROR: generate_bst returned NULL in main\n");
    }
}

/*!
 * Generates a binary search tree (BST) consisting of pointers to line from
 * lines
 *
 * @param [in] lines pointer to an array of pointers to line
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to line comparator function
 *
 * @return Pointer to root node
 *
 * @note Returns NULL in case of failure
 */
struct node* generate_bst(const char* const* lines,
    int n_lines,
    int (*line_cmp)(const void*, const void*))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    struct node* root = NULL;
    if ((root = (struct node*)calloc(1, sizeof(struct node))) != NULL) {
        root->line = lines[0];

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
}

/*!
 * Inserts node into BST
 *
 * @param [in, out] parent pointer to parent node
 * @param [in] line pointer to line of chars
 * @param [in] line_cmp to pointer to line comparator function
 *
 * @return Pointer to inserted node
 *
 * @note Returns NULL in case of failure
 */
struct node* insert_node_into_bst(struct node* parent,
    const char* line,
    int (*line_cmp)(const void*, const void*))
{
    assert(parent != NULL);
    assert(line != NULL);
    assert(line_cmp != NULL);

    if ((*line_cmp)(&parent->line, &line) >= 0) {
        if (parent->left == NULL) {
            if ((parent->left = (struct node*)calloc(1, sizeof(struct node))) != NULL) {
                parent->left->line = line;
                parent->left->left = NULL;
                parent->left->right = NULL;

                return parent->left;
            } else {
                printf("ERROR: calloc returned NULL in insert_node_into_bst\n");
                return NULL;
            }
        } else {
            return insert_node_into_bst(parent->left, line, line_cmp);
        }
    } else {
        if (parent->right == NULL) {
            if ((parent->right = (struct node*)calloc(1, sizeof(struct node))) != NULL) {
                parent->right->line = line;
                parent->right->left = NULL;
                parent->right->right = NULL;

                return parent->right;
            } else {
                printf("ERROR: calloc returned NULL in insert_node_into_bst\n");
                return NULL;
            }
        } else {
            return insert_node_into_bst(parent->right, line, line_cmp);
        }
    }
}

/*!
 * Writes BST to file output
 *
 * @param [in] current pointer to current node
 * @param [in] output pointer to output file
 */
void write_bst_to_file(const struct node* current, const char* file_name)
{
    assert(current != NULL);
    assert(file_name != NULL);

    static int invoked = 0;
    static FILE* output = NULL;

    if (!invoked) {
        if ((output = fopen(file_name, "w")) == NULL) {
            printf("ERROR: invalid file name passed to fopen\n");

            return;
        } else {
            invoked = 1;
        }
    }

    if (current->left != NULL) {
        write_bst_to_file(current->left, file_name);
    }

    const char* line_ptr = current->line;

    while (isspace(*line_ptr)) {
        ++line_ptr;
    }

    if (isupper(line_ptr[0]) && islower(line_ptr[1])) {
        fprintf(output, "%s\n", line_ptr);
    }

    if (current->right != NULL) {
        write_bst_to_file(current->right, file_name);
    }
}

/*!
 * Deletes BST
 *
 * @param [in, out] current pointer to current node
 */
void delete_bst(struct node* current)
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
 *  Checks if c is an alpha
 *
 * @param [in] c
 *
 * @return 1 if c is an alpha, 0 otherwise
 */
int is_alpha(int c)
{
    return (('A' <= (c)) && ((c) <= 'Z')) || (('a' <= (c)) && ((c) <= 'z'));
}

/*!
 * Converts an alpha to lower
 *
 * @param [in] c character
 *
 * @return Lowercase version of c or unmodified c, if it'str not an alpha
 */
int to_lower(int c)
{
    if (is_alpha(c)) {
        return tolower(c);
    } else {
        return c;
    }
}

/*!
 * Direct line comparator function. Compares two line, discarding non-alpha
 * characters (in terms of this their length is equal to the number of alpha
 * characters), processing the string in direct order
 *
 * @param [in] arg1 first pointer to pointer to line
 * @param [in] arg2 second pointer to pointer to line
 *
 * @return 0, if *arg1 is equal to *arg2, 1 if *arg1 is greater than *arg2, -1
 * otherwise
 */
int line_cmp_direct(const void* arg1, const void* arg2)
{
    assert(arg1 != NULL);
    assert(arg2 != NULL);

    const char *line1 = *((const char**)arg1), *line2 = *((const char**)arg2);

    while (*line1 && *line2) {
        if (to_lower(*line1) == to_lower(*line2)) {
            ++line1;
            ++line2;
        } else {
            if (is_alpha(*line1) && is_alpha(*line2)) {
                break;
            }

            if (!is_alpha(*line1)) {
                ++line1;
            }

            if (!is_alpha(*line2)) {
                ++line2;
            }
        }
    }

    while (*line1 && !is_alpha(*line1)) {
        ++line1;
    }
    while (*line2 && !is_alpha(*line2)) {
        ++line2;
    }

    if (!*line1 && !*line2) {
        return 0;
    } else if (*line1 && *line2) {
        return (to_lower(*line1) > to_lower(*line2)) ? 1 : -1;
    } else {
        return (*line1) ? 1 : -1;
    }
}

/*!
 * Reversed line comparator function. Compares two lines, discarding non-alpha
 * characters (in terms of this their length is equal to the number of alpha
 * characters), processing the string in reverse order
 *
 * @param [in] arg1 first pointer to line
 * @param [in] arg2 second pointer to line
 *
 * @return 0, if *arg1 is equal to *arg2, 1 if *arg1 is greater than *arg2, -1
 * otherwise
 */
int line_cmp_reversed(const void* arg1, const void* arg2)
{
    const char *line1 = *((const char**)arg1), *line2 = *((const char**)arg2);
    size_t len1 = strlen(line1), len2 = strlen(line2);
    len1 = (len1 > 0) ? len1 : 1;
    len2 = (len2 > 0) ? len2 : 1;
    const char *line1R = line1 + len1 - 1, *line2R = line2 + len2 - 1;

    while (*line1R && *line2R) {
        if (to_lower(*line1R) == to_lower(*line2R)) {
            --line1R;
            --line2R;
        } else {
            if (is_alpha(*line1R) && is_alpha(*line2R)) {
                break;
            }

            if (!is_alpha(*line1R)) {
                --line1R;
            }

            if (!is_alpha(*line2R)) {
                --line2R;
            }
        }
    }

    while (*line1R && !is_alpha(*line1R)) {
        --line1R;
    }
    while (*line2R && !is_alpha(*line2R)) {
        --line2R;
    }

    if (!*line1R && !*line2R) {
        return 0;
    } else if (*line1R && *line2R) {
        return (to_lower(*line1R) > to_lower(*line2R)) ? 1 : -1;
    } else {
        return (*line1R) ? 1 : -1;
    }
}