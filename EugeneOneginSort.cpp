#ifndef _WIN32
#error This program must be compiled under Windows
#endif

#define _CRT_SECURE_NO_WARNINGS

#define ERROR_OCCURED(msg) do {                                                                       \
                               fprintf(stderr, "%s at %s(%d):%s\n\n", (msg), __FILE_NAME__, __LINE__, \
                               __func__);                                                             \
                           } while (0)

#define ERROR_OCCURED_CALLING(func, msg) do {                                                                       \
                                             fprintf(stderr, "%s %s at %s(%d):%s\n\n", (msg), #func, __FILE_NAME__, \
                                             __LINE__, __func__);                                                   \
                                         } while (0)

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>

/*!
 * Global char buffer for reading text file into
 */
static char *BUFFER = NULL;

/*!
 * Constant for defining the output file name of the program
 */
static const char *OUTPUT_FILE_NAME = "output.txt";

/*!
 * Data structure defining the node of a binary search tree. Contains a pointer
 * to line of chars and two pointers to child nodes
 */
struct node {
    const char *line;
    struct node *left;
    struct node *right;
};

/*!
 * Enum describing possible line sorting modes
 */
enum sort_mode {
    DIRECT, REVERSED
};

/*!
 * Enum describing possible sort algorithms
 */
enum sort_alg {
    QUICK, TREE
};

int eugene_onegin_sort(const char *input_file_name, enum sort_mode mode,
                       int (*sort_and_output_to_file)(const char *const *, size_t, int (*line_cmp)(const void *, const void *)));

char **read_lines_from_file(const char *file_name, size_t *n_lines);
size_t count_lines_in_buffer();
int read_file_to_buffer(const char *file_name);

int q_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *));
int write_lines_to_file(const char *const *lines, size_t n_lines);

int append_lines_to_file(const char *const *lines, size_t n_lines);

int tree_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *));
struct node *generate_bst(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *));
struct node *insert_node_into_bst(struct node *parent, const char *line, int (*line_cmp)(const void *, const void *));
int write_bst_to_file(FILE *output, const struct node *current);
void delete_bst(struct node *current);

int is_alpha(int c);
int to_lower(int c);
int line_cmp_direct(const void *adr_of_ptr_to_line1, const void *adr_of_ptr_to_line2);
int line_cmp_reversed(const void *adr_of_ptr_to_line1, const void *adr_of_ptr_to_line2);

int main(int argc, const char *argv[])
{
    fprintf(stderr, "\n");

    printf("Eugene Onegin sort\n\n"
           "Poem lines from input file (mandatory first command line argument) will be sorted and written to output file\n"
           "\"output.txt\". The order in which 2 lines are processed during comparison is direct by default or reversed\n"
           "(set by optional command line argument \"-reversed\" or \"--r\"). The sort algorithm is tree sort by\n"
           "default or quick sort (set by optional command line argument \"-quick\" or \"--q\"). Also, the original\n"
           "text will be appended to the output file\n\n");

    if (argc >= 2 && argc <= 4) {
        const char *input_file_name = argv[1];
        enum sort_mode mode = DIRECT;
        enum sort_alg alg = TREE;

        size_t i = 1, matched_args = 0;

        for (i = 2; i < argc; ++i) {
            if ((strcmp(argv[i], "--r") == 0) || (strcmp(argv[i], "-reversed") == 0)) {
                mode = REVERSED;
                ++matched_args;
            } else if ((strcmp(argv[i], "--q") == 0) || (strcmp(argv[i], "-quick") == 0)) {
                alg = QUICK;
                ++matched_args;
            }
        }

        if (matched_args != argc - 2) {
            ERROR_OCCURED("Invalid optional command line arguments (must be \"-reversed\" or \"--r\" and \"-quick\"\n"
                          "or \"--q\") - using correctly matched arguments or defaults");
        }

        int return_value = 0;

        if (!(return_value =
                  eugene_onegin_sort(input_file_name, mode, (alg == TREE) ? tree_sort_and_output_to_file : q_sort_and_output_to_file))) {
            printf("Successfully sorted text from the input file. Check the output file for results\n");

            return EXIT_SUCCESS;
        } else if (return_value == 1) {
            printf("Input file was empty, output file wasn't created");

            return EXIT_SUCCESS;
        } else {
            ERROR_OCCURED_CALLING(eugene_onegin_sort, "returned a non-zero value");

            return EXIT_FAILURE;
        }
    } else if (argc > 4) {
        ERROR_OCCURED("Invalid command line arguments - first must be the input file name,\n the second one is\n"
                      "optional and must be equal to \"-reversed\" or \"--r\" (sets\n the order in which 2 lines will\n"
                      "be processed during comparison to reversed)");

        return EXIT_FAILURE;
    } else {
        ERROR_OCCURED("Run the program with command line arguments");

        return EXIT_FAILURE;
    }
}

