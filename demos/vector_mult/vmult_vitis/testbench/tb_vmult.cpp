# include "../include/vmult.h"
#include <iostream>
#include <cmath>

int main() {
    data_t a[MAX_SIZE], b[MAX_SIZE], c[MAX_SIZE];
    data_t c_exp[MAX_SIZE];
    int n = MAX_SIZE;

    // Create test vectors and expected results
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = 2 * i;
        c_exp[i] = a[i] * b[i];
    }

    // Call the HLS vector multiplication function
    vec_mult(a, b, c, n);

    // Verify results
    bool pass = true;
    float tol = 1e-6;
    for (int i = 0; i < n; i++) {
#if DATA_FLOAT
        bool err = (fabs(c[i] - c_exp[i]) > tol);
#else
        bool err = (c[i] != c_exp[i]);
#endif
        if (err) {
            std::cout << "Mismatch at " << i << ": " << c[i] << " != " << c_exp[i] << std::endl;
            pass = false;
        }
    }

    std::cout << (pass ? "Test passed!" : "Test failed!") << std::endl;
    return 0;
}
