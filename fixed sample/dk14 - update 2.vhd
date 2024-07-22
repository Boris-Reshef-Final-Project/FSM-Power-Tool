-- This file was fixed formally and this should work for what we need + synthesis
-- We're taking the risk of having glitches here (b/c of clock timing for wait states)
-- but it's a trade worth having instead of having worse tpd.

-- Need to do a double over this file to make sure everything works as expected.

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity dk14 is
port(
	rst		: in	std_logic;
	clk		: in	std_logic_vector(1 downto 0);
	x		: in	std_logic_vector(2 downto 0);
	y		: out	std_logic_vector(4 downto 0);
	clken : out std_logic_vector(1 downto 0)
);
end entity dk14;

architecture arc_state_machine of dk14 is
	type state_0 is (st0, st1, st2,st3_wait);
	type state_1 is (st3, st4, st5, st6,st7_wait);
	signal s0 : state_0;
	signal s1 : state_1;
	shared variable y1,y2: std_logic_vector(y'range);
	shared variable z1,Z2,z3,z4,z5,z6: std_logic;
begin
	cfsm0: process(clk(0), rst) begin
		if(rst = '1') then
			s0	<=	st0;
		elsif rising_edge(clk(0)) then
		    z1:='0'; z2:='0'; z3:='0';
			case s0 is
				when st0 =>
					case? x is
						when "000" =>
							s0 <= st1;
							y1 := "00010";
						when "100" =>
							s0 <= st3_wait;
							y1 := "00010";
							z1:= '1';
						when "111" =>
							s0 <= st2;
							y1 := "01010";
						when "110" =>
							s0 <= st3_wait;
							y1 := "01010";
							z1:= '1';
						when "011" =>
							s0 <= st2;
							y1 := "01000";
						when "001" =>
							s0 <= st3_wait;
							y1 := "00010";
							z2:= '1';
						when "101" =>
							s0 <= st3_wait;
							y1 := "01010";
							z2:= '1';
						when "010" =>
							s0 <= st3_wait;
							y1 := "01000";
							z3:= '1';
						when others => NULL;
					end case?;
				when st1 =>
					case? x is
						when "000" =>
							s0 <= st0;
							y1 := "01001";
						when "100" =>
							s0 <= st1;
							y1 := "01001";
						when "111" =>
							s0 <= st2;
							y1 := "00100";
						when "110" =>
							s0 <= st3_wait;
							y1 := "00100";
							z2:= '1';
						when "011" =>
							s0 <= st1;
							y1 := "00101";
						when "001" =>
							s0 <= st0;
							y1 := "00101";
						when "101" =>
							s0 <= st0;
							y1 := "00001";
						when "010" =>
							s0 <= st1;
							y1 := "00001";
						when others => NULL;
					end case?;
				when st2 =>
					case? x is
						when "000" =>
							s0 <= st2;
							y1 := "10010";
						when "100" =>
							s0 <= st3_wait;
							y1 := "10010";
							z1:= '1';
						when "111" =>
							s0 <= st2;
							y1 := "01010";
						when "110" =>
							s0 <= st3_wait;
							y1 := "01010";
							z1:= '1';
						when "011" =>
							s0 <= st2;
							y1 := "01000";
						when "001" =>
							s0 <= st3_wait;
							y1 := "10010";
							z2:= '1';
						when "101" =>
							s0 <= st3_wait;
							y1 := "01010";
							z2:= '1';
						when "010" =>
							s0 <= st3_wait;
							y1 := "01000";
							z3:= '1';
						when others => NULL;
					end case?;
				when st3_wait => if(z4='1') then
									s0<=st2;
								elsif (z5='1')	then
									s0<=st0;
								elsif (z6='1')	then
									s0<=st1;
								end if;	
				when others => NULL;
			end case;
		end if;
	end process cfsm0;

	cfsm1: process(clk(1), rst) begin
		if(rst = '1') then
			s1	<=	st3;
		elsif rising_edge(clk(1)) then
		   z4 := '0'; z5 := '0'; z6 := '0';
			case s1 is
				when st3 =>
					case? x is
						when "000" =>
							s1 <= st7_wait;
							y2:= "00010";
							z4 := '1';
						when "100" =>
							s1 <= st3;
							y2:= "00010";
						when "111" =>
							s1 <= st7_wait;
							y2:= "00100";
							z4 := '1';
						when "110" =>
							s1 <= st4;
							y2:= "00100";
						when "011" =>
							s1 <= st7_wait;
							y2:= "10100";
							z4 := '1';
						when "001" =>
							s1 <= st4;
							y2:= "00010";
						when "101" =>
							s1 <= st4;
							y2:= "10100";
						when "010" =>
							s1 <= st6;
							y2:= "10000";
						when others => NULL;
					end case?;
				when st4 =>
					case? x is
						when "000" =>
							s1 <= st7_wait;
							y2:= "01001";
							z5 := '1';
						when "100" =>
							s1 <= st7_wait;
							y2:= "01001";
							z6 := '1';
						when "111" =>
							s1 <= st7_wait;
							y2:= "10001";
							z5 := '1';
						when "110" =>
							s1 <= st7_wait;
							y2:= "10101";
							z5 := '1';
						when "011" =>
							s1 <= st7_wait;
							y2:= "00101";
							z6:= '1';
						when "001" =>
							s1 <= st7_wait;
							y2:= "00101";
							z5 := '1';
						when "101" =>
							s1 <= st7_wait;
							y2:= "10001";
							z6 := '1';
						when "010" =>
							s1 <= st7_wait;
							y2:= "10101";
							z6 := '1';
						when others => NULL;
					end case?;
				when st5 =>
					case? x is
						when "000" =>
							s1 <= st7_wait;
							y2:= "01001";
							z5 := '1';
						when "100" =>
							s1 <= st7_wait;
							y2:= "01001";
							z6 := '1';
						when "111" =>
							s1 <= st7_wait;
							y2:= "10001";
							z5 := '1';
						when "110" =>
							s1 <= st7_wait;
							y2:= "10101";
							z5 := '1';
						when "011" =>
							s1 <= st7_wait;
							y2:= "10100";
							z4 := '1';
						when "001" =>
							s1 <= st4;
							y2:= "10100";
						when "101" =>
							s1 <= st7_wait;
							y2:= "10001";
							z6 := '1';
						when "010" =>
							s1 <= st7_wait;
							y2:= "10101";
							z6:= '1';
						when others => NULL;
					end case?;
				when st6 =>
					case? x is
						when "000" =>
							s1 <= st7_wait;
							y2:= "10010";
							z4 := '1';
						when "100" =>
							s1 <= st3;
							y2:= "10010";
						when "111" =>
							s1 <= st7_wait;
							y2:= "10001";
							z5 := '1';
						when "110" =>
							s1 <= st7_wait;
							y2:= "10101";
							z5 := '1';
						when "011" =>
							s1 <= st7_wait;
							y2:= "10100";
							z4 := '1';
						when "001" =>
							s1 <= st4;
							y2:= "10010";
						when "101" =>
							s1 <= st7_wait;
							y2:= "10001";
							z6 := '1';
						when "010" =>
							s1 <= st7_wait;
							y2:= "10101";
							z6 := '1';
						when others => NULL;
					end case?;
                   when st7_wait=> if(z1='1') then
										s1<=st3;
									elsif (z2='1')	then
										s1<=st4;
									elsif (z3='1')	then
										s1<=st5;
									end if;		
				when others => NULL;
			end case;
		end if;
	end process cfsm1;
	
	y<=y1 or y2;
	clken(0) <= z4 or z5 or z6;
	clken(1) <= z1 or z2 or z3;
	
end arc_state_machine;
