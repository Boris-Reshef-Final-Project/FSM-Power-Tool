set do_file_path [file dirname [info script]]
vcom -2008 -work work $do_file_path/%.vhd
vcom -2008 -work work $do_file_path/PLL_altpll.vhd
vcom -2008 -work work $do_file_path/pack_top_*.vhd
vcom -2008 -work work $do_file_path/top_*.vhd
vcom -2008 -work work $do_file_path/pack_tb_*.vhd
vcom -2008 -work work $do_file_path/tb_*.vhd