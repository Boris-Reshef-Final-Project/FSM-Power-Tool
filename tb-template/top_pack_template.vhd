library ieee;
use ieee.std_logic_1164.all;

package top_pack_$ is
    type int_arr is array (natural range<>) of integer;
    constant c_polynom : int_arr := (?g);
    function LFSR (buff : std_logic_vector; polinom : int_arr) return std_logic_vector;
end package top_pack_$;


package body top_pack_$ is

    function LFSR (buff : std_logic_vector; polinom : int_arr) return std_logic_vector is
        variable or_gate, xor_gate, sin : std_logic :='0'; 
        begin
             for i in buff'range loop
                 or_gate:=or_gate or buff(i);
             end loop;
             for j in polinom'range loop
                 xor_gate:=xor_gate xor buff(polinom(j));
             end loop;
             sin:=xor_gate or (not or_gate);
             return (buff((buff'high-1) downto buff'low) & sin);
        end function LFSR;
    
end package body top_pack_$;