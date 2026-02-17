import os
import shutil
import shutil
import numpy as np
import pandas as pd
import json
import zipfile

class GradescopeTest:
    def __init__(self,
                 name : str,
                 max_score : int):
        self.name = name
        self.max_score = max_score
        
    def run(self, **kwargs):
        """
        Runs the test
        
        Returns:
        --------
        score : int
            The score obtained in the test
        feedback : str
            Feedback message for the test
        """
        raise NotImplementedError("Subclasses should implement this!")
    
    
class TestPython(GradescopeTest):
    """
    Test the python implementation 
    """
    def __init__(
            self,
            tolz : float =1e-4,
            tolf : float =1e-5,
            ntest_min : int =3):
        """
        Constructor

        Parameters:
        -----------        
        tolz : float
            Tolerance for zero (|f(x_root)| < tolz)
        tolf : float
            Tolerance for function value (|f(x_root) - fx| < tolf)
        ntest_min : int
            Minimum number of test samples required in the CSV file
        """
        self.tolz = tolz
        self.tolf = tolf
        self.ntest_min = ntest_min

        # Score for function value and zero tests
        self.max_score_fx = 10
        self.max_score_zero = 10
        max_score = self.max_score_fx + self.max_score_zero
        super().__init__(
            name=f"Python Implementation", 
            max_score=max_score)
    def run(self):
        """
        Test Python implementation by reading CSV and tests:

        1.  There are at least ntest_min samples in the CSV file
        2.  | f(x_root) = fx | < tolf
        3.  | fx | < tolz
        """
        # Construct CSV filename
        csv_file = f'test_outputs/tv_python.csv'
        
        try:
            # Read CSV file
            df = pd.read_csv(csv_file)
            
            # Check if there are enough samples
            num_samples = len(df)
            if num_samples < self.ntest_min:
                score = 0
                feedback = f"Insufficient samples: {num_samples} samples found, but at least 100 required."
                return score, feedback
            
            # Extract columns
            a0 = df['a0'].values
            a1 = df['a1'].values
            a2 = df['a2'].values
            x_root = df['x_root'].values
            fx = df['fx'].values

            # Compute MSE of fx
            froot = a0 + a1*x_root + a2*x_root**2 + x_root**3
            ferr = np.mean(np.abs(fx - froot))
            zerr = np.mean(np.abs(fx))
            sf = max(0, 1 - np.log10( max(ferr/self.tolf, 1) ) )
            score_fx = int(round(self.max_score_fx * sf))

            # Feedback for function value test
            feedback = "\nFunction value test:  "
            if score_fx == self.max_score_fx:
                feedback += "PASS"
            elif score_fx > 0:
                feedback += "PARTIAL CREDIT"
            else:
                feedback += "FAIL."  
            feedback += "  Function error: %12.4e tol: %12.e\n" %\
                        (ferr, self.tolf)
            
            # Score for zero test
            sz = max(0, 1 - np.log10( max(zerr/self.tolz, 1) ) ) 
            score_zero = int(round(self.max_score_zero * sz))
            feedback += "Zero test:  "
            if score_zero == self.max_score_zero:
                feedback += "PASS"
            elif score_zero > 0:
                feedback += "PARTIAL CREDIT"
            else:
                feedback += "FAIL."
            feedback += "  Zero error: %12.4e tol: %12.e\n" %\
                        (zerr, self.tolz)
            score = score_fx + score_zero
            return score, feedback
        
        except FileNotFoundError:
            score = 0
            feedback = f"Error: File '{csv_file}' not found."
            return score, feedback
        except Exception as e:
            score = 0
            feedback = f"Error reading or processing file: {str(e)}"
            return score, feedback


class TestVitis(GradescopeTest):
    """
    Test the Vitis DUT implementation using simulation CSV output.
    """
    def __init__(
            self,
            tolx: float = 1e-5,
            tol_fx: float = 1e-5,
            tol_niter: float = 3,
            ntest_min: int = 3,
            csim : bool = True):
        """
        Constructor

        Parameters:
        -----------
        tolx : float
            Tolerance for mean absolute root error
        tol_fx : float
            Tolerance for mean absolute function value error
        tol_niter : float
            Tolerance for mean absolute iteration count error
        ntest_min : int
            Minimum number of test samples required in the CSV file
        csim : bool
            Whether to test C simulation output (True) or RTL simulation output (False)
        """
        self.tolx = tolx
        self.tol_fx = tol_fx
        self.tol_niter = tol_niter
        self.ntest_min = ntest_min
        self.csim = csim

        self.max_score_x = 5
        self.max_score_fx = 5
        self.max_score_niter = 5
        max_score = self.max_score_x + self.max_score_fx + self.max_score_niter
        if self.csim:
            name = "Vitis DUT C-Simulation"
        else:
            name = "Vitis DUT RTL Simulation"
        super().__init__(
            name=name,
            max_score=max_score)

    def run(self):
        """
        Test Vitis implementation by reading CSV and tests:

        1. mean(abs(x_root - x_root_dut)) < tolx
        2. mean(abs(fx - fx_dut)) < tol_fx
        3. mean(abs(iterations - niter_dut)) < tol_niter
        """
        if self.csim:
            csv_file = 'test_outputs/tv_csim.csv'
        else:
            csv_file = 'test_outputs/tv_rtl.csv'

        try:
            df = pd.read_csv(csv_file)

            num_samples = len(df)
            if num_samples < self.ntest_min:
                score = 0
                feedback = f"Insufficient samples: {num_samples} samples found, but at least {self.ntest_min} required."
                return score, feedback

            required_cols = [
                'x_root', 'x_root_dut',
                'fx', 'fx_dut',
                'iterations', 'niter_dut'
            ]
            missing_cols = [c for c in required_cols if c not in df.columns]
            if len(missing_cols) > 0:
                score = 0
                feedback = f"Missing required columns in {csv_file}: {missing_cols}"
                return score, feedback

            xerr = np.mean(np.abs(df['x_root'].values - df['x_root_dut'].values))
            fxerr = np.mean(np.abs(df['fx'].values - df['fx_dut'].values))
            nitererr = np.mean(np.abs(df['iterations'].values - df['niter_dut'].values))

            score_x = self.max_score_x if xerr < self.tolx else 0
            score_fx = self.max_score_fx if fxerr < self.tol_fx else 0
            score_niter = self.max_score_niter if nitererr < self.tol_niter else 0

            feedback = "\nRoot test: "
            feedback += "PASS" if score_x == self.max_score_x else "FAIL"
            feedback += "  mean(abs(x_root-x_root_dut)): %12.4e tol: %12.4e\n" % (xerr, self.tolx)

            feedback += "Function test: "
            feedback += "PASS" if score_fx == self.max_score_fx else "FAIL"
            feedback += "  mean(abs(fx-fx_dut)): %12.4e tol: %12.4e\n" % (fxerr, self.tol_fx)

            feedback += "Iteration test: "
            feedback += "PASS" if score_niter == self.max_score_niter else "FAIL"
            feedback += "  mean(abs(iterations-niter_dut)): %12.4e tol: %12.4e\n" % (nitererr, self.tol_niter)

            score = score_x + score_fx + score_niter
            return score, feedback

        except FileNotFoundError:
            score = 0
            feedback = f"Error: File '{csv_file}' not found."
            return score, feedback
        except Exception as e:
            score = 0
            feedback = f"Error reading or processing file: {str(e)}"
            return score, feedback


