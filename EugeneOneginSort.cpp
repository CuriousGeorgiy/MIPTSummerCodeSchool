#ifndef _WIN32
#error This program must be compiled under Windows
#endif

#define _CRT_SECURE_NO_WARNINGS

#define ERROR_OCCURED(msg) do {                                                                                  \
                               fprintf(stderr, "%s at %s(%d):%s\n\n", (msg), __FILE_NAME__, __LINE__, __func__); \
                           } while (0)

#define ERROR_OCCURED_CALLING(func, msg) do {                                                                                            \
                                             fprintf(stderr, "%s %s at %s(%d):%s\n\n", #func, (msg), __FILE_NAME__, __LINE__, __func__); \
                                         } while (0)

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>

/*!
 * Buffer for reading the input file into
 */
static char *BUFFER = NULL;

/*!
 * Constant defining the program's output file name
 */
static const char *OUTPUT_FILE_NAME = "output.txt";

/*!
 * Constant defining the number of mandatory command line arguments
 */
static const size_t N_MANDATORY_ARGS = 1;

/*!
 * Constant defining the number of optional command line arguments
 */
static const size_t N_OPTIONAL_ARGS = 2;

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
 * Enum defining possible line sorting modes
 */
enum sort_mode {
    DIRECT, REVERSED
};

/*!
 * Enum defining possible sort algorithms
 */
enum sort_alg {
    QUICK, TREE
};

int eugene_onegin_sort(const char *input_file_name, enum sort_mode mode,
                       int (*sort_and_output_to_file)(const char *const *, size_t, int (*line_cmp)(const void *, const void *)));

char **get_lines_from_buffer(size_t n_lines);
size_t count_lines_in_buffer();
int read_file_to_buffer(const char *file_name);

int q_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *));
int write_lines_to_file(const char *const *lines, size_t n_lines, const char *open_mode);

int tree_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *));
struct node *generate_bst(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *));
const struct node *insert_node_into_bst(struct node *parent, const char *line, int (*line_cmp)(const void *, const void *));
int write_bst_to_file(FILE *output, const struct node *current);
void delete_bst(struct node *current);

int is_alpha(int c);
int to_lower(int c);
int line_cmp_direct(const void *adr_of_ptr_to_line1, const void *adr_of_ptr_to_line2);
int line_cmp_reversed(const void *adr_of_ptr_to_line1, const void *adr_of_ptr_to_line2);

