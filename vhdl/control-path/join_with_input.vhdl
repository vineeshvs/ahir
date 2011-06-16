library ieee;
use ieee.std_logic_1164.all;
library ahir;
use ahir.Types.all;
use ahir.subprograms.all;
use ahir.BaseComponents.all;

entity join_with_input is
  port ( preds      : in   BooleanArray;
    	symbol_in  : in   boolean;
    	symbol_out : out  boolean;
	clk: in std_logic;
	reset: in std_logic);
end join_with_input;

architecture default_arch of join_with_input is
  signal symbol_out_sig : BooleanArray(0 downto 0);
  signal place_sigs: BooleanArray(preds'range);
begin  -- default_arch
  
  placegen: for I in preds'range generate
    placeBlock: block
	signal place_pred: BooleanArray(0 downto 0);
    begin
	place_pred(0) <= preds(I);
	pI: place generic map(marking => false, bypass => true)
		port map(place_pred,symbol_out_sig,place_sigs(I),clk,reset);
    end block;
  end generate placegen;
  -- The transition is enabled only when all preds are true.
  
  symbol_out_sig(0) <= symbol_in and AndReduce(place_sigs);
  symbol_out <= symbol_out_sig(0);
end default_arch;
