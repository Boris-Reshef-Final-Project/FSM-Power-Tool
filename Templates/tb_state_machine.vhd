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
  signal rst  : std_logic := '1';
  signal clk  : std_logic_vector(num_clocks  downto 0) := (others => '0');
  signal x    : std_logic_vector(num_inputs  downto 0) := (others => '0');
  signal y    : std_logic_vector(num_outputs downto 0);
  signal done : boolean := false;

  constant error_reporting : integer := 1; -- 0 = off, 1 = only errors, 2 = errors and good products
  
  
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
    alias CS_0 is       << signal DUT.G1.FSM.s0 : state_0 >>;
    alias CS_1 is       << signal DUT.G1.FSM.s1 : state_1 >>;
    alias clken is      << signal DUT.G1.FSM.clken : std_logic_vector(clk'range) >>;
    variable l          : line;
    variable k          : integer := 0;
    variable halt       : integer := 0;
    
    variable good_lines : integer := 0;
    variable repeat     : integer := 0;
    variable run_time   : time := 0 ns;
    variable start_time : time := 0 ns;

  begin
    
    x <= (others => '0');
    wait for clk_period/10;
    wait until rising_edge(clk(0));
    rst <= '0' after 2 * clk_period; -- Release reset after 2cc
    wait for 4 * clk_period;
    
    start_time := now;
    while (run_time < run_length) or (repeat < 10) loop
      for i in test_array'range loop
        
        k := 0;
        halt := 0;
        wait until rising_edge(clk(0));
        while ((halt = 0)) loop
          k := k mod test_array'length;
          wait until falling_edge(clk(0));
          x <= test_array(k).x;
          wait for 0 ns;
          wait until falling_edge(clk(0));
          k := k+1;
          if ((clken'delayed(1 ns) /= "11") and (clken(test_array(i).C_fsm) = '1')) then
            if ((test_array(i).C_fsm = 0) and (CS_1 = test_array(i).CS_1)) then
              halt := 1;
            elsif ((test_array(i).C_fsm = 1) and (CS_0 = test_array(i).CS_0)) then
              halt := 1;
            else
              halt := 0;
            end if;
          end if;
        end loop;
        wait for 0 ns;
        
        wait until rising_edge(clk(0));
        x <= test_array(i).x;
        CS_0 <= force test_array(i).CS_0;
        CS_1 <= force test_array(i).CS_1;
        wait for 0 ns;
        wait until rising_edge(clk(0));
      
        CS_0 <= release;
        CS_1 <= release;
        wait for 0 ns;
      
        while not (CS_0'quiet and CS_1'quiet) loop
          wait for 0 ns;
        end loop;

        if (test_array(i).C_fsm /= test_array(i).N_fsm) then
          wait on y for clk_period/10;
        end if;
        
        if (repeat = 0) then
          if (error_reporting /= 0) then
            assert (y'delayed(clk_period/2) = test_array(i).y)
                report "Immediate check failed at line " & integer'image(i) & ": Expected y = " & to_string(test_array(i).y) & ", Got y = " & to_string(y)
                severity warning;
            assert (CS_0 = test_array(i).NS_0)
                report "State check failed at line " & integer'image(i) & ": Expected NS_0 = " & to_string(test_array(i).NS_0) & ", Got NS_0 = " & to_string(CS_0)
                severity warning;
            assert (CS_1 = test_array(i).NS_1)
                report "State check failed at line " & integer'image(i) & ": Expected NS_1 = " & to_string(test_array(i).NS_1) & ", Got NS_1 = " & to_string(CS_1)
                severity warning;
          end if;
          if (error_reporting = 2)
            Assert (not ((y'delayed(clk_period/2) = test_array(i).y) and (CS_0 = test_array(i).NS_0) and (CS_1 = test_array(i).NS_1)))
                report "Good product at line " & integer'image(i) 
                severity note;
            write(l, string'("")); writeline(OUTPUT, l); -- write empty line to the console to make it more readable
          end if;
          
          if ((y'delayed(clk_period/2) = test_array(i).y) and (CS_0 = test_array(i).NS_0) and (CS_1 = test_array(i).NS_1)) then
            good_lines := good_lines + 1;
          end if;
        end if;
        good_lines := good_lines;
      end loop;
      repeat := repeat + 1;
      run_time := now - start_time;
    end loop;
    
    done <= true;
    report "Testbench finished." & LF & " Result: Pass=" & to_string(good_lines) & "  Fail=" & to_string(test_array'length - good_lines) severity note;
    wait;
	
  end process stimulus;

end architecture arc_tb_universal;

