library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_textio.all;
use std.textio.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

library work;
use work.tb_package_$.all; -- Use the package

entity tb_$ is
end entity tb_$;

architecture arc_tb_universal of tb_$ is
  signal rst : std_logic := '1';
  signal clk : std_logic_vector(num_clocks  - 1 downto 0) := (others => '0');
  signal x   : std_logic_vector(num_inputs  downto 0)     := (others => 'U');
  signal y   : std_logic_vector(num_outputs downto 0);
  signal z   : std_logic_vector(num_clocks  - 1 downto 0) := (0 => '1', others => '0');

  
  -- Procedure to verify the output vs. the test array
  procedure check_output(variable x : in std_logic_vector; variable st : inout state; variable DUT_y : in std_logic_vector; variable result : out boolean) is
  begin
    -- considerations: state naming (if applicable), reset state (if applicable), output dont-care fixed to '0'

  end procedure check_output;
  
begin

  -- Instantiate the device under test (DUT)
  DUT : entity work.$
    port map
      (rst => rst, clk => clk, x => x, y => y, z => z);

  -- Generate clock(s) with enable
  -- This is going to be replaced with the PLL in the physical tests.
  clk <= (z xor clk) after (clk_period/2);

  -- Test process
  stinulus : process
  begin

    rst <= '0' after 2 * clk_period; -- Release reset after 2cc

    -- Start applying input stimulus and check results against the .kis file with asserts using the check_output procedure
    -- for input stimulus, find a way to randomlize the inputs
    -- for output, use the check_output procedure to compare the output with the .kis file

    wait;
  end process;

end architecture arc_tb_universal;

