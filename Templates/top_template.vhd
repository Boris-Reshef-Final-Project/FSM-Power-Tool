Library IEEE;
use ieee.std_logic_1164.all;
library work;
use work.all;
use work.pack_top_$.all;
library altera; 
use altera.altera_primitives_components.all;
library altera_mf;
use altera_mf.altera_mf_components.all;
library cycloneive;
use cycloneive.all;
use altera.altera_syn_attributes.all;

entity top_$ is
    generic (
        simulation      : boolean := false;
        power_analyzer  : boolean := true;
        baseline_power  : boolean := false;
        full_fpga       : boolean := false;
        duplicates      : integer := 100
    );
    port(
        rst	    : in	std_logic := '1';
        clk	    : in    std_logic_vector(?c downto 0) := (others => '0'); -- Read COMMENT1 below.
        x		: in	std_logic_vector(?x downto 0) := (others => '0');
        y		: out	std_logic_vector(?y downto 0);
        y_2     : out	std_logic_vector(?y downto 0);
        clken_2 : out	std_logic_vector(?c downto 0)
    );
    -- COMMENT1: The clk input is should be only 1-bit wide, but in-order 
    -- to prevent the Quartus fitter from optimizing away one of the PLLs
    -- (due to the "Auto merge PLLs" option), we need to pass the (same) clk signal
    -- as a vector of ports of the same size as the number of PLLs. This way, the fitter
    -- will not merge the PLLs, and we will have the desired number of PLLs on the chip.

end entity top_$;
 
    
architecture arc_top of top_$ is
    
        signal clken, clk_src, fsm_clk, pll_out, s	: std_logic_vector(clk'range)   := (others => '0');
        signal fsm_input, LFSR_out                  : std_logic_vector(x'range)     := (others => '0');
		  
		component $ is
		generic(a :integer := 0);
			port(
			rst		: in	std_logic;
			clk		: in	std_logic_vector(?c downto 0);
			x		: in	std_logic_vector(?x downto 0);
			y		: out	std_logic_vector(?y downto 0);
			clken	: out	std_logic_vector(?c downto 0)
			);
		end component $;
		  
		signal original_y	    : std_logic_vector(y'range);
		signal dupes_y 		    : std_vec_array(duplicates-1 downto 0)(y'range) := (others => (others => '0'));
        signal original_clken	: std_logic_vector(clken'range);
        signal dupes_clken	    : std_vec_array(duplicates-1 downto 0)(clk'range) := (others => (others => '0'));

		attribute keep 	            of dupes_y  : signal is true;
		attribute altera_attribute  of G2       : label  is "-name PRESERVE_REGISTER on";
		attribute dont_merge        of G2       : label  is true;
        
    begin
        y       <= original_y;
		y_2     <= original_y       or or_array(dupes_y);
        clken   <= original_clken;
        clken_2 <= original_clken   or or_array(dupes_clken);
		
        -- In software: input = x, clk = clk
        -- In hardware: input = LFSR_out, clk = pll_out
        fsm_input <= LFSR_out when (baseline_power or full_fpga) else x;
        clk_src   <= clk when (not full_fpga) else pll_out;

        G0: if (not baseline_power) generate
            ena_unit: for i in clk'range generate
                process(clk_src(i), clken(i)) is
                begin
                    if (clk_src(i) = '0') then
                        s(i) <= clken(i);
                    end if;
                end process;
                fsm_clk(i) <= clk_src(i) and s(i);
            end generate ena_unit;
        end generate G0;


        -- generate 1 FSM
        -- Used in: simulation, power analyzer, full_fpga
        G1: if (not baseline_power) generate
            FSM: $
			generic map (a => -1)
            port map(
                rst	    => rst,
                clk	    => fsm_clk,
                x		=> fsm_input,
                y		=> original_y,
                clken	=> original_clken
                );
        end generate G1;


        -- generate many FSMs to fill the area on the chip to about 30%
        -- change the value of 'duplicates' (generic) to control the amount of FSMs
        -- Used in: power_analyzer, full_fpga
        G2: if (not baseline_power) generate
            FSM_DUPES: for i in 0 to (duplicates - 1) generate
                FSM_area_fill: $
                generic map (a => i)
				port map(
					rst	    => rst,
					clk	    => fsm_clk,
					x		=> fsm_input,
					y		=> dupes_y(i),
					clken	=> dupes_clken(i)
					);
            end generate FSM_DUPES;
        end generate G2;


        -- generate PLLs
        -- Used in: simulation, power_analyzer, full_fpga
        G3: if full_fpga generate
            PLL_LOOP: for i in clk'range generate
                PLL: entity work.PLL_altpll PORT MAP (
                    inclk0  => clk(i),
                    c0      => pll_out(i)
                );
            end generate PLL_LOOP;
        end generate G3;


        -- generate LFSR
        -- Used in: baseline_power, full_fpga
        GEN_LFSR: if (baseline_power or full_fpga) generate
            process(clk, rst) is
            begin
                if (rst = '1') then
                    LFSR_out <= (others => '1');
                elsif rising_edge(clk(0)) then
                    LFSR_out <= LFSR(LFSR_out, c_polynom);
                end if;
            end process;
        end generate GEN_LFSR;
         
end architecture arc_top;
