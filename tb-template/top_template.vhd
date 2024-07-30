Library IEEE;
use ieee.std_logic_1164.all;
library work;
use work.all;
use work.top_pack_$.all;
library altera; 
use altera.altera_primitives_components.all;
library altera_mf;
use altera_mf.altera_mf_components.all;
library cycloneive_atoms;
use cycloneive_atoms.all;
library cycloneive_components;
use cycloneive_components.all;

entity top_$ is
    generic (
        simulation_mode : boolean := false;
        power_analysis  : boolean := true;
        baseline_power  : boolean := false;
        full_fpga       : boolean := false;
        duplicates      : integer := 100
    );
    port(
        rst	    : in	std_logic := '1';
        inclk	: in    std_logic;
        x		: in	std_logic_vector(?x downto 0) := (others => '0');
        y		: out	std_logic_vector(?y downto 0) := (others => '0')	
    );
    end entity top_$;
 
    
architecture arc_top of top_$ is
    
        signal clken, clk_out, S, PLL_clk	: std_logic_vector(?c downto 0) := "01";
        signal LFSR_out : std_logic_vector(x'range) := (others => '1');
    
    begin

        G_FSM: if simulation_mode generate
            FSM: entity work.$
            port map(
                rst	    => rst,
                clk	    => clk_out,
                x		=> x,
                y		=> y,
                clken	=> clken
                );

                process(PLL_clk(0), clken) is
                begin
                    if (PLL_clk(0)='0') then
                        S(0) <= clken(0);
                    end if;
                end process;
                process(PLL_clk(1), clken) is
                begin
                    if (PLL_clk(1)='0') then
                        S(1) <= clken(1);
                    end if;
                end process;
                clk_out <= PLL_clk and S;

                PLL_altpll_A : PLL_altpll PORT MAP (
                    inclk0	 => inclk,
                    c0	 => PLL_clk(0)
                );
                
                PLL_altpll_B : PLL_altpll PORT MAP (
                    inclk0	 => inclk,
                    c0	 => PLL_clk(1)
                );

        end generate G_FSM;


        G_MANY_FSMS: if full_fpga generate

            FSM_main: entity work.$
            port map(
                rst	    => rst,
                clk	    => clk_out,
                x		=> LFSR_out,
                y		=> y,
                clken	=> clken
                );

            G_FSM_LOOP1: for i in 0 to duplicates generate
                FSM_area_fill: entity work.$
                port map(
                    rst	    => rst,
                    clk	    => clk_out,
                    x		=> LFSR_out,
                    y		=> open,
                    clken	=> open
                    );
            end generate G_FSM_LOOP1;
        end generate G_MANY_FSMS;

        G_PWR_ANALYSIS: if power_analysis generate

            FSM_main: entity work.$
            port map(
                rst	    => rst,
                clk	    => clk_out,
                x		=> x,
                y		=> y,
                clken	=> clken
                );

            G_FSM_LOOP2: for i in 0 to duplicates generate
                FSM_area_fill: entity work.$
                port map(
                    rst	    => rst,
                    clk	    => clk_out,
                    x		=> x,
                    y		=> open,
                    clken	=> open
                    );
            end generate G_FSM_LOOP2;
        end generate G_PWR_ANALYSIS;

        
        G_PLL: if not baseline_power generate

            process(PLL_clk(0), clken) is
            begin
                if (PLL_clk(0)='0') then
                    S(0) <= clken(0);
                end if;
            end process;
            process(PLL_clk(1), clken) is
            begin
                if (PLL_clk(1)='0') then
                    S(1) <= clken(1);
                end if;
            end process;
            clk_out <= PLL_clk and S;

            PLL_altpll_A : PLL_altpll PORT MAP (
                inclk0	 => inclk,
                c0	 => PLL_clk(0)
            );
            
            PLL_altpll_B : PLL_altpll PORT MAP (
                inclk0	 => inclk,
                c0	 => PLL_clk(1)
            );

        end generate G_PLL;


        G_BASELINE: if (baseline_power or full_fpga) generate
            LFSR_out <= LFSR(LFSR_out, c_polynom);
            y <= LFSR_out;
        end generate G_BASELINE;
         
end architecture arc_top;