open_project -reset poly_proj
set_top poly
add_files poly.cpp
add_files -tb poly_tb.cpp

open_solution -reset "solution1"
set_part {xc7z020clg484-1}
create_clock -period 10

set script_dir [file dirname [file normalize [info script]]]
set data_dir [file join $script_dir "data"]

if {[catch {csim_design -argv "$data_dir"} res]} {
    puts "FAIL:  poly C-Simulation failed."
    puts $res
    exit 1
}

puts "SUCCESS:  poly C-Simulation passed."

if {[catch {csynth_design} res]} {
    puts "FAIL:  poly C-Synthesis failed."
    puts $res
    exit 1
}

puts "SUCCESS: poly C-Synthesis passed."
exit 0
