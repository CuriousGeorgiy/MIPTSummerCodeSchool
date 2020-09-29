#ifndef _WIN32
#error This program must be compiled under Windows
#endif

#define _CRT_SECURE_NO_WARNINGS

#ifdef __clang__
#define ERROR_OCCURRED_CALLING(func, msg) fprintf(stderr, "%s %s at %s(%d):%s\n\n", #func, (msg), __FILE_NAME__, __LINE__, __func__)
#else
#define ERROR_OCCURRED_CALLING(func, msg) fprintf(stderr, "%s %s at %s(%d):%s\n\n", #func, (msg), __FILE__, __LINE__, __func__)
#endif

#define FREE(ptr) do { \
                      free(ptr); \
                      (ptr) = NULL; \
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
static const size_t N_OPTIONAL_ARGS = 3;

/*!
 * Data structure defining text lines. Contains a pointer to char and length of the line
 */
struct line_t {
    char *str;

    size_t len;
};

/*!
 * Data structure defining the node of a binary search tree. Contains a pointer
 * to line of chars and two pointers to child nodes
 */
struct node_t {
    node_t *left;
    node_t *right;

    line_t line;
};

/*!
 * Enum defining possible line sort modes
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


typedef int comparator_func_t(const void *, const void *);
typedef int sort_and_output_to_file_wrapper_func_t(const line_t *, size_t, comparator_func_t *);

int eugene_onegin_sort(const char *input_file_name, sort_mode mode, sort_and_output_to_file_wrapper_func_t *sort_and_output_to_file);

line_t *get_lines_from_buffer(size_t n_lines);
size_t count_lines_in_buffer();
int read_file_to_buffer(const char *file_name);

int q_sort_and_output_to_file(const line_t *lines, size_t n_lines, comparator_func_t *line_cmp);
int write_lines_to_file(const line_t *lines, size_t n_lines, const char *open_mode);

int tree_sort_and_output_to_file(const line_t *lines, size_t n_lines, comparator_func_t *line_cmp);
node_t *generate_bst(const line_t *lines, size_t n_lines, comparator_func_t *line_cmp);
const node_t *insert_node_into_bst(node_t *parent, line_t line, comparator_func_t *line_cmp);
int write_bst_to_file(FILE *output, const node_t *current);
void delete_bst(node_t *current);

int is_alpha(int c);
int to_lower(int c);
int line_cmp_direct(const void *line1, const void *line2);
int line_cmp_reversed(const void *line1, const void *line2);

int main(int argc, const char *argv[])
{
    --argc;
    fprintf(stderr, "\n");

    printf("Eugene Onegin sort\n\n");

    if (argc < N_MANDATORY_ARGS) {
        printf("Please rerun the program and specify the input file as the first command line argument\n");
        return EXIT_FAILURE;
    }

    const char *input_file_name = argv[1];
    argc -= N_MANDATORY_ARGS;

    sort_mode mode = DIRECT;

    sort_alg alg = TREE;

    bool verbose = false;

    size_t matched_args = 0;

    for (size_t i = 2; i < argc; ++i) {
        if ((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--reversed") == 0)) {
            mode = REVERSED;
            ++matched_args;
        }

        if ((strcmp(argv[i], "-q") == 0) || (strcmp(argv[i], "--quick") == 0)) {
            alg = QUICK;
            ++matched_args;
        }

        if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0)) {
            verbose = true;
            ++matched_args;
        }
    }

    if (matched_args != argc - N_OPTIONAL_ARGS + 1) {
        printf("Invalid optional command line arguments (must be \"-reversed\" or \"--r\" and \"-quick\"\n"
               "or \"--q\") - using correctly matched arguments or defaults");
    }

    if (verbose) {
        printf("Poem lines from input file (mandatory first command line argument) will be sorted and written to output file\n"
               "\"output.txt\". The order in which 2 lines are processed during comparison is direct by default or reversed\n"
               "(set by optional command line argument \"-reversed\" or \"--r\"). The sort algorithm is tree sort by\n"
               "default or quick sort (set by optional command line argument \"-quick\" or \"--q\"). Also, the original\n"
               "text will be appended to the output file\n\n");
    }

    int error_code =
            eugene_onegin_sort(input_file_name, mode, (alg == TREE) ? tree_sort_and_output_to_file : q_sort_and_output_to_file);

    switch (error_code) {
    case 0: {
        printf("Successfully sorted text from the input file. Check the output file for results\n");
        return EXIT_SUCCESS;
    }

    case 1: {
        printf("Input file was empty, output file wasn't created\n");
        return EXIT_SUCCESS;
    }

    default:
        ERROR_OCCURRED_CALLING(eugene_onegin_sort, "returned a non-zero value");
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
int eugene_onegin_sort(const char *input_file_name, sort_mode mode, sort_and_output_to_file_wrapper_func_t *sort_and_output_to_file)
{
    if (read_file_to_buffer(input_file_name)) {
        ERROR_OCCURRED_CALLING(read_file_to_buffer, "returned a non-zero value");

        return -1;
    }

    assert(BUFFER);

    size_t n_lines = count_lines_in_buffer();
    line_t *lines  = get_lines_from_buffer(n_lines);

    if ((lines == NULL) && (n_lines != 0)) {
        ERROR_OCCURRED_CALLING(read_lines_from_file, "returned NULL and n_lines != 0");

        FREE(BUFFER);

        return -1;
    }

    if (n_lines == 0) {
        FREE(BUFFER);

        return 1;
    }

    int sort_and_output_to_file_error_flag =
            (*sort_and_output_to_file)(lines, n_lines, (mode == DIRECT) ? line_cmp_direct : line_cmp_reversed),
        write_lines_to_file_error_flag     = write_lines_to_file(lines, n_lines, "a");

    if (sort_and_output_to_file_error_flag) {
        ERROR_OCCURRED_CALLING(sort_and_output_to_file, "returned a non-zero value");
    }

    if (write_lines_to_file_error_flag) {
        ERROR_OCCURRED_CALLING(write_lines_to_file, "returned a non-zero value");
    }

    FREE(BUFFER);
    FREE(lines);

    return sort_and_output_to_file_error_flag && write_lines_to_file_error_flag;
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
line_t *get_lines_from_buffer(size_t n_lines)
{
    line_t *lines = (line_t *) calloc(n_lines + 1, sizeof(*lines));

    if (lines == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");

        return NULL;
    }

    if ((lines[0].str = strtok(BUFFER + 1, "\r\n")) == NULL) {
        ERROR_OCCURRED_CALLING(strtok, "returned NULL");

        return NULL;
    }
    lines[0].str[-1] = '\0';
    lines[0].len = strlen(lines[0].str);

    for (size_t i = 1; i < n_lines; ++i) {
        if ((lines[i].str = strtok(NULL, "\r\n")) == NULL) {
            ERROR_OCCURRED_CALLING(strtok, "returned NULL");

            return NULL;
        }

        lines[i].str[-1] = '\0';
        lines[i].len = strlen(lines[i].str);
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
        ERROR_OCCURRED_CALLING(CreateFileA, "returned INVALID_HANDLE_VALUE");

        return -1;
    }

    DWORD input_file_size = GetFileSize(input_file_handle, NULL);

    if (input_file_size == 0) {
        return 1;
    }
    HANDLE input_file_mapping_handle = CreateFileMappingA(input_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);

    if (input_file_mapping_handle == NULL) {
        ERROR_OCCURRED_CALLING(CreateFileMappingA, "returned NULL");

        if (CloseHandle(input_file_handle) == 0) {
            ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file handle");
        }

        return -1;
    }

    LPVOID input_file_map_view = MapViewOfFile(input_file_mapping_handle, FILE_MAP_READ, 0, 0, 0);

    if (input_file_map_view == NULL) {
        ERROR_OCCURRED_CALLING(MapViewOfFile, "returned NULL");

        if (CloseHandle(input_file_mapping_handle) == 0) {
            ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file mapping handle");
        }

        if (CloseHandle(input_file_handle) == 0) {
            ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file handle");
        }

        return -1;
    }

    if ((BUFFER = (char *) calloc(input_file_size + 2, sizeof(*BUFFER))) == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");

        return -1;
    }

    memcpy(BUFFER + 1, input_file_map_view, input_file_size);

    int error_flag1 = 0, error_flag2 = 0, error_flag3 = 0;

    if ((error_flag1 = UnmapViewOfFile(input_file_map_view)) == 0) {
        ERROR_OCCURRED_CALLING(UnmapViewOfFile, "returned zero");
    }

    if ((error_flag2 = CloseHandle(input_file_mapping_handle)) == 0) {
        ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file mapping");
    }

    if ((error_flag3 = CloseHandle(input_file_handle)) == 0) {
        ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file");
    }

    return !(error_flag1 && error_flag2 && error_flag3);
}

/*!
 * Counts the number of lines in BUFFER
 *
 * @return the number of lines
 */
