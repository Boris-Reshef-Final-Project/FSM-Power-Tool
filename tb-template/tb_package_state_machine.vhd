/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
File:           tb_package_state_machine.vhd
Authors:        Boris Karasov, Reshef Schachter
Date:           2023-2024
Institution:    Afeka College of Engineering
Description:    This is our final project for our Bachelor's degree in Electrical Engineering.
Notes:          The project is meant to work with a VHDL2008 compiler and a C++17 compiler.
File Description: This file is the testbench package, and should be used with the main test bench file.
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------**/


-- THIS IS FOR THE OPTIMIZED VERSION!!!!


library ieee;
use ieee.std_logic_1164.all;

package tb_package_$ is

  -- constants and types
  constant num_inputs   : positive  := ?x; -- number of inputs -1
  constant num_outputs  : positive  := ?y; -- number of outputs -1
  constant num_clocks   : positive  := ?c; -- number of clocks -1
  constant clk_period   : time      := ?t; -- clock period, usually 10 ns
  -- state type
  ?s

  type FSMproduct is record
    x  : std_logic_vector(num_inputs  downto 0);
    CS_0 : state_0;
	  CS_1 : state_1;
    NS_0 : state_0;
	  NS_1 : state_1;
	  C_fsm : integer;
	  N_fsm : integer;
    y  : std_logic_vector(num_outputs downto 0);
  end record;

  type FSMproduct_array is array(natural range <>) of FSMproduct;
  
  constant test_array : FSMproduct_array(?p downto 0) := (
    ?q
  );
  

end package tb_package_$;


package body tb_package_$ is
end package body tb_package_$;