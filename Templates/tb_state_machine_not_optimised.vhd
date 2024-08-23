/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
File:           tb_$.vhd
Template        tb_state_machine_not_optimised.vhd
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


-- This is the TB file for the machine WITHOUT (X) OPTIMISATION!!!!


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
  
  -- Signals
  signal rst  : std_logic := '1';
  signal clk  : std_logic_vector(num_clocks  downto 0) := (others => '0');
  signal x    : std_logic_vector(num_inputs  downto 0) := (others => '0');
  signal y    : std_logic_vector(num_outputs downto 0);
  signal done : boolean := false;
  
begin

  -- Create instance of the device under test (DUT)
  DUT : entity work.top_$
    generic map (simulation => true, power_analyzer => false, baseline_power => false, full_fpga => false, duplicates => 1)
    port    map (rst => rst, clk => clk, x => x, y => y);

  
  -- Generate clock(s)
  genclk: process begin
    while not done loop
      clk <= not clk;
      wait for clk_period/2;
    end loop;
    wait;
  end process genclk;

  -- Test process
  stimulus : process is
    alias CS is << signal DUT.G1.FSM.s0 : state >>;
  begin
    
    x <= (others => '0');
    wait for 1 ns;
    wait until rising_edge(clk(0));
    rst <= '0' after 2 * clk_period; -- Release reset after 2cc
    wait for 4 * clk_period;

    for i in test_array'range loop
      
      wait until rising_edge(clk(0));
      -- Apply input stimulus
      x <= test_array(i).x;
      CS <= force test_array(i).CS;
	    wait for 0 ns;
      wait until rising_edge(clk(0));
	  
      CS <= release;
      wait for 0 ns;

      assert (y = test_array(i).y)
          report "Immediate check failed at line " & integer'image(i) & ": Expected y = " & to_string(test_array(i).y) & ", Got y = " & to_string(y)
          severity warning;
      assert (CS = test_array(i).NS)
          report "State check failed at line " & integer'image(i) & ": Expected NS = " & to_string(test_array(i).NS) & ", Got NS = " & to_string(CS)
          severity warning;
      
      Assert (not ((y = test_array(i).y) and (CS = test_array(i).NS)))
          report "Good product at line " & integer'image(i) 
          severity note;
      
      wait for 2 * clk_period;

    end loop;
    done <= true;
    report "Testbench finished" severity note;
    wait;

  end process stimulus;

end architecture arc_tb_universal;