size_t count_lines_in_buffer()
{
    assert(BUFFER != NULL);

    int pre_pre_c = EOF,
        pre_c     = EOF;

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
int q_sort_and_output_to_file(const line_t *lines, size_t n_lines, comparator_func_t *line_cmp)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    line_t *lines_copy = NULL;

    if ((lines_copy = (line_t *) calloc(n_lines, sizeof(*lines_copy))) == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");

        return -1;
    }

    memcpy(lines_copy, lines, n_lines * sizeof(*lines));

    qsort(lines_copy, n_lines, sizeof(*lines), line_cmp);

    if (write_lines_to_file(lines_copy, n_lines, "w")) {
        ERROR_OCCURRED_CALLING(write_lines_to_file, "returned a non-zero value");

        FREE(lines_copy);

        return -1;
    }

    FREE(lines_copy);

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
int write_lines_to_file(const line_t *lines, size_t n_lines, const char *open_mode)
{
    FILE *output = NULL;

    if ((output = fopen(OUTPUT_FILE_NAME, open_mode)) == NULL) {
        ERROR_OCCURRED_CALLING(fopen, "returned NULL");

        return -1;
    }

    if (strcmp(open_mode, "a") == 0) {
        fprintf(output, "\nORIGINAL TEXT\n\n");
    }

    for (size_t i = 0; i < n_lines; ++i) {
        const char *str = lines[i].str;

        while (isspace(*str)) {
            ++str;
        }

        if (str[0] && islower(str[1])) {
            if (fprintf(output, "%s\n", str) < 0) {
                ERROR_OCCURRED_CALLING(fprintf, "returned negative value");

                if (fclose(output)) {
                    ERROR_OCCURRED_CALLING(fclose, "returned a non-zero value");
                }

                return -1;
            }
        }
    }

    if (fclose(output)) {
        ERROR_OCCURRED_CALLING(fclose, "returned a non-zero value");

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
int tree_sort_and_output_to_file(const line_t *lines, size_t n_lines, comparator_func_t *line_cmp)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    node_t *root = generate_bst(lines, n_lines, line_cmp);

    if (root == NULL) {
        ERROR_OCCURRED_CALLING(generate_bst, "returned NULL");

        return -1;
    }

    FILE *output = fopen(OUTPUT_FILE_NAME, "w");

    if (output == NULL) {
        ERROR_OCCURRED_CALLING(fopen, "returned NULL");

        delete_bst(root);

        return -1;
    }

    int write_bst_to_file_error_flag = write_bst_to_file(output, root),
        fclose_error_flag            = fclose(output);

    if (write_bst_to_file_error_flag) {
        ERROR_OCCURRED_CALLING(write_bst_to_file, "returned a non-zero value");
    }

    delete_bst(root);

    if (fclose_error_flag) {
        ERROR_OCCURRED_CALLING(fclose, "returned a non-zero value");
    }

    return write_bst_to_file_error_flag && fclose_error_flag;
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
node_t *generate_bst(const line_t *lines, size_t n_lines, comparator_func_t *line_cmp)
{
    assert(lines != NULL);
    assert(line_cmp != NULL);

    assert(n_lines > 0);

    node_t *root = (node_t *) calloc(1, sizeof(*root));

    if (root == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");

        return NULL;
    }

    root->line = lines[0];

    root->left  = NULL;
    root->right = NULL;

    for (size_t i = 1; i < n_lines; ++i) {
        if (insert_node_into_bst(root, lines[i], line_cmp) == NULL) {
            ERROR_OCCURRED_CALLING(insert_node_into_bst, "returned NULL");

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
const node_t *insert_node_into_bst(node_t *parent, line_t line, comparator_func_t *line_cmp)
{
    assert(parent != NULL);
    assert(line_cmp != NULL);

    if ((*line_cmp)(&parent->line, &line) >= 0) {
        if (parent->left == NULL) {
            if ((parent->left = (node_t *) calloc(1, sizeof(*parent))) == NULL) {
                ERROR_OCCURRED_CALLING(calloc, "returned NULL");

                return NULL;
            }

            parent->left->line = line;

            parent->left->left  = NULL;
            parent->left->right = NULL;

            return parent->left;
        } else {
            return insert_node_into_bst(parent->left, line, line_cmp);
        }
    }

    if (parent->right == NULL) {
        if ((parent->right = (node_t *) calloc(1, sizeof(*parent))) == NULL) {
            ERROR_OCCURRED_CALLING(calloc, "returned NULL");

            return NULL;
        }

        parent->right->line  = line;
        parent->right->left  = NULL;
        parent->right->right = NULL;

        return parent->right;
    } else {
        return insert_node_into_bst(parent->right, line, line_cmp);
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
int write_bst_to_file(FILE *output, const node_t *current)
{
    assert(output != NULL);
    assert(current != NULL);

    if (current->left != NULL) {
        if (write_bst_to_file(output, current->left)) {
            ERROR_OCCURRED_CALLING(write_bst_to_file, "returned a non-zero value");

            return -1;
        }
    }

    const char *str = current->line.str;

    while (isspace(*str)) {
        ++str;
    }

    if (str[0] && islower(str[1])) {
        if (fprintf(output, "%s\n", str) < 0) {
            ERROR_OCCURRED_CALLING(fprintf, "returned a negative value");

            return -1;
        }
    }

    if (current->right != NULL) {
        if (write_bst_to_file(output, current->right)) {
            ERROR_OCCURRED_CALLING(write_bst_to_file, "returned a non-zero value");

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
void delete_bst(node_t *current)
{
    assert(current != NULL);

    if (current->left != NULL) {
        delete_bst(current->left);
    }

    if (current->right != NULL) {
        delete_bst(current->right);
    }

    FREE(current);
}

/*!
 * Wrapper over standard library isalpha function. Preliminarily converts parameter to unsigned char type
 *
 * @param [in] ch character code
 *
 * @return 1 if ch is an alpha, 0 otherwise
 */
int is_alpha(int ch)
{
    return isalpha((unsigned char) ch);
}

/*!
 * Wrapper over standard library isalpha function. Preliminarily converts parameter to unsigned char type
 *
 * @param [in] ch character code
 *
 * @return Lowercase version of ch or ch, if it's not an alpha
 */
int to_lower(int ch)
{
    return tolower((unsigned char) ch);
}

/*!
 * Compares two lines, discarding non-alpha characters, processing the lines in direct order
 *
 * @param [in] line1 first pointer to line
 * @param [in] line2 second pointer to line
 *
 * @return 0, if line1 is equal to line2, 1 if line1 is greater than line2, -1 otherwise
 * otherwise
 */
int line_cmp_direct(const void *line1, const void *line2)
{
    assert(line1 != NULL);
    assert(line2 != NULL);

    const char *str1 = ((const line_t *) line1)->str,
               *str2 = ((const line_t *) line2)->str;

    while (*str1 && *str2) {
        if (to_lower(*str1) == to_lower(*str2)) {
            ++str1;
            ++str2;
        } else {
            if (is_alpha(*str1) && is_alpha(*str2)) {
                break;
            }

            if (!is_alpha(*str1)) {
                ++str1;
            }
            if (!is_alpha(*str2)) {
                ++str2;
            }
        }
    }

    while (*str1 && !is_alpha(*str1)) {
        ++str1;
    }
    while (*str2 && !is_alpha(*str2)) {
        ++str2;
    }

    return to_lower(*str1) - to_lower(*str2);
}

/*!
 * Compares two lines, discarding non-alpha characters, processing the lines in reversed order
 *
 * @param [in] line1 first pointer to line
 * @param [in] line2 second pointer to line
 *
 * @return 0, if line1 is equal to line2, 1 if line1 is greater than line2, -1 otherwise
 * otherwise
 */
int line_cmp_reversed(const void *line1, const void *line2)
{
    size_t len1 = ((const line_t *) line1)->len,
           len2 = ((const line_t *) line2)->len;

    len1 = (len1 > 0) ? len1 : 1;
    len2 = (len2 > 0) ? len2 : 1;

    const char *str1R = ((const line_t *) line1)->str + len1 - 1,
               *str2R = ((const line_t *) line2)->str + len2 - 1;

    while (*str1R && *str2R) {
        if (to_lower(*str1R) == to_lower(*str2R)) {
            --str1R;
            --str2R;
        } else {
            if (is_alpha(*str1R) && is_alpha(*str2R)) {
                break;
            }

            if (!is_alpha(*str1R)) {
                --str1R;
            }

            if (!is_alpha(*str2R)) {
                --str2R;
            }
        }
    }

    while (*str1R && !is_alpha(*str1R)) {
        --str1R;
    }
    while (*str2R && !is_alpha(*str2R)) {
        --str2R;
    }

    return to_lower(*str1R) - to_lower(*str2R);
}