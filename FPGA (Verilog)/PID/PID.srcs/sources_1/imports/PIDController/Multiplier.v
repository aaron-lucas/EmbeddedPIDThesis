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
(multiplier ,multiplicand, result); 
	input [W-1:0]  multiplicand, multiplier;
	output        result;

	wire [2*W:0]   product;
	reg [W-1:0]   result;

    assign product = multiplicand * multiplier;
    

	wire [W:0]   multsx = {multiplicand[W-1],multiplicand};

endmodule
