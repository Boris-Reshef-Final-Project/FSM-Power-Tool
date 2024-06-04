library ieee;
use ieee.std_logic_1164.all;

package tb_package_$ is

  -- constants and types
  constant num_inputs   : positive  := ?x; -- number of inputs -1
  constant num_outputs  : positive  := ?y; -- number of outputs -1
  constant num_clocks   : positive  := ?c; -- number of clocks -1
  constant clk_period   : time      := ?t;
  -- state type
  ?s

  type FSMproduct is record
    x  : std_logic_vector(num_inputs  downto 0);
    CS : state;
    NS : state;
    y  : std_logic_vector(num_outputs downto 0);
  end record;

  type FSMproduct_array is array(natural range <>) of FSMproduct;
  
  constant test_array : FSMproduct_array(?p downto 0) := (
    ?q
  );
  

end package tb_package_$;


package body tb_package_$ is
end package body tb_package_$;