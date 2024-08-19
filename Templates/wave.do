onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate rst
add wave -noupdate clk
add wave -noupdate x
add wave -noupdate y
TreeUpdate [SetDefaultTree]
quietly WaveActivateNextPane
add wave -noupdate DUT/G1/FSM/clk
add wave -noupdate DUT/G1/FSM/clken
add wave -r -noupdate DUT/G1/FSM/s?
add wave -noupdate DUT/G1/FSM/x
add wave -noupdate DUT/G1/FSM/y
add wave -noupdate DUT/G1/FSM/z
add wave -r -noupdate -group {internal y} DUT/G1/FSM/sig_y?
add wave -r -noupdate -group {internal y} DUT/G1/FSM/y?
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 2} {0 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 150
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits us
update
WaveRestoreZoom {0 ps} {110000 ps}