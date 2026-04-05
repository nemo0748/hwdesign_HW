open_project -reset poly_proj
set_top poly
add_files poly.cpp 
add_files -tb poly_tb.cpp 

open_solution -reset "solution1"
set_part {xc7z020clg484-1}
create_clock -period 10


if {[catch {csim_design } res]} {
    puts "FAIL:  poly C-Simulation failed."
    puts $res
    exit 1
}

puts "SUCCESS:  poly C-Simulation passed."

# ------------------------------------------------------------
# C Synthesis.  Comment out if you want to run C simulation only.
# ------------------------------------------------------------
if {[catch {csynth_design } res]} {
    puts "FAIL:  poly C-Synthesis failed."
    puts $res
    exit 1
}

puts "SUCCESS:  poly C-Synthesis passed."


# ------------------------------------------------------------
# RTL Co-Simulation
# Pass argument "rtl" to testbench via cosim.argv
# Comment out if you want to run C synthesis only.
# ------------------------------------------------------------
if {[catch {cosim_design -trace_level all} res]} {
    puts "FAIL:  poly RTL Co-Simulation failed."
    puts $res
    exit 1
}
puts "SUCCESS:  poly RTL Co-Simulation passed."
