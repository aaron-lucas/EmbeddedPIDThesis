`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 30.10.2020 18:19:49
// Design Name: 
// Module Name: QuadratureDecoder_tb
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


module QuadratureDecoder_tb();
    reg clk;
    reg chA, chB;
    reg reset;
    wire [15:0] counter;
    wire direction;

    QuadratureDecoder #(16) uut (
        .clk(clk), 
        .chA(chA),
        .chB(chB),
        .reset(reset),
        .direction(direction),
        .counter(counter)
    );
    
    initial begin
        clk <= 0;
        chA <= 0;
        chB <= 0;
        reset <= 0;
    end
    
    always #10 clk <= ~clk;
    
    always begin
        #1
        chA <= ~chA;
        #99;
    end
    
    always begin
        #51;
        chB <= ~chB;
        #49;
    end
    
endmodule
