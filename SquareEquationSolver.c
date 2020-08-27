#include <stdio.h>
#include <math.h>
#include <assert.h>

#define INF_ROOTS -1

//! \brief Solves square equation a*x^2 + b*x + c = 0 saves its roots
//! \param a [in] quadratic coefficient
//! \param b [in] linear coefficient
//! \param c [in] free term
//! \param root1 [out] root with positive discriminant root
//! \param root2 [out] root with negative discriminant root
//! \return Return the number of roots or INF_ROOTS in case of infinite number of roots
int solve_square(double a, double b, double c, double *root1, double *root2);

//! \brief Compare 2 floats
//! \param f1
//! \param f2
//! \return Return 1 if the floats are equal or 0 if they are not
int is_equal(double f1, double f2);

int main()
{
    printf("Square equation solver\n");

    double a = 0, b = 0, c = 0;
    printf("Enter a, b, c coefficients\n");
    scanf("%lg %lg %lg", &a, &b, &c);

    double root1 = 0, root2 = 0;
    int nRoots = solve_square(a, b, c, &root1, &root2);

    switch (nRoots) {
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

    return 0;
}

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
            return (c == 0) ? INF_ROOTS : 0;
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

int is_equal(double f1, double f2)
{
    return (islessequal(f1, f2) && isgreaterequal(f1, f2)) ? 1 : 0;
}