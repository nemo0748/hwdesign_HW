`timescale 1ns/1ps

module tb_vac_csv;

    // Parameters
    localparam int DATA_W = 32;
    localparam int ADDR_W = 5;
    localparam time CLK_PERIOD = 10ns;
    localparam int PIPE_LAT = 4;

    // Signals
    logic clk = 0, rst;
    logic [ADDR_W-1:0] s_axil_awaddr;
    logic s_axil_awvalid, s_axil_awready;
    logic [DATA_W-1:0] s_axil_wdata;
    logic s_axil_wvalid, s_axil_wready;
    logic [1:0] s_axil_bresp;
    logic s_axil_bvalid, s_axil_bready;

    logic signed [31:0] s_axis_tdata;
    logic s_axis_tvalid, s_axis_tready;
    logic signed [31:0] result_sat, result_ovf;

    // Clock
    always #(CLK_PERIOD/2) clk = ~clk;

    // DUT Instantiation
    vac #(.DATA_W(DATA_W), .ADDR_W(ADDR_W)) dut (
        .clk            (clk),
        .rst            (rst),
        .s_axil_awaddr  (s_axil_awaddr),
        .s_axil_awvalid (s_axil_awvalid),
        .s_axil_awready (s_axil_awready),
        .s_axil_wdata   (s_axil_wdata),
        .s_axil_wvalid  (s_axil_wvalid),
        .s_axil_wready  (s_axil_wready),
        .s_axil_bresp   (s_axil_bresp),
        .s_axil_bvalid  (s_axil_bvalid),
        .s_axil_bready  (s_axil_bready),
        .s_axis_tdata   (s_axis_tdata),
        .s_axis_tvalid  (s_axis_tvalid),
        .s_axis_tready  (s_axis_tready),
        .result_sat     (result_sat),
        .result_ovf     (result_ovf)
    );

    assign s_axil_bready = 1'b1;

    // Non-hanging AXI Write — flattened (no fork/join_any) for portability across
    // simulators that don't fully support `disable fork` inside a join_any.
    integer write_timeout;
    task automatic safe_write(input [ADDR_W-1:0] addr, input [DATA_W-1:0] data);
        @(posedge clk);
        s_axil_awaddr  <= addr; s_axil_awvalid <= 1'b1;
        s_axil_wdata   <= data; s_axil_wvalid  <= 1'b1;
        // Wait until the slave samples both VALIDs while READY is asserted.
        // With the fixed regbank, AWREADY/WREADY drop only after a successful
        // accept (because BVALID then rises), so a single posedge after the
        // ready check guarantees the write has committed.
        write_timeout = 0;
        while (!(s_axil_awready && s_axil_wready) && write_timeout < 100) begin
            @(posedge clk);
            write_timeout = write_timeout + 1;
        end
        if (write_timeout >= 100) $display("ERROR: AXI Write Timeout at %h", addr);
        @(posedge clk);
        s_axil_awvalid <= 1'b0; s_axil_wvalid  <= 1'b0;
    endtask

    // ------------------------------------------------------------------
    // Manual signed-integer CSV parser
    //
    // Vivado XSim 2023.x (and several other simulators) has a known bug
    // where $fscanf("%d,...") returns wrong values for negative numbers and
    // silently skips those rows. We work around it by reading whole lines
    // with $fgets and tokenizing them ourselves, byte-by-byte. Uses only
    // basic Verilog-2005 constructs (no `break`, no `int` function ports)
    // so it compiles cleanly on Vivado XSim, ModelSim, VCS, and Icarus.
    //
    // $fgets writes the line RIGHT-ALIGNED in the packed reg: the LAST byte
    // of the line lands at LSB (index 0) and the first byte at index ret-1
    // (where `ret` is the number of bytes returned by $fgets). We therefore
    // walk indices from `ret-1` down to `0`. Each character occupies one
    // byte at lbuf[8*idx +: 8].
    // ------------------------------------------------------------------
    localparam integer LBUF_BYTES = 512;
    integer fin, fout;
    integer scan_ok;
    integer fgets_ret;
    reg [8*LBUF_BYTES-1:0] line_buf;
    reg [8*LBUF_BYTES-1:0] header_line;

    logic signed [31:0] tv_A, tv_B, tv_C, tv_D, tv_x, tv_exp_sat, tv_exp_ovf;
    logic signed [31:0] field_vals [0:6];

    // Allow input CSV path to be overridden from the simulator command line
    // with +TV=<path>. Default tries the same directory first, then ../, so
    // the testbench works whether you run it from inside or alongside the
    // project folder (Vivado/VCS/ModelSim/Icarus all behave differently on cwd).
    string tv_path;
    integer vector_count, processed_count;

    // Parser state (shared across the helper tasks below).
    // `cursor` is a DOWNWARD index into line_buf: the first char of the line
    // is at cursor = line_len-1, the last at cursor = 0. We stop when
    // cursor < 0 or when we hit a newline / null.
    integer cursor;
    integer line_len;
    integer n_fields;
    reg [7:0] ch;
    reg neg;
    reg [63:0] acc;
    reg parsed_any;
    reg done_num;

    // Task: parse one signed integer beginning at the current cursor position.
    // Skips leading whitespace, commas, and an optional sign. Updates
    // `cursor`, `acc`, `neg`, and `parsed_any`.
    task automatic parse_one_int;
        begin
            // Skip leading whitespace, CR, and commas
            done_num = 1'b0;
            while (cursor >= 0 && !done_num) begin
                ch = line_buf[8*cursor +: 8];
                if (ch == " " || ch == "\t" || ch == "\r" || ch == ",") begin
                    cursor = cursor - 1;
                end else begin
                    done_num = 1'b1;
                end
            end

            // Optional sign
            neg = 1'b0;
            if (cursor >= 0) begin
                ch = line_buf[8*cursor +: 8];
                if (ch == "-") begin
                    neg    = 1'b1;
                    cursor = cursor - 1;
                end else if (ch == "+") begin
                    cursor = cursor - 1;
                end
            end

            // Digits
            acc        = 64'd0;
            parsed_any = 1'b0;
            done_num   = 1'b0;
            while (cursor >= 0 && !done_num) begin
                ch = line_buf[8*cursor +: 8];
                if (ch >= "0" && ch <= "9") begin
                    acc        = acc * 64'd10 + (ch - "0");
                    parsed_any = 1'b1;
                    cursor     = cursor - 1;
                end else begin
                    done_num = 1'b1;
                end
            end
        end
    endtask

    // Task: parse a full CSV line of up to 7 signed integers from line_buf
    // into field_vals[0..6]. Sets `n_fields` to the number of fields parsed.
    // `line_len` (the number of bytes returned by $fgets) must already be set.
    task automatic parse_csv_line;
        reg done_line;
        begin
            n_fields  = 0;
            cursor    = line_len - 1;
            done_line = 1'b0;
            while (cursor >= 0 && n_fields < 7 && !done_line) begin
                ch = line_buf[8*cursor +: 8];
                // Stop at end-of-line / null terminator
                if (ch == 8'h00 || ch == "\n") begin
                    done_line = 1'b1;
                end else begin
                    parse_one_int();
                    if (parsed_any) begin
                        if (neg) field_vals[n_fields] = -$signed(acc[31:0]);
                        else     field_vals[n_fields] =  $signed(acc[31:0]);
                        n_fields = n_fields + 1;
                    end else begin
                        // No digits parsed — advance one to avoid infinite loop
                        cursor = cursor - 1;
                    end
                end
            end
        end
    endtask

    initial begin
        // Initialize stream signals so they are never X
        s_axil_awaddr  = '0; s_axil_awvalid = 1'b0;
        s_axil_wdata   = '0; s_axil_wvalid  = 1'b0;
        s_axis_tdata   = '0; s_axis_tvalid  = 1'b0;

        // Reset sequence
        rst = 1; repeat(5) @(posedge clk); rst = 0; repeat(5) @(posedge clk);

        // Resolve input CSV path (try several locations so locally-run sims work)
        if (!$value$plusargs("TV=%s", tv_path)) tv_path = "test_vectors.csv";
        fin = $fopen(tv_path, "r");
        if (!fin) begin
            tv_path = "../test_vectors.csv";
            fin     = $fopen(tv_path, "r");
        end
        if (!fin) begin
            tv_path = "./test_vectors.csv";
            fin     = $fopen(tv_path, "r");
        end
        fout = $fopen("results.csv", "w");

        if (!fin)  $fatal(1, "Could not open input CSV (tried test_vectors.csv, ../test_vectors.csv, ./test_vectors.csv). Pass +TV=<path> to override.");
        if (!fout) $fatal(1, "Could not create output CSV");

        $display("Reading vectors from: %s", tv_path);

        // Skip input header and write output header
        fgets_ret = $fgets(header_line, fin);
        $fwrite(fout, "A,B,C,D,x,got_sat,got_ovf,exp_sat,exp_ovf,status\n");

        vector_count    = 0;
        processed_count = 0;
        while (!$feof(fin)) begin
            // Read the line as a string and parse manually — works around the
            // Vivado XSim $fscanf("%d") bug that mishandles negative numbers
            // and silently skips those rows.
            line_buf  = '0;
            fgets_ret = $fgets(line_buf, fin);
            if (fgets_ret <= 0) begin
                // End-of-file
                vector_count = vector_count;  // no-op
            end else begin
                line_len = fgets_ret;
                parse_csv_line();
                vector_count = vector_count + 1;

                if (n_fields == 7) begin
                    tv_A       = field_vals[0];
                    tv_B       = field_vals[1];
                    tv_C       = field_vals[2];
                    tv_D       = field_vals[3];
                    tv_x       = field_vals[4];
                    tv_exp_sat = field_vals[5];
                    tv_exp_ovf = field_vals[6];

                    processed_count = processed_count + 1;

                    // 1. Write coefficients
                    safe_write(5'h00, tv_A);
                    safe_write(5'h04, tv_B);
                    safe_write(5'h08, tv_C);
                    safe_write(5'h0C, tv_D);

                    // 2. Wait for AXI writes to fully settle and clear the pipe
                    repeat(10) @(posedge clk);

                    // 3. Send X
                    @(posedge clk);
                    s_axis_tdata  <= tv_x;
                    s_axis_tvalid <= 1'b1;
                    @(posedge clk);
                    s_axis_tvalid <= 1'b0;

                    // 4. Wait for the specific pipeline latency
                    repeat(PIPE_LAT) @(posedge clk);

                    // Write results to CSV
                    if (result_sat === tv_exp_sat && result_ovf === tv_exp_ovf) begin
                        $fwrite(fout, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,PASS\n",
                                tv_A, tv_B, tv_C, tv_D, tv_x,
                                result_sat, result_ovf, tv_exp_sat, tv_exp_ovf);
                    end else begin
                        $fwrite(fout, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,FAIL\n",
                                tv_A, tv_B, tv_C, tv_D, tv_x,
                                result_sat, result_ovf, tv_exp_sat, tv_exp_ovf);
                    end
                end else if (n_fields > 0) begin
                    // Partially parsed (and not blank) — emit a warning
                    $display("WARNING: line %0d only had %0d fields (expected 7)",
                             vector_count, n_fields);
                end
                // else: blank line, silently ignore
            end
        end

        $fclose(fin);
        $fclose(fout);
        $display("Done! Processed %0d vectors. Results saved to results.csv",
                 processed_count);
        $finish;
    end
endmodule
