vcd file -dumpports {ex6.vcd }
vsim work.ex6_tb
add wave -r *
vcd add -r ex6_tb/DUT/*
run 5 us
vcd off





