`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    12:14:34 07/07/2020 
// Design Name: 
// Module Name:    PWM 
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
module PWM #(parameter N=32)
(
    input [N-1:0] sig_in,
    input clk,
    input reset,
    output reg pwm_out
);

wire [N-1:0] TOP = 32'd500000;

reg [N-1:0] counter;

initial begin
	pwm_out <= 1'b0;
	counter <= 1'b0;
end

always @(posedge clk) 
    if (reset) begin
        counter <= 1'b0;
        pwm_out <= 1'b0;
    end
	else begin
		counter <= counter + 1;
		pwm_out <= (sig_in > counter) ? 1'b1 : 1'b0;
		
		if (counter == TOP) counter <= 1'b0;
    end
	
endmodule
