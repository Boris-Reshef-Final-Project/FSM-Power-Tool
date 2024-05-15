----------------------------------------------------------------------------
-- File: SynthCase2008.vhd
-- Author: Reshef Schachter
-- Date: 14/05/2024
-- Description: Verifiying synthesisablity of "case?" statement in VHDL.
--              This code was made to test if the method we've devised
--              for the parser as part of our Bachelors final project is
--              synthesisable and viable.
--              Please notice that "case?" is not the same as "case".
--              
----------------------------------------------------------------------------
----------------------------------------------------------------------------
-------THIS WAS MADE TO WORK ONLY WITH A COMPILER FOR VHDL 2008!!-----------
----------------------------------------------------------------------------
----------------------------------------------------------------------------
--
-- We have verified using Quartus Prime Lite 20.1 and Modelsim 2020.1
-- that this code is synthesisable and works as expected.
-- the "case" statement (without the "?") interprets "don't care" as a
-- value and not as either '0' or '1' (wildcard value) as we need it to be.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity SynthCase2008 is
  port
  (
    clk    : in std_logic;
    rst    : in std_logic;
    ctrl   : in std_logic_vector(3 downto 0);
    data_in: in std_logic_vector(23 downto 0);
    result : out std_logic_vector(3 downto 0)
  );
end entity SynthCase2008;

architecture arc_isSynth of SynthCase2008 is
begin
  process (clk, rst) begin

    if rst = '1' then
      result <= (others => '0');

    elsif rising_edge(clk) then

      case? ctrl is
        when "0---" => result <= data_in(23 downto 20);
        when "101-" => result <= data_in(19 downto 16);
        when "11-0" => result <= data_in(15 downto 12);
        when "1101" => result <= data_in(11 downto 8);
        when "1111" => result <= data_in(7 downto 4);
        when others => result <= data_in(3 downto 0);
        -- when "0---" => result <= x"A";
        -- when "101-" => result <= x"B";
        -- when "11-0" => result <= x"C";
        -- when "1101" => result <= x"D";
        -- when "1111" => result <= x"E";
        -- when others => result <= x"F";
      end case?;

    end if;
  end process;
end architecture arc_isSynth;