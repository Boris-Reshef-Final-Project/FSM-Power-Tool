-- Testbench for SynthCase2008.vhd

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_synthcase2008 is
end entity tb_synthcase2008;

architecture tb_arch of tb_synthcase2008 is

    signal clk      : std_logic := '0';
    signal rst      : std_logic := '1';
    signal ctrl     : std_logic_vector(3 downto 0) := x"0";
    signal data_in  : std_logic_vector(23 downto 0) := x"ABCDEF";
    signal result   : std_logic_vector(3 downto 0);

    type Array_Type is array (natural range <>) of std_logic_vector(3 downto 0);
    constant values : Array_Type(0 to 15) := (
        x"a", x"a", x"a", x"a", x"a", x"a", x"a", x"a",
        x"f", x"f", x"b", x"b", x"c", x"d", x"c", x"e"
    );

begin

    -- Clock generation
    clk <= (not clk) after 50 ps;

    -- Instantiate the Device Under Test (DUT)
    DUT: entity work.SynthCase2008
    port map(clk => clk, rst => rst, ctrl => ctrl, data_in => data_in, result => result);

    -- Stimulus process
    stimulus_process : process
    begin
        -- Initial signal simulation states
        rst <= '1';
        wait for 200 ps;
        rst <= '0';
        wait for 250 ps;

        -- Increment ctrl from x"0" to x"F"
        for i in 0 to 15 loop
            ctrl <= std_logic_vector(to_unsigned(i, ctrl'length));
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            assert result = values(i) report "Mismatch at index " & integer'image(i) severity error;
        end loop;

        -- End simulation
        wait for 1 ns;
        report "Simulation finished. Rejoice! :)" severity note;
        wait;
    end process stimulus_process;


end architecture tb_arch;