/*!
 * Poem lines from input file will be sorted and written to the output file. The
 * order
 * in which 2 lines are processed during comparison is direct (default) or reversed, which is
 * defined by the sort mode
 *
 * @param [in] input_file_name pointer to string containing the input file name
 * @param [in] mode enum constant which sets the sort mode ('d' or 'r')
 * @param [in] sort_and_output_to_file pointer to function which does the sorting and output
 *
 * @return 0 in case of success, 1 in case the input file was empty, a different non-zero value otherwise
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int eugene_onegin_sort(const char *input_file_name, enum sort_mode mode,
                       int (*sort_and_output_to_file)(const char *const *, size_t, int (*line_cmp)(const void *, const void *)))
{
    size_t n_lines = 0;
    char **lines = NULL;

    if (((lines = read_lines_from_file(input_file_name, &n_lines)) != NULL) || (n_lines == 0)) {
        if (n_lines > 0) {
            int error_flag1 = 0, error_flag2 = 0;

            if ((error_flag1 = (*sort_and_output_to_file)(lines, n_lines, (mode == DIRECT) ? line_cmp_direct : line_cmp_reversed)) != 0) {
                ERROR_OCCURED_CALLING(sort_and_output_to_file, "returned a non-zero value");
            }

            if ((error_flag2 = append_lines_to_file(lines, n_lines)) != 0) {
                ERROR_OCCURED_CALLING(append_lines_to_file, "returned a non-zero value");
            }

            free(BUFFER);
            free(lines);

            return error_flag1 && error_flag2;
        } else {
            free(BUFFER);

            return 1;
        }
    } else {
        ERROR_OCCURED("read_lines_from_file returned NULL and n_lines != 0");

        free(BUFFER);

        return -1;
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
char **read_lines_from_file(const char *file_name, size_t *n_lines)
{
    assert(file_name != NULL);
    assert(n_lines != NULL);

    if (!read_file_to_buffer(file_name) && ((*n_lines = count_lines_in_buffer()) > 0)) {
        char **lines = NULL;

        if ((lines = (char **) calloc(*n_lines, sizeof(*lines))) != NULL) {
            lines[0] = strtok(BUFFER + 1, "\n");
            lines[0][-1] = '\0';

            size_t i = 1;
            for (i = 1; i < *n_lines; ++i) {
                lines[i] = strtok(NULL, "\n");
                lines[i][-1] = '\0';
            }

            return lines;
        } else {
            ERROR_OCCURED_CALLING(calloc, "returned NULL");
            return NULL;
        }
    } else {
        if (*n_lines != 0) {
            ERROR_OCCURED_CALLING(read_file_to_buffer, "returned non-zero value and n_lines != 0");
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
 * @return 0 in case of success, a non-zero value otherwise
 *
 * @note Returns -1 in case of failure, 1 if the input file was empty. Puts '\0'
 * in the beginning of the buffer (for the purpose of dividing strings by '\0')
 */
