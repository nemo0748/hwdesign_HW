set unroll_factors {1 2 4}

foreach uf $unroll_factors {
    set project_name [format "poly_uf%d" $uf]
    set cflags [format "-DUNROLL_FACTOR=%d" $uf]

    puts "============================================================"
    puts [format "Running HLS flow for UNROLL_FACTOR=%d in project %s" $uf $project_name]
    puts "============================================================"

    open_project -reset $project_name
    set_top poly
    add_files -cflags $cflags poly.cpp
    add_files -tb -cflags $cflags poly_tb.cpp

    open_solution -reset "solution1"
    set_part {xc7z020clg484-1}
    create_clock -period 10

    if {[catch {csim_design} res]} {
        puts [format "FAIL: poly C-Simulation failed for UNROLL_FACTOR=%d." $uf]
        puts $res
        close_project
        exit 1
    }

    puts [format "SUCCESS: poly C-Simulation passed for UNROLL_FACTOR=%d." $uf]

    # ------------------------------------------------------------
    # C Synthesis. Comment out if you want to run C simulation only.
    # ------------------------------------------------------------
    if {[catch {csynth_design} res]} {
        puts [format "FAIL: poly C-Synthesis failed for UNROLL_FACTOR=%d." $uf]
        puts $res
        close_project
        exit 1
    }

    puts [format "SUCCESS: poly C-Synthesis passed for UNROLL_FACTOR=%d." $uf]

    # ------------------------------------------------------------
    # RTL Co-Simulation
    # Pass argument "rtl" to testbench via cosim.argv.
    # Comment out if you want to run C synthesis only.
    # ------------------------------------------------------------
    #if {[catch {cosim_design -trace_level all} res]} {
    #    puts [format "FAIL: poly RTL Co-Simulation failed for UNROLL_FACTOR=%d." $uf]
    #    puts $res
    #    close_project
    #    exit 1
    #}
    #puts [format "SUCCESS: poly RTL Co-Simulation passed for UNROLL_FACTOR=%d." $uf]

    close_project
}

exit
