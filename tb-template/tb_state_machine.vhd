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
    signal rst      : std_logic := '1';
    signal clk      : std_logic_vector(num_clocks  - 1  downto 0) := (others => '0');
    signal x        : std_logic_vector(num_inputs  - 1  downto 0) := (others => '0');
    signal y        : std_logic_vector(num_outputs - 1  downto 0);
    signal z        : std_logic_vector(num_clocks  - 1  downto 0);

    file kis_src    : text;
    variable prod   : line;
    variable f_stat : file_open_status;
    variable st     : state := st0;

    -- Skip white spaces in line, spaces and tabs(HT) and returns the new position
    procedure skip_white(variable L : inout line; pos : inout integer) is
        variable old_L : line := L;
     begin
     while pos <= old_L'high loop
         case old_L(pos) is
         when ' ' | HT  =>
             pos := pos + 1;
         when others =>
             exit;
         end case;
     end loop;
         L := new string'(old_L(pos to old_L'high));
         deallocate(old_L);
     end procedure skip_white;


     -- Procedure to verify the output vs. the .kis file
    procedure check_output(variable x : in std_logic_vector; variable st: inout state; variable DUT_y : in std_logic_vector; variable result : out boolean) is

        -- considerations: state naming (if applicable), reset state (if applicable), output dont-care fixed to '0'

    end procedure check_output;

begin

    -- Open the file
    file_open(f_stat, kis_src, kis_src_path, read_mode);
    -- check if the file is open
    assert (f_stat = open_ok) 
        report "Source file reading failure " & time'image(now) severity failure;
    report "Source .kis file successfully opened";

    -- Instantiate the device under test (DUT)
    DUT : entity work.srcname
        port map (rst => rst, clk => clk, x => x, y => y, z => z);

    -- Generate clock(s) with enable
    -- This is going to be replaced with the PLL in the physical tests.
    clk <= (z xor clk) after (clk_period/2);

    -- Test process
    stinulus: process
    begin
    
        rst <= '0' after 2*clk_period; -- Release reset after 2cc

        -- Start applying input stimulus and check results against the .kis file with asserts using the check_output procedure
        -- for input stimulus, find a way to randomlize the inputs


        wait;
    end process;

end architecture arc_tb_universal;