int read_file_to_buffer(const char *file_name)
{
    assert(file_name != NULL);

    HANDLE input_file_handle = NULL;

    if ((input_file_handle = CreateFileA(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL)) != INVALID_HANDLE_VALUE) {
        DWORD n_chars = GetFileSize(input_file_handle, NULL);

        if (n_chars > 0) {
            HANDLE input_file_mapping_handle = NULL;

            if ((input_file_mapping_handle = CreateFileMappingA(input_file_handle, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL) {
                LPVOID input_file_map_view = NULL;

                if ((input_file_map_view = MapViewOfFile(input_file_mapping_handle, FILE_MAP_READ, 0, 0, 0)) != NULL) {
                    if ((BUFFER = (char *) calloc(n_chars + 2ul, sizeof(*BUFFER))) != NULL) {
                        CopyMemory(BUFFER + 1, input_file_map_view, n_chars);

                        int error_flag1 = 0, error_flag2 = 0, error_flag3 = 0;

                        if ((error_flag1 = UnmapViewOfFile(input_file_map_view)) == 0) {
                            ERROR_OCCURED_CALLING(UnmapViewOfFile, "returned zero");
                        }

                        if ((error_flag2 = CloseHandle(input_file_mapping_handle)) == 0) {
                            ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file mapping handle");
                        }

                        if ((error_flag3 = CloseHandle(input_file_handle)) == 0) {
                            ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file handle");
                        }

                        return !(error_flag1 && error_flag2 && error_flag3);
                    } else {
                        ERROR_OCCURED_CALLING(calloc, "returned NULL");

                        return -1;
                    }
                } else {
                    ERROR_OCCURED_CALLING(MapViewOfFile, "returned NULL");

                    if (CloseHandle(input_file_mapping_handle) == 0) {
                        ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file mapping handle");
                    }

                    if (CloseHandle(input_file_handle) == 0) {
                        ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file mapping handle");
                    }

                    return -1;
                }
            } else {
                ERROR_OCCURED_CALLING(CreateFileMappingA, "returned NULL");

                if (CloseHandle(input_file_handle) == 0) {
                    ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file mapping handle\n");
                }

                return -1;
            }
        } else {
            return 1;
        }
    } else {
        ERROR_OCCURED_CALLING(CreateFileA, "returned INVALID_HANDLE_VALUE");

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
size_t count_lines_in_buffer()
{
    assert(BUFFER != NULL);

    int pre_c = EOF;
    size_t n_lines = 0;
    const char *reader = BUFFER + 1;

    while (*reader) {
        if ((*reader == '\n') && (pre_c != '\n') && (pre_c != EOF)) {
            ++n_lines;
        }

        pre_c = (unsigned char) *(reader++);
    }

    if ((pre_c != EOF) && (pre_c != '\n')) {
        ++n_lines;
    }

    return n_lines;
}

/*!
 * Sorts lines using the quick sort algorithm and writes the sorted lines to the output file
 *
 * @param [in] lines pointer to an array of lines
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to line comparator function
 *
 * @return 0 in case of success, a non-zero value otherwise
 * @note The output file name is OUTPUT_FILE_NAME
 */
int q_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    char **lines_copy = NULL;

    if ((lines_copy = (char **) calloc(n_lines, sizeof(*lines_copy))) != NULL) {
        memcpy(lines_copy, lines, n_lines * sizeof(*lines_copy));

        qsort(lines_copy, n_lines, sizeof(*lines), line_cmp);

        if (write_lines_to_file(lines_copy, n_lines)) {
            ERROR_OCCURED_CALLING(write_lines_to_file, "returned a non-zero value");

            free(lines_copy);

            return -1;
        } else {
            free(lines_copy);

            return 0;
        }
    } else {
        ERROR_OCCURED_CALLING(calloc, "returned NULL");

        return -1;
    }
}

/*!
 * Writes lines to the output file
 *
 * @param lines pointer to an array of lines
 * @param n_lines size of array
 *
 * @return 0 in case of success, otherwise a non-zero value
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int write_lines_to_file(const char *const *lines, size_t n_lines)
{
    FILE *output = NULL;

    if ((output = fopen(OUTPUT_FILE_NAME, "w")) != NULL) {
        size_t i = 0;
        for (i = 0; i < n_lines; ++i) {
            const char *line_ptr = lines[i];

            while (isspace(*line_ptr)) {
                ++line_ptr;
            }

            if (line_ptr[0] && islower(line_ptr[1])) {
                if (fprintf(output, "%s\n", line_ptr) < 0) {
                    ERROR_OCCURED_CALLING(fprintf, "returned negative value");

                    return -1;
                }
            }
        }

        if (fclose(output)) {
            ERROR_OCCURED_CALLING(fclose, "returned a non-zero value");

            return -1;
        } else {
            return 0;
        }
    } else {
        ERROR_OCCURED_CALLING(fopen, "returned NULL");

        return -1;
    }
}

/*!
 * Append lines to the output file
 *
 * @param lines pointer to an array of lines
 * @param n_lines size of array
 *
 * @return 0 in case of success, otherwise a non-zero value
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int append_lines_to_file(const char *const *lines, size_t n_lines)
{
    FILE *output = NULL;

    if ((output = fopen(OUTPUT_FILE_NAME, "a")) != NULL) {
        fprintf(output, "\nORIGINAL TEXT\n\n");

        size_t i = 0;
        for (i = 0; i < n_lines; ++i) {
            const char *line_ptr = lines[i];

            while (isspace(*line_ptr)) {
                ++line_ptr;
            }

            if (line_ptr[0] && islower(line_ptr[1])) {
                if (fprintf(output, "%s\n", line_ptr) < 0) {
                    ERROR_OCCURED_CALLING(fprintf, "returned a negative value");

                    return -1;
                }
            }
        }

        if (fclose(output)) {
            ERROR_OCCURED_CALLING(fclose, "returned a non-zero value");

            return -1;
        } else {
            return 0;
        }
    } else {
        ERROR_OCCURED_CALLING(fopen, "returned NULL");

        return -1;
    }
}

/*!
 * Sorts lines using the tree sort algorithm and writes the sorted lines to output file
 *
 * @param [in] lines pointer to an array of lines
 * @param [in] n_lines array size
 * @param [in] line_cmp pointer to line comparator function
 * @param [in] file_name name of file to which the result of sorting is written
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int tree_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    struct node *root = NULL;
    if ((root = generate_bst(lines, n_lines, line_cmp)) != NULL) {
        FILE *output = NULL;

        if ((output = fopen(OUTPUT_FILE_NAME, "w"))) {
            int error_flag1 = 0, error_flag2 = 0;

            if ((error_flag1 = write_bst_to_file(output, root))) {
                ERROR_OCCURED_CALLING(write_bst_to_file, "returned a non-zero value");
            }

            delete_bst(root);

            if ((error_flag2 = fclose(output))) {
                ERROR_OCCURED_CALLING(fclose, "returned a non-zero value");
            }

            return error_flag1 && error_flag2;
        } else {
            ERROR_OCCURED_CALLING(fopen, "returned NULL");

            delete_bst(root);

            return -1;
        }
    } else {
        ERROR_OCCURED_CALLING(generate_bst, "returned NULL");

        return -1;
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
struct node *generate_bst(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    struct node *root = NULL;
    if ((root = (struct node *) calloc(1, sizeof(*root))) != NULL) {
        root->line = lines[0];

        root->left = NULL;
        root->right = NULL;

        size_t i = 1;
        for (i = 1; i < n_lines; ++i) {
            if (insert_node_into_bst(root, lines[i], line_cmp) == NULL) {
                ERROR_OCCURED_CALLING(insert_node_into_bst, "returned NULL");

                return NULL;
            }
        }

        return root;
    } else {
        ERROR_OCCURED_CALLING(calloc, "returned NULL");

        return NULL;
    }
}

/*!
 * Recursively inserts node into BST
 *
 * @param [in, out] parent pointer to parent node
 * @param [in] line pointer to line of chars
 * @param [in] line_cmp to pointer to line comparator function
 *
 * @return Pointer to inserted node
 *
 * @note Returns NULL in case of failure
 */
struct node *insert_node_into_bst(struct node *parent, const char *line, int (*line_cmp)(const void *, const void *))
{
    assert(parent != NULL);
    assert(line != NULL);
    assert(line_cmp != NULL);

    if ((*line_cmp)(&parent->line, &line) >= 0) {
        if (parent->left == NULL) {
            if ((parent->left = (struct node *) calloc(1, sizeof(*parent))) != NULL) {
                parent->left->line = line;
                parent->left->left = NULL;
                parent->left->right = NULL;

                return parent->left;
            } else {
                ERROR_OCCURED_CALLING(calloc, "returned NULL");

                return NULL;
            }
        } else {
            return insert_node_into_bst(parent->left, line, line_cmp);
        }
    } else {
        if (parent->right == NULL) {
            if ((parent->right = (struct node *) calloc(1, sizeof(*parent))) != NULL) {
                parent->right->line = line;
                parent->right->left = NULL;
                parent->right->right = NULL;

                return parent->right;
            } else {
                ERROR_OCCURED_CALLING(calloc, "returned NULL");

                return NULL;
            }
        } else {
            return insert_node_into_bst(parent->right, line, line_cmp);
        }
    }
}

/*!
 * Writes BST to output file
 *
 * @param [in] output pointer to output file
 * @param [in] current pointer to current node
 *
 * @return 0 in case of success, a non-zero value otherwise
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int write_bst_to_file(FILE *output, const struct node *current)
{
    assert(output != NULL);
    assert(current != NULL);

    if (current->left != NULL) {
        if (write_bst_to_file(output, current->left)) {
            ERROR_OCCURED_CALLING(write_bst_to_file, "returned a non-zero value");

            return -1;
        }
    }

    const char *line_ptr = current->line;

    while (isspace(*line_ptr)) {
        ++line_ptr;
    }

    if (line_ptr[0] && islower(line_ptr[1])) {
        if (fprintf(output, "%s\n", line_ptr) < 0) {
            ERROR_OCCURED_CALLING(fprintf, "returned a negative value");

            return -1;
        }
    }

    if (current->right != NULL) {
        if (write_bst_to_file(output, current->right)) {
            ERROR_OCCURED_CALLING(write_bst_to_file, "returned a non-zero value");

            return -1;
        }
    }

    return 0;
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
 *  Checks if c is an alpha
 *
 * @param [in] c
 *
 * @return 1 if c is an alpha, 0 otherwise
 */
int is_alpha(int ch)
{
    return (('A' <= (ch)) && ((ch) <= 'Z')) || (('a' <= (ch)) && ((ch) <= 'z'));
}

/*!
 * Converts a ch to lower if it's an alpha
 *
 * @param [in] ch character code
 *
 * @return Lowercase version of ch or ch if it's not an alpha
 */
int to_lower(int ch)
{
    if (is_alpha(ch)) {
        return tolower(ch);
    } else {
        return ch;
    }
}

/*!
 * Direct line comparator function. Compares two line, discarding non-alpha
 * characters (in terms of this their length is equal to the number of alpha
 * characters), processing the string in direct order
 *
 * @param [in] adr_of_ptr_to_line1 first address of pointer to pointer to line
 * @param [in] adr_of_ptr_to_line1 second address of pointer to pointer to line
 *
 * @return 0, if line1 is equal to line2, 1 if line1 is greater than line22, -1 otherwise
 * otherwise
 */
int line_cmp_direct(const void *adr_of_ptr_to_line1, const void *adr_of_ptr_to_line2)
{
    assert(adr_of_ptr_to_line1 != NULL);
    assert(adr_of_ptr_to_line2 != NULL);

    const char *line1 = *((const char **) adr_of_ptr_to_line1), *line2 = *((const char **) adr_of_ptr_to_line2);

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
 * @param [in] adr_of_ptr_to_line1 first address of pointer to line
 * @param [in] adr_of_ptr_to_line2 second address of pointer to line
 *
 * @return 0, if line1 is equal to line2, 1 if line1 is greater than line2, -1 otherwise
 * otherwise
 */
int line_cmp_reversed(const void *adr_of_ptr_to_line1, const void *adr_of_ptr_to_line2)
{
    const char *line1 = *((const char **) adr_of_ptr_to_line1), *line2 = *((const char **) adr_of_ptr_to_line2);
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