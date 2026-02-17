#include <ap_fixed.h>
#include <hls_math.h>

/**
 * Root finding solver for monic cubic: f(x) = a0 + a1*x + a2*x^2 + x^3
 * 
 * Uses gradient descent: x[k+1] = x[k] - step*f(x[k])
 * 
 * Parameters (AXI4-Lite slaves):
 *   a0, a1, a2: Cubic coefficients
 *   x0: Initial guess
 *   tol: Convergence tolerance
 *   max_iter: Maximum iterations
 *   step: Step size for gradient descent
 * 
 * Outputs (AXI4-Lite slaves):
 *   x: Final estimated root
 *   fx: Function value at final x
 *   niter: Number of iterations performed
 */
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
        int &niter)
{
    // AXI4-Lite slave interface pragmas for inputs
    #pragma HLS interface s_axilite port=a0
    #pragma HLS interface s_axilite port=a1
    #pragma HLS interface s_axilite port=a2
    #pragma HLS interface s_axilite port=x0
    #pragma HLS interface s_axilite port=tol
    #pragma HLS interface s_axilite port=max_iter
    #pragma HLS interface s_axilite port=step
    
    // AXI4-Lite slave interface pragmas for outputs
    #pragma HLS interface s_axilite port=x
    #pragma HLS interface s_axilite port=fx
    #pragma HLS interface s_axilite port=niter
    #pragma HLS interface s_axilite port=return bundle=CTRL

    // TODO:  Implement the root finding algorithm using the Euler 
    // method.  Note:
    // - You should use a for-loop that iterates up to max_iter
    // - In each iteration, compute f(x), and update x
    // - Check for convergence by comparing |f(x)| to tol.  
    //   If converged, set niter and return.
    
}
