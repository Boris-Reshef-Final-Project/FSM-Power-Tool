/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
File:           tb_state_machine_not_optimised.vhd
Authors:        Boris Karasov, Reshef Schachter
Date:           2023-2024
Institution:    Afeka College of Engineering
Description:    This is our final project for our Bachelor's degree in Electrical Engineering.
Notes:          The project is meant to work with a VHDL2008 compiler and a C++17 compiler.
File Description: This file is the testbench for the state machine. It is a universal testbench that can be used the state machines
  that are produced by our project. For the TB to work it requires the package file as well.
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------**/


-- THIS IS FOR THE NOT-OPTIMISED VERSION!!!!


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_textio.all;
use std.textio.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

library work;
use work.tb_package_$.all; -- Use the package

entity tb_$ is
end entity tb_$;

architecture arc_tb_universal of tb_$ is
  signal rst : std_logic := '1';
  signal clk : std_logic_vector(num_clocks  downto 0) := (others => '0');
  signal x   : std_logic_vector(num_inputs  downto 0) := (others => '0');
  signal y   : std_logic_vector(num_outputs downto 0);
  
begin

  -- Instantiate the device under test (DUT)
  DUT : entity work.top_$
    generic map (simulation => true, power_analyzer => false, baseline_power => false, full_fpga => false, duplicates => 1);
    port    map (rst => rst, clk => clk, x => x, y => y);

  -- Generate clock(s) with enable
  -- This is going to be replaced with the PLL in the physical tests.
  clk <= not clk after (clk_period/2);


  -- Test process
  stimulus : process
    alias CS is       << signal DUT.s0 : state >>;    
    alias new_clk is  << signal DUT.clk_out : std_logic_vector(num_clocks downto 0) >>;
  begin
    
    x <= (others => '0');
    wait for 1 ns;
    wait until rising_edge(new_clk); -- wait for PLL to lock
    rst <= '0' after 2 * clk_period; -- Release reset after 2cc
    

    for i in test_array'range loop
      -- Apply input stimulus
      CS <= force test_array(i).CS;
      wait until rising_edge(new_clk);
      CS <= release;
      x <= test_array(i).x;
      -- Wait for the next clock edge
      wait until rising_edge(new_clk);
      -- Check the output and Assert the result of y and NS (CS should now be the next state)
      assert (y = test_array(i).y) and (CS = test_array(i).NS) 
        report "Bad product at line " & integer'image(i) & "\n" &
               "Expected: y = " & to_string(test_array(i).y) & " and NS = " & to_string(test_array(i).NS) & "\n" &
               "Got:      y = " & to_string(y)               & " and NS = " & to_string(CS) & "\n"
              severity warning;
    end loop; 

    wait;
  end process;

end architecture arc_tb_universal;