int main(int argc, const char *argv[])
{
    --argc;
    fprintf(stderr, "\n");

    printf("Eugene Onegin sort\n\n"
           "Poem lines from input file (mandatory first command line argument) will be sorted and written to output file\n"
           "\"output.txt\". The order in which 2 lines are processed during comparison is direct by default or reversed\n"
           "(set by optional command line argument \"-reversed\" or \"--r\"). The sort algorithm is tree sort by\n"
           "default or quick sort (set by optional command line argument \"-quick\" or \"--q\"). Also, the original\n"
           "text will be appended to the output file\n\n");

    if (argc >= N_MANDATORY_ARGS && argc <= N_MANDATORY_ARGS + N_OPTIONAL_ARGS) {
        const char *input_file_name = argv[1];
        enum sort_mode mode = DIRECT;
        enum sort_alg alg = TREE;

        argc -= N_MANDATORY_ARGS;

        size_t matched_args = 0;
        for (size_t i = 2; i < argc; ++i) {
            if ((strcmp(argv[i], "--r") == 0) || (strcmp(argv[i], "-reversed") == 0)) {
                mode = REVERSED;
                ++matched_args;
            } else if ((strcmp(argv[i], "--q") == 0) || (strcmp(argv[i], "-quick") == 0)) {
                alg = QUICK;
                ++matched_args;
            }
        }

        if (matched_args != argc - N_OPTIONAL_ARGS + 1) {
            ERROR_OCCURED("Invalid optional command line arguments (must be \"-reversed\" or \"--r\" and \"-quick\"\n"
                          "or \"--q\") - using correctly matched arguments or defaults");
        }

        int error_code =
            eugene_onegin_sort(input_file_name, mode, (alg == TREE) ? tree_sort_and_output_to_file : q_sort_and_output_to_file);

        switch (error_code) {
        case 0:
            printf("Successfully sorted text from the input file. Check the output file for results\n");

            return EXIT_SUCCESS;
        case 1:
            printf("Input file was empty, output file wasn't created\n");

            return EXIT_SUCCESS;
        default:
            ERROR_OCCURED_CALLING(eugene_onegin_sort, "returned a non-zero value");

            return EXIT_FAILURE;
        }
    } else if (argc > N_MANDATORY_ARGS + N_OPTIONAL_ARGS) {
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
 * Poem lines from input file will be sorted and written to output file. The order in which two lines are processed during
 * comparison is direct (default) or reversed, which is defined by the sort mode. The sort algorithm is defined by sort_and_output_to_file
 *
 * @param [in] input_file_name name of the input file
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
    if (read_file_to_buffer(input_file_name)) {
        ERROR_OCCURED_CALLING(read_file_to_buffer, "returned a non-zero value");

        return -1;
    }

    size_t n_lines = count_lines_in_buffer();
    char **lines = get_lines_from_buffer(n_lines);

    if ((lines == NULL) && (n_lines != 0)) {
        ERROR_OCCURED("read_lines_from_file returned NULL and n_lines != 0");

        free(BUFFER);

        return -1;
    }

    if (n_lines > 0) {
        int error_flag1 = (*sort_and_output_to_file)(lines, n_lines, (mode == DIRECT) ? line_cmp_direct : line_cmp_reversed),
            error_flag2 = write_lines_to_file(lines, n_lines, "a");

        if (error_flag1) {
            ERROR_OCCURED_CALLING(sort_and_output_to_file, "returned a non-zero value");
        }

        if (error_flag2) {
            ERROR_OCCURED_CALLING(write_lines_to_file, "returned a non-zero value");
        }

        free(BUFFER);
        free(lines);

        return error_flag1 && error_flag2;
    } else {
        free(BUFFER);

        return 1;
    }
}

/*!
 * Gets lines from BUFFER by tokenizing it
 *
 * @param [out] n_lines number of lines in BUFFER
 *
 * @return pointer to an array of retrieved lines
 *
 * @note Returns NULL in case of failure
 */
char **get_lines_from_buffer(size_t n_lines)
{
    char **lines = (char **) calloc(n_lines, sizeof(*lines));

    if (lines == NULL) {
        ERROR_OCCURED_CALLING(calloc, "returned NULL");

        return NULL;
    }

    if ((lines[0] = strtok(BUFFER + 1, "\r\n")) == NULL) {
        ERROR_OCCURED_CALLING(strtok, "returned NULL");

        return NULL;
    }

    lines[0][-1] = '\0';

    for (size_t i = 1; i < n_lines; ++i) {
        if ((lines[i] = strtok(NULL, "\r\n")) == NULL) {
            ERROR_OCCURED_CALLING(strtok, "returned NULL");

            return NULL;
        }

        lines[i][-1] = '\0';
    }

    return lines;
}

/*!
 * Reads input file to BUFFER. BUFFER must be freed by caller
 *
 * @param [in] input_file_name name of the input file
 *
 * @return 0 in case of success, 1 if the input file was empty, a different non-zero value otherwise
 *
 * @note Puts '\0' at the beginning of BUFFER for the purpose of dividing strings by '\0'
 */
int read_file_to_buffer(const char *input_file_name)
{
    assert(input_file_name != NULL);

    HANDLE input_file_handle = CreateFileA(input_file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);

    if (input_file_handle == INVALID_HANDLE_VALUE) {
        ERROR_OCCURED_CALLING(CreateFileA, "returned INVALID_HANDLE_VALUE");

        return -1;
    }

    DWORD n_chars = GetFileSize(input_file_handle, NULL);

    if (n_chars > 0) {
        HANDLE input_file_mapping_handle = CreateFileMappingA(input_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);

        if (input_file_mapping_handle == NULL) {
            ERROR_OCCURED_CALLING(CreateFileMappingA, "returned NULL");

            if (CloseHandle(input_file_handle) == 0) {
                ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file handle");
            }

            return -1;
        }

        LPVOID input_file_map_view = MapViewOfFile(input_file_mapping_handle, FILE_MAP_READ, 0, 0, 0);

        if (input_file_map_view == NULL) {
            ERROR_OCCURED_CALLING(MapViewOfFile, "returned NULL");

            if (CloseHandle(input_file_mapping_handle) == 0) {
                ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file mapping handle");
            }

            if (CloseHandle(input_file_handle) == 0) {
                ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file handle");
            }

            return -1;
        }

        if ((BUFFER = (char *) calloc(n_chars + 2, sizeof(*BUFFER))) == NULL) {
            ERROR_OCCURED_CALLING(calloc, "returned NULL");

            return -1;
        }

        CopyMemory(BUFFER + 1, input_file_map_view, n_chars);

        int error_flag1 = 0, error_flag2 = 0, error_flag3 = 0;

        if ((error_flag1 = UnmapViewOfFile(input_file_map_view)) == 0) {
            ERROR_OCCURED_CALLING(UnmapViewOfFile, "returned zero");
        }

        if ((error_flag2 = CloseHandle(input_file_mapping_handle)) == 0) {
            ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file mapping");
        }

        if ((error_flag3 = CloseHandle(input_file_handle)) == 0) {
            ERROR_OCCURED_CALLING(CloseHandle, "returned zero on closing input file");
        }

        return !(error_flag1 && error_flag2 && error_flag3);
    } else {
        return 1;
    }
}

/*!
 * Counts the number of lines in BUFFER
 *
 * @return the number of lines
 */
size_t count_lines_in_buffer()
{
    assert(BUFFER != NULL);

    int pre_pre_c = EOF, pre_c = EOF;
    size_t n_lines = 0;
    const char *reader = BUFFER + 1;

    while (*reader) {
        if ((*reader == '\n') && (pre_c == '\r') && (pre_pre_c != '\n') && (pre_pre_c != EOF)) {
            ++n_lines;
        }

        pre_pre_c = pre_c;
        pre_c = (unsigned char) *(reader++);
    }

    if ((pre_c != '\n') && (pre_c != EOF) && (pre_pre_c != '\r')) {
        ++n_lines;
    }

    return n_lines;
}

/*!
 * Sorts lines using the quick sort algorithm and writes the sorted lines to output file
 *
 * @param [in] lines pointer to array of pointers to line
 * @param [in] n_lines the array size
 * @param [in] line_cmp pointer to the line comparator function
 *
 * @return 0 in case of success, a non-zero value otherwise
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int q_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    char **lines_copy = NULL;

    if ((lines_copy = (char **) calloc(n_lines, sizeof(*lines_copy))) == NULL) {
        ERROR_OCCURED_CALLING(calloc, "returned NULL");

        return -1;
    }

    memcpy(lines_copy, lines, n_lines * sizeof(*lines));

    qsort(lines_copy, n_lines, sizeof(*lines), line_cmp);

    if (write_lines_to_file(lines_copy, n_lines, "w")) {
        ERROR_OCCURED_CALLING(write_lines_to_file, "returned a non-zero value");

        free(lines_copy);

        return -1;
    }

    free(lines_copy);

    return 0;
}

/*!
 * Writes lines to the output file
 *
 * @param [in] lines pointer to an array of lines
 * @param [in] n_lines the array size
 * @param [in] open_mode the mode in which the output file is opened
 *
 * @return 0 in case of success, otherwise a non-zero value
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int write_lines_to_file(const char *const *lines, size_t n_lines, const char *open_mode)
{
    FILE *output = NULL;

    if ((output = fopen(OUTPUT_FILE_NAME, open_mode)) == NULL) {
        ERROR_OCCURED_CALLING(fopen, "returned NULL");

        return -1;
    }

    if (strcmp(open_mode, "a") == 0) {
        fprintf(output, "\nORIGINAL TEXT\n\n");
    }

    for (size_t i = 0; i < n_lines; ++i) {
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
    }

    return 0;
}

/*!
 * Sorts lines using the tree sort algorithm and writes the sorted lines to output file
 *
 * @param [in] lines pointer to array of pointers to line
 * @param [in] n_lines the array size
 * @param [in] line_cmp pointer to line comparator function
 *
 * @note The output file name is OUTPUT_FILE_NAME
 */
int tree_sort_and_output_to_file(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    struct node *root = generate_bst(lines, n_lines, line_cmp);
    if (root == NULL) {
        ERROR_OCCURED_CALLING(generate_bst, "returned NULL");

        return -1;
    }

    FILE *output = fopen(OUTPUT_FILE_NAME, "w");

    if (output == NULL) {
        ERROR_OCCURED_CALLING(fopen, "returned NULL");

        delete_bst(root);

        return -1;
    }

    int error_flag1 = write_bst_to_file(output, root), error_flag2 = fclose(output);

    if (error_flag1) {
        ERROR_OCCURED_CALLING(write_bst_to_file, "returned a non-zero value");
    }

    delete_bst(root);

    if (error_flag2) {
        ERROR_OCCURED_CALLING(fclose, "returned a non-zero value");
    }

    return error_flag1 && error_flag2;
}

/*!
 * Generates a binary search tree (BST) consisting of lines
 *
 * @param [in] lines pointer to array of pointers to line
 * @param [in] n_lines the array size
 * @param [in] line_cmp pointer to the line comparator function
 *
 * @return pointer to the root node
 *
 * @note Returns NULL in case of failure
 */
struct node *generate_bst(const char *const *lines, size_t n_lines, int (*line_cmp)(const void *, const void *))
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    struct node *root = (struct node *) calloc(1, sizeof(*root));
    if (root == NULL) {
        ERROR_OCCURED_CALLING(calloc, "returned NULL");

        return NULL;
    }

    root->line = lines[0];

    root->left = NULL;
    root->right = NULL;

    for (size_t i = 1; i < n_lines; ++i) {
        if (insert_node_into_bst(root, lines[i], line_cmp) == NULL) {
            ERROR_OCCURED_CALLING(insert_node_into_bst, "returned NULL");

            return NULL;
        }
    }

    return root;
}

