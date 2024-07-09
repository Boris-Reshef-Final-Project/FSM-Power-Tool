vcd file -dumpports {$.vcd}
vsim work.tb_$
add wave -r *
vcd add -r tb_$/DUT/*
run @
vcd off