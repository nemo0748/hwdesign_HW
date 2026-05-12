set unroll_factors {4 8}

foreach uf $unroll_factors {

    # Set the solution and open the project
    # Note we do not do `open_project vmult_hls --reset` 
    # since we want to maintain multiple solutions
    set sol_name "sol_uf$uf"
    open_project vmult_hls 
    set_top vec_mult

    # Pass UNROLL_FACTOR as a macro
    add_files -cflags "-Iinclude -DPIPELINE_EN=1 -DUNROLL_FACTOR=$uf" src/vmult.cpp
    add_files -tb testbench/tb_vmult.cpp

    open_solution $sol_name -reset

    # Set the part number for Pynq-Z2
    set_part {xc7z020clg400-1}
    create_clock -period 4 -name default

    # Set the part number for the RFSoC 4x2
    #set_part {xczu48dr-ffvg1517-2-e}
    #create_clock -period 3.33 -name default

    # Optional: run C simulation
    # csim_design

    csynth_design

    # Optional: run co-simulation
    # cosim_design

    # Optional: export IP
    # export_design -format ip_catalog

    # Save and close the project
    close_project

}

exit