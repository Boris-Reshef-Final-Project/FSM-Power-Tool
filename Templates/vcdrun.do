vsim work.tb_$
vcd file -dumpports $.vcd
add wave -r *
run -all
vcd off
quit -sim