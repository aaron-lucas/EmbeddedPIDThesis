`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02.11.2020 16:26:13
// Design Name: 
// Module Name: System_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module System_tb();

reg clk;
reg reset;
reg chA;
reg chB;
wire ctrlPin = 1'b0;
wire pwm;
wire timPin;

System uut(
    .clk(clk),
    .resetBtn(reset),
    .chAPin(chA),
    .chBPin(chB),
    .controlPin(ctrlPin),
    .pwmPin(pwm),
    .timingPin(timPin)
);

initial begin
    clk     <= 1'b0;
    reset   <= 1'b0;
    chA     <= 1'b0;
    chB     <= 1'b0;
end

always begin
    #5;
    clk <= ~clk;
end

always begin
    #1000000;
    chA <= ~chA;
    #1000000;
end

always begin
    #2000000;
    chB <= ~chB;
end


endmodule
