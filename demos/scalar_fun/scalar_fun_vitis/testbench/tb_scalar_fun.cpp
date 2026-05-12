#include <iostream>

void simp_fun(int x, int w, int b, int& y);

int main() {
    struct TestCase {
        int x, w, b;
    };

    TestCase tests[] = {
        {3, 2, 4},
        {-1, 5, 0},
        {10, -2, 3},
        {0, 1, -5},
        {7, 7, 7}
    };

    const int num_tests = sizeof(tests) / sizeof(TestCase);

    bool all_passed = true;

    for (int i = 0; i < num_tests; i++) {

        // Get test inputs
        int y;
        int x = tests[i].x;
        int w = tests[i].w;
        int b = tests[i].b;

        // Compute expected output
        int y_exp = w * x + b;
        if (y_exp < 0) y_exp = 0;

        simp_fun(x, w, b, y);


        std::cout << "Test " << i << " got " << y << ", expected " << y_exp;
        if (y != y_exp) {
            std::cout << " FAIL" << std::endl;
            all_passed = false;
        }
        else {  
            std::cout <<  " PASS" << std::endl;
        }
        
    if (all_passed)
        std::cout << "All tests passed!" << std::endl;
    else
        std::cout << "Some tests failed." << std::endl;
    }
    

    return 0;
}