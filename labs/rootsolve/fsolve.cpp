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
    float& x,
    float& fx,
    int& niter)
{
    // --- AXI4-LITE INTERFACE PRAGMAS ---
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
#pragma HLS interface s_axilite port=return   bundle=CTRL

// Local variables
    float x_curr = x0;
    float fx_curr = 0.0f;
    int i = 0;

    // loop with early termination
    for (i = 0; i < 2000; i++) {
#pragma HLS PIPELINE II=1
        if (i >= max_iter) break;
        fx_curr = a0 + x_curr * (a1 + x_curr * (a2 + x_curr));
        if (hls::abs(fx_curr) < tol) {
            break;
        }
        x_curr = x_curr - (step * fx_curr);
    }

    //internal state to AXI-mapped output 
    x = x_curr;
    fx = fx_curr;
    niter = i;
}