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
        y		: out	std_logic_vector(?y downto 0)	
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
        
    begin

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
            FSM: entity work.$
            port map(
                rst	    => rst,
                clk	    => fsm_clk,
                x		=> fsm_input,
                y		=> y,
                clken	=> clken
                );
        end generate G1;


        -- generate many FSMs to fill the area on the chip to about 30%
        -- change the value of 'duplicates' (generic) to control the amount of FSMs
        -- Used in: power_analyzer, full_fpga
        G2: if (full_fpga or power_analyzer) generate
            FSM_DUPES: for i in 0 to (duplicates - 1) generate
                FSM_area_fill: entity work.$
                    port map(
                        rst	    => rst,
                        clk	    => fsm_clk,
                        x		=> fsm_input,
                        y		=> open,
                        clken	=> open
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
                -- Prevent the LFSR from being optimized away in baseline_power mode
                if (baseline_power) then
                    y(x'range) <= fsm_input;
                end if;
            end process;
        end generate GEN_LFSR;
         
end architecture arc_top;