module top(
	input wire CLOCK_50,
	input wire [0:0] KEY
);
	system nios_system(
		.clk_clk (CLOCK_50),
		.reset_reset_n (KEY[0])
	);
endmodule