library ieee;
use ieee.std_logic_1164.all;

package srcname_tb_pack is

  -- constants and types
  constant num_inputs   : positive  := ~;
  constant num_outputs  : positive  := ~;
  constant num_clocks   : positive  := ~;
  constant num_resets   : positive  := ~;
  constant clk_freq     : positive  := ~; -- in MHz  
  constant clk_period   : time      := ~ ns;
  constant replacer     : std_logic := '0'; -- used to replace '-' in y values
  type state is (~);
    
  -- file names and locations
  constant kis_src_path : string    := ~; -- file path of source .kis file
  
  --functions
  function change_dontc_outputs (arg : std_logic_vector) return std_logic_vector;

end package srcname_tb_pack;


package body srcname_tb_pack is

  -- Alter a .kis vector for result asserting in the TB
  -- turn a std_logic_vector into a std_logic_vector with '-' replaced by replacer (usually '0')
  function change_dontc_outputs (arg : std_logic_vector) return std_logic_vector is
    variable result : std_logic_vector(arg'range);
    begin
      for i in arg'range loop
        result(i) <= replacer when (arg(i) = '-') else arg(i);
      end loop;
      return result;
  end function change_dontc_outputs;

end package body srcname_tb_pack;