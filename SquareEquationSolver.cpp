#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*!
 * Constant for indicating infinite number of roots
 */
const int INF_ROOTS = -1;

/*!
 * Constant for floating point number tolerance
 */
const double EPS = 1e-10;

int solve_square(double a, double b, double c, double *root1, double *root2);
int solve_linear(double b, double c, double *root);

int are_almost_equal(double f1, double f2);

int test_case(const char *name, int expr);
void test_solve_square(void);

int main(int argc, const char *argv[])
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
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    } else if (argc == 2) {
        if (strcmp(argv[1], "--t") == 0 || strcmp(argv[1], "-test") == 0) {
            test_solve_square();
        } else {
            printf("ERROR: invalid command line argument, use --t or -test for testing program\n");
            return EXIT_FAILURE;
        }
    } else {
        printf("ERROR: invalid command line arguments, use --t or -test for testing program\n");
        return EXIT_FAILURE;
    }
}

/*!
 * Solves square equation ax^2 + bx + c = 0 and saves its roots
 *
 * @param a [in] quadratic coefficient
 * @param b [in] linear coefficient
 * @param c [in] free term
 * @param root1 [out] pointer to the greater root
 * @param root2 [out] pointer to the lesser root
 *
 * @return the number of roots
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

    if (are_almost_equal(a, 0)) {
        int n_roots = solve_linear(b, c, root1);
        *root2 = *root1;

        return n_roots;
    } else {
        double d = b * b - 4 * a * c;
        double parabola_vertex = -b / (2 * a);

        if (are_almost_equal(d, 0)) {
            *root1 = *root2 = parabola_vertex;

            return 1;
        } else if (d > 0) {
            *root1 = parabola_vertex + sqrt(d) / (2 * a);
            *root2 = parabola_vertex - sqrt(d) / (2 * a);

            return 2;
        } else {
            return 0;
        }
    }
}

/*!
 * Solves linear equation bx + c = 0 and saves its root
 *
 * @param b [in] linear coefficient
 * @param c [in] free term
 * @param root [out] pointer to the greater root
 *
 * @return the number of roots
 *
 * @note Returns INFS_ROOTS in case of an infinite number of roots
 */
int solve_linear(double b, double c, double *root)
{
    assert(isfinite(b));
    assert(isfinite(c));
    assert(root != NULL);

    if (are_almost_equal(b, 0)) {
        return (are_almost_equal(c, 0)) ? INF_ROOTS : 0;
    } else {
        *root = -c / b;
        return 1;
    }
}

/*!
 * Compares 2 double precision floating point numbers
 *
 * @param dbl1 [in] first double precision floating point number
 * @param dbl2 [in] second double precision floating point number
 *
 * @return 1 if the numbers are almost equal, considering the tolerance, otherwise 0
 *
 * @note The tolerance is defined by EPS
 */
int are_almost_equal(double dbl1, double dbl2)
{
    assert(isfinite(dbl1));
    assert(isfinite(dbl2));

    return (fabs(dbl1 - dbl2) < EPS) ? 1 : 0;
}

/*!
 * Tests a case
 *
 * @param [in] name name of test case
 * @param [in] expr expression to be tested
 *
 * @return 1 if expr is true, otherwise 0
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

/*!
 * Tests solve_square function
 */
void test_solve_square(void)
{
    printf("Testing solve_square function:\n");

    double root1 = 0, root2 = 0;

    int n_tests_passed = 0, n_tests_failed = 0;

#define TEST_CASE(name, expr) (test_case((name), (expr)) ? ++n_tests_passed : ++n_tests_failed)

    TEST_CASE("infinite number of roots",
              solve_square(0, 0, 0, &root1, &root2) == INF_ROOTS);
    TEST_CASE("0 roots, constant equation",
              solve_square(0, 0, 1, &root1, &root2) == 0);
    TEST_CASE("0 roots, quadratic equation",
              solve_square(1, 1, 1, &root1, &root2) == 0);
    TEST_CASE("1 root, linear equation, number of roots",
              solve_square(0, 1, 1, &root1, &root2) == 1);
    TEST_CASE("1 root, linear equation, correctness of roots",
              are_almost_equal(root1, root2) && are_almost_equal(root1, -1));
    TEST_CASE("1 root, quadratic equation, number of roots",
              solve_square(1, -2, 1, &root1, &root2) == 1);
    TEST_CASE("1 root, quadratic equation, correctness of roots",
              are_almost_equal(root1, root2) && are_almost_equal(root1, 1));
    TEST_CASE("2 roots, number of roots",
              solve_square(2, 5, 3, &root1, &root2) == 2);
    TEST_CASE("2 roots, correctness of roots",
              are_almost_equal(root1, -1) && are_almost_equal(root2, -1.5));

#undef TEST_CASE

    printf("Finished testing solve_square function: %d tests passed, %d tests failed. The"
           "total number of tests was: %d\n",
           n_tests_passed, n_tests_failed, n_tests_passed + n_tests_failed);
}