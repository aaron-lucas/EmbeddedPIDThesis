`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    15:34:30 07/05/2020 
// Design Name: 
// Module Name:    Multiplier 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////

// Modified from http://www.ece.lsu.edu/ee3755/2002/l07.html

module Multiplier #(parameter W=32, parameter Q=16)
(multiplier ,multiplicand ,clk, reset, ready, start, result); 
	input [W-1:0]  multiplicand, multiplier;
	input         start, clk, reset;
	output        result;
	output        ready;

	reg [2*W:0]   product;
	reg [W-1:0]   result;

	reg [5:0]     bit; 						// [log2(W):0]
	wire          ready = !bit;
	reg           lostbit;
	   
	initial bit = 0;

	wire [W:0]   multsx = {multiplicand[W-1],multiplicand};

	always @(posedge clk) begin
		if (reset) begin
			bit = 0;
			product = 0;
		end

		if( ready && start && ~reset ) begin
			bit     = 16;						//  W/2
			product = { 33'd0, multiplier }; 	// (W+1)'d0
			lostbit = 0;
		end else if( bit ) begin:A
			case ( {product[1:0],lostbit} )
				3'b001: product[2*W:W] = product[2*W:W] + multsx;
				3'b010: product[2*W:W] = product[2*W:W] + multsx;
				3'b011: product[2*W:W] = product[2*W:W] + 2 * multiplicand;
				3'b100: product[2*W:W] = product[2*W:W] - 2 * multiplicand;
				3'b101: product[2*W:W] = product[2*W:W] - multsx;
				3'b110: product[2*W:W] = product[2*W:W] - multsx;
			endcase

			lostbit = product[1];
			product = { product[2*W], product[2*W], product[2*W:2] };
			bit     = bit - 1;
			
			if (bit == 0) result <= product[W-1+Q:Q];

		end
	end

endmodule
