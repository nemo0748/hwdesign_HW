#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>

static std::string get_tb_dir() {
    std::string path = __FILE__;
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? "." : path.substr(0, pos);
}

static const std::string tb_dir = get_tb_dir();


// Forward declaration of the function under test
void fsolve(
        float a0,
        float a1,
        float a2,
        float x0,
        float tol,
        int max_iter,
        float step,
        float &x,
        float &fx,
        int &niter);

// Structure to hold test vectors
struct TestVector {
    float a0;
    float a1;
    float a2;
    float x0;
    float step;
    float tol;
    int max_iter;
    float x_root_expected;
    float fx_expected;
    int iterations_expected;
};

// Function to parse CSV line
bool parse_csv_line(const std::string &line, TestVector &tv, bool is_header = false) {
    if (is_header) return true; // Skip header line
    
    std::stringstream ss(line);
    std::string token;
    int field_count = 0;

    while (std::getline(ss, token, ',')) {
        if (field_count == 0) {
            tv.a0 = std::stof(token);
        } else if (field_count == 1) {
            tv.a1 = std::stof(token);
        } else if (field_count == 2) {
            tv.a2 = std::stof(token);
        } else if (field_count == 3) {
            tv.x0 = std::stof(token);
        } else if (field_count == 4) {
            tv.step = std::stof(token);
        } else if (field_count == 5) {
            tv.tol = std::stof(token);
        } else if (field_count == 6) {
            tv.max_iter = std::stoi(token);
        } else if (field_count == 7) {
            tv.x_root_expected = std::stof(token);
        } else if (field_count == 8) {
            tv.fx_expected = std::stof(token);
        } else if (field_count == 9) {
            tv.iterations_expected = std::stoi(token);
        }
        field_count++;
    }

    return field_count == 10;
}

int main(int argc, char** argv) {

    // Determine if "rtl" argument is provided
    bool test_rtl = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "rtl") {
            test_rtl = true;
            break;
        }
    }
    
    // Read the the test vectors from the CSV file
    // Note that the path is relative to the location of this source file,
    // so it should work regardless of the current working directory
    // when running the testbench
    std::string csv_path = tb_dir + "/test_outputs/tv_python.csv";
    std::cout << "Resolved CSV path = " << csv_path << std::endl;

    std::ifstream csv(csv_path);
    if (!csv.is_open()) {
        std::cerr << "Error: Could not open CSV file" << std::endl;
        return 1;
    }

    // Select the output CSV file based on the mode 
    // (C simulation or RTL simulation)
    std::string vitis_path;
    if (test_rtl) {
        std::cout << "Running in RTL co-simulation mode" << std::endl;
        vitis_path = tb_dir + "/test_outputs/tv_rtl.csv";
    } else {
        std::cout << "Running in C simulation mode" << std::endl;
        vitis_path = tb_dir + "/test_outputs/tv_csim.csv";
    }
    std::ofstream vitis_csv(vitis_path);
    if (!vitis_csv.is_open()) {
        std::cerr << "Error: Could not open output CSV file" << std::endl;
        return 1;
    }

    // Write header to output CSV file
    vitis_csv << "a0,a1,a2,x0,step,tol,max_iter,x_root,fx,iterations,";
    vitis_csv << "x_root_dut,fx_dut,niter_dut" << std::endl;


    std::vector<TestVector> test_vectors;
    std::string line;
    
    // Skip header line
    std::getline(csv, line);

    // Read all test vectors
    while (std::getline(csv, line)) {
        if (line.empty()) continue;
        
        TestVector tv;
        if (parse_csv_line(line, tv)) {
            test_vectors.push_back(tv);
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }
    csv.close();

    // Run tests
    int pass_count = 0;
    int fail_count = 0;

    std::cout << "========================================" << std::endl;
    std::cout << "fsolve Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total test vectors: " << test_vectors.size() << std::endl;
    std::cout << std::endl;

    for (size_t i = 0; i < test_vectors.size(); i++) {
        const TestVector &tv = test_vectors[i];
        
        // Call the function under test
        float x = 0.0f;
        float fx = 0.0f;
        int niter = 0;
        
        // TODO:  Call the fsolve with the parameters from 
        // the test vector and capture the outputs x, fx, and niter.
        //   fsolve(...);
     
        // Write results to output CSV file
        vitis_csv << std::setprecision(9) << std::fixed
              << tv.a0 << "," << tv.a1 << "," << tv.a2 << ","
              << tv.x0 << "," << tv.step << "," << tv.tol << ","
              << tv.max_iter << "," << tv.x_root_expected << ","
              << tv.fx_expected << "," << tv.iterations_expected << ","
              << x << "," << fx << "," << niter << std::endl;

        // TODO:  Compare the outputs:
        // Set test_pass = True if and only if:
        // 1) |x_ - tv_x_root_expected| < tol_x
        // 2) |fx - tv_fx_expected| < tol_fx
        // 3) |niter - tv_iterations_expected| <= tol_niter 

        // Print test results
        std::cout << "Test " << (i + 1) << ": ";
        std::cout << "a=[" << std::fixed << std::setprecision(2) 
                  << tv.a0 << ", " << tv.a1 << ", " << tv.a2 << "], ";
        std::cout << "x0=" << tv.x0;
        
        if (test_pass) {
            std::cout << " [PASS]" << std::endl;
            pass_count++;
        } else {
            std::cout << " [FAIL]" << std::endl;
            fail_count++;
        }
        
        // TODO:  Print detailed results for each test
        // You can use this as you please to debug your code
        // and determine what is not matching when a test fails.
        
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test Results: " << pass_count << " passed, " << fail_count << " failed" << std::endl;
    std::cout << "========================================" << std::endl;

    vitis_csv.close();

    return (fail_count == 0) ? 0 : 1;
}