def submit(
    test_outputs : list,
    total_score : float,
):
    
    
    # Check if required files exist
    tv_dir = os.path.join(os.getcwd(), 'test_outputs')
    zip_files = [ os.path.join(os.getcwd(),'fsolve.cpp'),\
                os.path.join(os.getcwd(),'tb_fsolve.cpp'),\
                os.path.join(os.getcwd(),'fsolve.ipynb'),\
                os.path.join(tv_dir, 'tv_csim.csv'),\
                os.path.join(tv_dir, 'tv_rtl.csv'),\
                os.path.join(tv_dir, 'tv_python.csv'),\
                os.path.join(tv_dir, 'fsolve_hist.png')]
    for f in zip_files:
        if not os.path.isfile(f):
            print(f"Error: Required file {f} not found in current directory.")
            exit(1)

    # Build results in new format
    results = {
        'tests': test_outputs,
        'score': total_score
    }


    # Write results to a JSON file
    with open('submitted_results.json', 'w') as f:
        json.dump(results, f, indent=4)

    # Create submission directory
    submission_dir = 'submission'
    if os.path.exists(submission_dir):
        shutil.rmtree(submission_dir)
    os.makedirs(submission_dir)

    # Copy files to submission directory
    shutil.copy('submitted_results.json', os.path.join(submission_dir, 'submitted_results.json'))
    for f in zip_files:
        shutil.copy(f, os.path.join(submission_dir, os.path.basename(f)))

    # Create submission.zip containing the results and required source files
    with zipfile.ZipFile('submission.zip', 'w') as zipf:
        zipf.write('submitted_results.json', arcname='submitted_results.json')
        for f in zip_files:
            zipf.write(f, arcname=os.path.basename(f))

    print("Submission package created: submission.zip")
    print("Submission directory created: submission/")
    print("Upload submission.zip to Gradescope.")

def main():
    import argparse


    # List of possible tests
    tests = {
        'python': TestPython(),
        'csim': TestVitis(csim=True),
        'rtl': TestVitis(csim=False)
    }
    
    # Set up argument parser
    parser = argparse.ArgumentParser(description='Run tests root solve lab')
    parser.add_argument('--tests', nargs='+', default=['python'], 
                        help='List of tests to run (e.g., python, all)')
    parser.add_argument('--submit', action='store_true',
                        help='Create submission package after running tests')
    args = parser.parse_args()
    
    # Initialize list to store test outputs
    test_outputs = []
    
    # Determine which tests to run
    if 'all' in [t.lower() for t in args.tests]:
        # Run all tests
        tests_to_run = tests.keys()
    else:
        # Run only specified tests
        tests_to_run = [t.lower() for t in args.tests]

    # Loop over the list of tests
    for tname, tclass in tests.items():
        if tname.lower() in tests_to_run:
            print(f"Running test: {tname}")
            
            # Run the test with default parameters
            score, feedback = tclass.run()
            
            # Create test output dictionary
            test_out = {
                'name': tclass.name,
                'max_score': tclass.max_score,
                'score': score, 
                'feedback': feedback
            }
            
            # Add to list of test outputs
            test_outputs.append(test_out)
            
            # Print test result
            print(f"\n{tclass.name}:")
            print(f"  Score: {test_out['score']}/{tclass.max_score}")
            print(f"  Feedback: {feedback}")
    
    # Print summary
    print("\n" + "="*60)
    print("Test Summary")
    print("="*60)
    total_score = 0
    total_max_score = 0
    for test_out in test_outputs:
        print(f"{test_out['name']}: {test_out['score']}/{test_out['max_score']}")
        total_score += test_out['score']
        total_max_score += test_out['max_score']
    print("="*60)
    print(f"Total Score: {total_score}/{total_max_score}")
    
    # Call submit if --submit flag is set
    if args.submit:
        print("\n" + "="*60)
        print("Creating submission package...")
        print("="*60)
        submit(test_outputs, total_score)
    
    return test_outputs


if __name__ == "__main__":
    main()

