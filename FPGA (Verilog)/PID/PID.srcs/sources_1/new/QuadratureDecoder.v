`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 29.10.2020 20:06:42
// Design Name: 
// Module Name: QuadratureDecoder
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

// Modified from https://www.fpga4fun.com/QuadratureDecoder.html

module QuadratureDecoder #(parameter W=32) (
    input clk,
    input chA,
    input chB,
    input reset,
    input update,
    output reg direction,
    output reg [W-1:0] counter
    );
    
    reg [2:0] prevChA;
    reg [2:0] prevChB;
    
    reg [W-1:0] tickCount;
    
//    reg updateFlag;
    
    wire dir         = prevChA[1] ^ prevChB[2];
    wire countEnable = prevChA[2] ^ prevChA[1] ^ prevChB[2] ^ prevChB[1];
    
    initial begin
        tickCount   <= 1'b0;
//        updateFlag  <= 1'b0;
        counter     <= 0;
    end
    
    always @(posedge clk)
        if (reset) begin
            counter     <= 1'b0;
            tickCount   <= 1'b0;
        end
        else begin
            prevChA     <= {prevChA[1:0], chA};
            prevChB     <= {prevChB[1:0], chB};
            
            if (update) begin
                counter     <= tickCount;
                tickCount   <= 1'b0;
                direction   <= dir;
//                updateFlag  <= 1'b0;
            end
            else if (countEnable)
                if (dir) tickCount <= tickCount + 32'h0001_0000;
                else     tickCount <= tickCount - 32'h0001_0000;
            
        end
    
//    always @(posedge update) begin
//        direction   <= dir;
//        updateFlag  <= 1'b1;
//    end
        
endmodule
