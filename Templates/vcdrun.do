vsim work.tb_$
vcd file -dumpports $.vcd
add wave -r *
vcd add -r tb_$/DUT/*
run -all
vcd off
quit -sim