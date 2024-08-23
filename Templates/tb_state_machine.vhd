/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
File:           tb_$.vhd
Template        tb_state_machine.vhd
Authors:        Boris Karasov, Reshef Schachter
Date:           2023-2024
Institution:    Afeka College of Engineering
Description:    This is our final project for our Bachelor's degree in Electrical Engineering.
Notes:          The project is meant to work with a VHDL2008 compiler and a C++17 compiler.
Description:    This file is the testbench for the state machine. It is a universal testbench that can be used the state machines
                that are produced by our project. For the TB to work it requires the package file as well.
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------**/


-- This is the TB file for the machine WITH (V) OPTIMISATION!!!!


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_textio.all;
use std.textio.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

library work;
use work.pack_tb_$.all; -- Use the package

entity tb_$ is
end entity tb_$;

architecture arc_tb_universal of tb_$ is
  
  signal rst      : std_logic := '1';
  signal clk      : std_logic_vector(num_clocks  downto 0) := (others => '0');
  signal x        : std_logic_vector(num_inputs  downto 0) := (others => '0');
  signal y        : std_logic_vector(num_outputs downto 0);
  signal test_clk : std_logic := '0';
  
begin

  ----- Instantiate the device under test (DUT) -----
  DUT : entity work.top_$
    generic map (simulation => true, power_analyzer => false, baseline_power => false, full_fpga => false, duplicates => 1)
    port    map (rst => rst, clk => clk, x => x, y => y);


  ----- Generate clock(s) signal(s) -----
  genclk: process begin
    while simend = 0 loop
      clk <= not clk;
      wait for clk_period/2;
    end loop;
    wait;
  end process genclk;
  test_clk <= or clk; -- easy way to get a single clock signal for the stimulus


----- Test process -----
stimulus : process
  alias CS_0 is       << signal DUT.G1.FSM.s0 : state_0 >>;
  alias CS_1 is       << signal DUT.G1.FSM.s1 : state_1 >>;
  variable l : line;
begin
  -- Set initial values
  wait until falling_edge(test_clk);
  x <= test_array(0).x;
  rst <= '0'; -- Release reset
  
  for i in test_array'range loop

    wait until falling_edge(test_clk);
    -- Apply input stimulus: line numer 'i'
    CS_0 <= force test_array(i).CS_0;
    CS_1 <= force test_array(i).CS_1;
    x <= test_array(i).x;
    wait for clk_period/10;
    CS_0 <= release;
    CS_1 <= release;
    wait for clk_period/10;

    wait until rising_edge(test_clk);
    if (test_array(i).C_fsm /= test_array(i).N_fsm) then -- wait 1 additional clock cycle if the CFSM is going to change
      wait until rising_edge(test_clk);
    end if;
    wait on y for clk_period/10; -- wait for the output to stabilize

    -- Check the output and Assert the result of y and NS (CS should now be the next state)
    write(l, string'("")); writeline(OUTPUT, l);
    assert (y = test_array(i).y)
      report "Bad product at line " & integer'image(i) & LF &
             "Expected: y = " & to_string(test_array(i).y) & " ; NS_0 = " & to_string(test_array(i).NS_0) & " ; " & "NS_1 = " & to_string(test_array(i).NS_1) & LF &
             "Got:      y = " & to_string(y)               & " ; NS_0 = " & to_string(CS_0)               & " ; " & "NS_1 = " & to_string(CS_1)
            severity warning;
    
            assert (y /= test_array(i).y)
      report "Line " & integer'image(i) & " success" severity note;
    
  
  end loop; 
    
  report "Testbench finished" severity note;
  wait;
end process stimulus;

end architecture arc_tb_universal;