/*!
 * Recursively inserts node into BST
 *
 * @param [in, out] parent pointer to parent node
 * @param [in] line pointer to line
 * @param [in] line_cmp pointer to the line comparator function
 *
 * @return pointer to the inserted node
 *
 * @note Returns NULL in case of failure
 */
const struct node *insert_node_into_bst(struct node *parent, const char *line, int (*line_cmp)(const void *, const void *))
{
    assert(parent != NULL);
    assert(line != NULL);
    assert(line_cmp != NULL);

    if ((*line_cmp)(&parent->line, &line) >= 0) {
        if (parent->left == NULL) {
            if ((parent->left = (struct node *) calloc(1, sizeof(*parent))) == NULL) {
                ERROR_OCCURED_CALLING(calloc, "returned NULL");

                return NULL;
            }

            parent->left->line = line;
            parent->left->left = NULL;
            parent->left->right = NULL;

            return parent->left;
        } else {
            return insert_node_into_bst(parent->left, line, line_cmp);
        }
    } else {
        if (parent->right == NULL) {
            if ((parent->right = (struct node *) calloc(1, sizeof(*parent))) == NULL) {
                ERROR_OCCURED_CALLING(calloc, "returned NULL");

                return NULL;
            }

            parent->right->line = line;
            parent->right->left = NULL;
            parent->right->right = NULL;

            return parent->right;
        } else {
            return insert_node_into_bst(parent->right, line, line_cmp);
        }
    }
}

/*!
 * Recursively writes BST to output file
 *
 * @param [in, out] output pointer to output file
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
 * Recursively deletes BST
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
 *  Checks if ch is an alpha
 *
 * @param [in] ch character code
 *
 * @return 1 if ch is an alpha, 0 otherwise
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
 * Compares two lines, discarding non-alpha characters, processing the lines in direct order
 *
 * @param [in] adr_of_ptr_to_line1 first address of pointer to line
 * @param [in] adr_of_ptr_to_line2 second address of pointer to line
 *
 * @return 0, if line1 is equal to line2, 1 if line1 is greater than line2, -1 otherwise
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
 * Compares two lines, discarding non-alpha characters, processing the lines in reversed order
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