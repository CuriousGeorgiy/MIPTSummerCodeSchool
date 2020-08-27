#include <stdio.h>
#include <math.h>
#include <string.h>

#include <assert.h>

#define INF_ROOTS -1

int solve_square(double a, double b, double c, double *root1, double *root2);

int is_equal(double f1, double f2);

void test_solve_square(void);
int test_case(const char *name, int expr);

int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf("Square equation solver\n");

        double a = 0, b = 0, c = 0;
        printf("Enter a, b, c coefficients\n");

        if (scanf("%lg %lg %lg", &a, &b, &c) != 3) {
            printf("ERROR: invalid input\n");
            return 2;
        }

        double root1 = 0, root2 = 0;
        int n_roots = solve_square(a, b, c, &root1, &root2);

        switch (n_roots) {
        case 0:
            printf("Square equation does not have roots\n");
            break;
        case 1:
            printf("Square equation has 1 root: %lg\n", root1);
            break;
        case 2:
            printf("Square equation has 2 roots: %lg and %lg\n", root1, root2);
            break;
        case INF_ROOTS:
            printf("Square equation has an infinite number of roots\n");
            break;
        default:
            printf("ERROR: unexpected behavior of square equation solver\n");
            return 1;
        }
    } else if (argc == 2) {
        if (strcmp(argv[1], "--t") == 0 || strcmp(argv[1], "-test") == 0) {
            test_solve_square();
        } else {
            printf("ERROR: invalid command line argument, use --t or -test for testing\n");
            return 3;
        }
    } else {
        printf("ERROR: invalid command line arguments, use --t or -test for testing\n");
        return 3;
    }

    return 0;
}

/*! Solves square equation a*x^2 + b*x + c = 0 saves its roots
 *
 *  @param a [in] quadratic coefficient
 *  @param b [in] linear coefficient
 *  @param c [in] free term
 *  @param root1 [out] pointer to the greater root
 *  @param root2 [out] pointer to the lesser root
 *
 *  @return The number of roots
 *
 *  @note Returns INF_ROOTS in case of an infinite number of roots
 */
int solve_square(double a, double b, double c, double *root1, double *root2)
{
    assert(isfinite(a));
    assert(isfinite(b));
    assert(isfinite(c));

    assert(root1 != NULL);
    assert(root2 != NULL);
    assert(root1 != root2);

    if (is_equal(a, 0)) {
        if (is_equal(b, 0)) {
            return (is_equal(c, 0)) ? INF_ROOTS : 0;
        } else {
            *root1 = *root2 = -c / b;
            return 1;
        }
    } else {
        double d = b * b - 4 * a * c;

        if (isgreater(d, 0)) {
            *root1 = (-b + sqrt(d)) / (2 * a);
            *root2 = (-b - sqrt(d)) / (2 * a);

            return 2;
        } else if (isless(d, 0)) {
            return 0;
        } else {
            *root1 = *root2 = -b / (2 * a);
            return 1;
        }
    }
}

/*! Compares 2 floats
 *
 *  @param f1 [in] first float
 *  @param f2 [in] second float
 *
 *  @return 1 if the floats are equal, otherwise 0
 */
int is_equal(double f1, double f2)
{
    return (islessequal(f1, f2) && isgreaterequal(f1, f2)) ? 1 : 0;
}

/*! Tests a case
 *
 *  @param name name of test case
 *  @param expr expression to be tested
 *
 *  @return 1 if expr is true, otherwise 0
 */
int test_case(const char *name, int expr)
{
    if (expr) {
        printf("\ttest \"%s\" passed\n", name);
        return 1;
    } else {
        printf("\ttest \"%s\" failed\n", name);
        return 0;
    }
}

/*! Tests solve_square function
 *
 */
void test_solve_square(void)
{
    double root1 = 0, root2 = 0;
    int n_tests_passed = 0, n_tests_failed = 0;

    printf("Testing solve_square function:\n");

#define TEST_CASE(name, expr) test_case((name), (expr)) ? ++n_tests_passed : ++n_tests_failed
    TEST_CASE("infinite number of roots",
              solve_square(0, 0, 0, &root1, &root2) == INF_ROOTS);
    TEST_CASE("0 roots, constant equation",
              solve_square(0, 0, 1, &root1, &root2) == 0);
    TEST_CASE("0 roots, quadratic equation",
              solve_square(1, 1, 1, &root1, &root2) == 0);
    TEST_CASE("1 root, linear equation, number of roots",
              solve_square(0, 1, 1, &root1, &root2) == 1);
    TEST_CASE("1 root, linear equation, correctness of roots",
              is_equal(root1, root2) && is_equal(root1, -1));
    TEST_CASE("1 root, quadratic equation, number of roots",
              solve_square(1, -2, 1, &root1, &root2) == 1);
    TEST_CASE("1 root, quadratic equation, correctness of roots",
              is_equal(root1, root2) && is_equal(root1, 1));
    TEST_CASE("2 roots, number of roots",
              solve_square(2, 5, 3, &root1, &root2) == 2);
    TEST_CASE("2 roots, correctness of roots",
              is_equal(root1, -1) && is_equal(root2, -1.5));
#undef TEST_CASE

    printf("Finished testing solve_square function: %d tests passed, %d tests failed, the"
           "total number of tests was %d\n",
           n_tests_passed, n_tests_failed, n_tests_passed + n_tests_failed);
}