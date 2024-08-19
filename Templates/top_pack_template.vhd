library ieee;
use ieee.std_logic_1164.all;

package pack_top_$ is
    type int_arr is array (natural range<>) of integer;
    constant c_polynom : int_arr := (?g);
    function LFSR (buff : std_logic_vector; polynom : int_arr) return std_logic_vector;
end package pack_top_$;


package body pack_top_$ is

    function LFSR (buff : std_logic_vector; polynom : int_arr) return std_logic_vector is
        variable or_gate, xor_gate, s_in : std_logic :='0';
        begin
             for i in buff'range loop
                 or_gate := or_gate or buff(i);
             end loop;
             for j in polynom'range loop
                 xor_gate := xor_gate xor buff(polynom(j));
             end loop;
             s_in := xor_gate or (not or_gate);
             return (buff((buff'high-1) downto buff'low) & s_in);
        end function LFSR;
    
end package body pack_top_$;