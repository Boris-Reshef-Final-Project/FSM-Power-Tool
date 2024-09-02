vsim work.tb_$
vcd file $.vcd
add wave -r tb_$/DUT/*
vcd add -r tb_$/DUT/*
run -all
vcd off
quit -sim