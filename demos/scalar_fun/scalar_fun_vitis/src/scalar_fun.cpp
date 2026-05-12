

void simp_fun(int x, int w, int b, int& y) {

    #pragma HLS INTERFACE s_axilite port=x     bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=w     bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=b     bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=y     bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    int act_in = w * x + b;
    if (act_in > 0) 
        y = act_in;
    else
        y = 0;
    }