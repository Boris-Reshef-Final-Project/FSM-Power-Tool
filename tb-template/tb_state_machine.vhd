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
  signal clk : std_logic_vector(num_clocks - 1 downto 0) := (others => '0');
  signal x   : std_logic_vector(num_inputs - 1 downto 0) := (others => 'U');
  signal y   : std_logic_vector(num_outputs - 1 downto 0);
  signal z   : std_logic_vector(num_clocks - 1 downto 0) := (0 => '1', others => '0');

  -- Skip white spaces in line, spaces and tabs(HT) and returns the new position
  procedure skip_white(variable L : inout line; pos : inout integer) is
    variable old_L : line := L;
  begin
    while pos <= old_L'high loop
      case old_L(pos) is
        when ' ' | HT =>
          pos := pos + 1;
        when others =>
          exit;
      end case;
    end loop;
    L := new string'(old_L(pos to old_L'high));
    deallocate(old_L);
  end procedure skip_white;

  -- Skip lines in the .kis file
  procedure skip_lines(variable file_handle : in text; variable L : inout line) is
  begin
    while not endfile(file_handle) loop
      readline(file_handle, line);
      if L'length > 0 and L(1) /= '.' then
        exit;
      end if;
    end loop;
  end procedure skip_lines;
  
  

  -- Procedure to verify the output vs. the .kis file
  procedure check_output(variable x : in std_logic_vector; variable st : inout state; variable DUT_y : in std_logic_vector; variable result : out boolean) is
  begin
    -- considerations: state naming (if applicable), reset state (if applicable), output dont-care fixed to '0'

  end procedure check_output;

  
    -- Procedure to parse a line and extract 4 values
    procedure parse_line(variable L : in line; variable x : out std_logic_vector; variable CS : out state; variable NS : out state; variable y : out std_logic_vector) is
        variable words : array(0 to 3) of string(1 to 100);
        variable pos : integer := 1;
    begin
        -- Split the line into words
        for i in 0 to 3 loop
            skip_white(L, pos);
            while pos <= L'length and L(pos) /= ' ' loop
                words(i)(words(i)'length + 1) := L(pos);
                pos := pos + 1;
            end loop;
        end loop;

        -- Convert the words to the desired types
        x := std_logic_vector((to_stdlogicvector(words(0))), x'length);
        CS := to_state(words(1));
        NS := to_state(words(2));
        y := std_logic_vector(to_unsigned(to_integer(unsigned(to_stdlogicvector(words(3)))), y'length));
    end procedure parse_line;
  
begin
    
  file_read : process
    file kis_src    : text;
    variable prod   : line;
    variable f_stat : file_open_status;
    variable CS     : state;
    variable NS     : state;
  begin
    -- Open the file
    file_open(f_stat, kis_src, kis_src_path, read_mode);
    -- check if the file is open
    assert (f_stat = open_ok)
    report "Source file reading failure " & time'image(now) severity failure;
    report "Source .kis file opened successfully";
    skip_lines(kis_src, prod);
    loop -- Read the .kis file
      readline(kis_src, prod);
      -- Skip white spaces
      skip_white(prod, 1);
      -- Check if the line is a comment
      if prod(1) = '.' then
        skip_lines(kis_src, prod);
        exit;
      end if;
      -- Read the value of x,CS,NS,y from the .kis file
      -- 
      --x <=
      --wait until falling_edge(clk);
      --
      wait;
    end loop;
    -- Read the value of x,CS,NS,y from the .kis file
    -- 
    --x <=
    --wait until falling_edge(clk);
    --assert(y = DUT_y) report "Output mismatch" severity failure;
    --assert(spyCS = NS) report "State mismatch" severity failure;
    wait;
  end process file_read;

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

