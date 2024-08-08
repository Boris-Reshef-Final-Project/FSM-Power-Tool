/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
File:           tb_state_machine.vhd
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


-- THIS IS FOR THE OPTIMIZED VERSION!!!!


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
  signal rst      : std_logic := '1';
  signal clk      : std_logic_vector(num_clocks  downto 0) := (others => '0');
  signal new_clk  : std_logic_vector(num_clocks  downto 0) := (others => '0');
  signal x        : std_logic_vector(num_inputs  downto 0) := (others => '0');
  signal y        : std_logic_vector(num_outputs downto 0);
  signal test_clk : std_logic;
  signal new_clk  : std_logic := '0';
  
begin

  -- Instantiate the device under test (DUT)
  DUT : entity work.top_$
    generic map (simulation => true, power_analyzer => false, baseline_power => false, full_fpga => false, duplicates => 1)
    port    map (rst => rst, clk => clk, x => x, y => y);


  new_clk <= not new_clk after (clk_period/2);

  -- Generate clock(s) with enable
  -- This is going to be replaced with the PLL in the physical tests.
  gen_clk : process (all) is
    alias clken is << signal DUT.clken : std_logic_vector(clk'range) >>;
  begin
    clk <= clken and new_clk;
  end process gen_clk;


  test_clk <= or clk;


  -- Test process
  stimulus : process
    alias CS_0 is       << signal DUT.G_FSM.FSM.s0 : state_0 >>;
    alias CS_1 is       << signal DUT.G_FSM.FSM.s1 : state_1 >>;
    
  begin
    
    x <= (others => '0');
    wait for 1 ns;
    wait until rising_edge(test_clk);
    rst <= '0' after 2 * clk_period; -- Release reset after 2cc
    

    for i in test_array'range loop
      -- Apply input stimulus
      CS_0 <= force test_array(i).CS_0;
      CS_1 <= force test_array(i).CS_1;
      wait until rising_edge(test_clk);
      CS_0 <= release;
      CS_1 <= release;
      x <= test_array(i).x;
      -- Wait for the next clock edge
      wait until rising_edge(test_clk);
      if (test_array(i).C_fsm /= test_array(i).N_fsm) then -- wait 1 additional clock cycle if the CFSM is going to change
        wait until rising_edge(test_clk);
      end if;
      -- Check the output and Assert the result of y and NS (CS should now be the next state)
      assert (y = test_array(i).y)
        report "Bad product at line " & integer'image(i) & LF &
               "Expected: y = " & to_string(test_array(i).y) & " ; NS_0 = " & to_string(test_array(i).NS_0) & " ; NS_1 = " & to_string(test_array(i).NS_1) & LF &
               "Got:      y = " & to_string(y)               & " ; NS_0 = " & to_string(CS_0)               & " ; NS_1 = " & to_string(CS_1) & LF
              severity warning;
    end loop; 

    wait;
  end process;

end architecture arc_tb_universal;

