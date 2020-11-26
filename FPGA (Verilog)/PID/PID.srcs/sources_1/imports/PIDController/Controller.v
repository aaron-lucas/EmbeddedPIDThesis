`timescale 1ns / 1ps

// Project: FPGA PID Controller
// Module: PID Controller Module
// Author: Aaron Lucas

module Controller #(parameter W=32, parameter Q=16)  // W = Number bit width, Q = Number of fractional bits
(
	input clk,
	input reset,
	input start,
    input signed [W-1:0] setpoint,
    input signed [W-1:0] feedback,
	
	output reg ready,
    output reg signed [W-1:0] ctrl_sig
);

// Controller Paramters (fixed-point representation)
// -------------------------------------------------
//wire signed [W-1:0] K_p1 = 32'h0000_52F2;               // 0.324
//wire signed [W-1:0] K_p2 = 32'h0000_52F2;               // 0.324
//wire signed [W-1:0] K_i1 = 32'h0000_3B71;               // 0.232192
//wire signed [W-1:0] K_d1 = 32'hFFFF_ED3A;               // -0.0733333
//wire signed [W-1:0] K_d2 = 32'hFFFF_ED3A;               // -0.0733333
//wire signed [W-1:0] K_d3 = 32'h0000_5555;               // 0.333333
wire signed [W-1:0] K_p1 = 32'h0000_0439;               // 0.0165
wire signed [W-1:0] K_p2 = 32'h0000_0439;               // 0.0165
wire signed [W-1:0] K_i1 = 32'h0000_086C;               // 0.032904
wire signed [W-1:0] K_d1 = 32'h0000_0000;               // 0.0
wire signed [W-1:0] K_d2 = 32'h0000_0000;               // 0.0
wire signed [W-1:0] K_d3 = 32'h0000_5555;               // 0.333333
// State Machine Constants
// -----------------------
parameter IDLE     = 2'b00;
parameter MULTIPLY = 2'b01;
parameter COMBINE  = 2'b10;
parameter SATURATE = 2'b11;

// Internal Variables
// ------------------

reg [1:0] state;
//reg startFlag;

reg signed [W-1:0] ctrl_sig_unsat;

// Previous states
reg signed [W-1:0] integral_prev;
reg signed [W-1:0] derivative_prev;
reg signed [W-1:0] dw_error_prev;  // Previous derivative weighted error

// Weighted errors
wire signed [W-1:0] pw_error;
wire signed [W-1:0] iw_error;
wire signed [W-1:0] dw_error;

// Calculation variables - proportional term
wire signed [W-1:0] proportional;
wire signed [W-1:0] mul_pr;		// Multiply setpoint
wire signed [W-1:0] mul_py;  	// Multiply feedback
wire mpr_ready;
wire mpy_ready;

// Calculation variables - integral term
wire signed [W-1:0] integral;
wire signed [W-1:0] mul_ie;   	// Multiply error
wire mie_ready;

// Calculation variables - derivative term
wire signed [W-1:0] derivative;
wire signed [W-1:0] mul_dr;		// Multiply setpoint
wire signed [W-1:0] mul_dy;		// Multiply feedback
wire signed [W-1:0] mul_dd;		// Multiply previous derivative
wire mdr_ready;
wire mdy_ready;
wire mdd_ready;

// Multiplier signals
reg mul_start;
wire mul_ready;

assign mul_ready = mpr_ready & mpy_ready & mie_ready & mdr_ready & mdy_ready & mdd_ready;

// Weighted Errors
assign pw_error = mul_pr - mul_py;
assign iw_error = setpoint - feedback;
assign dw_error = mul_dr - mul_dy;

// PID terms
assign proportional = pw_error;
assign integral = mul_ie + integral_prev;
assign derivative = dw_error - dw_error_prev + mul_dd;

// Multipliers
// -----------
Multiplier Mpr( setpoint, K_p1, clk, reset, mpr_ready, mul_start, mul_pr);
Multiplier Mpy( feedback, K_p2, clk, reset, mpy_ready, mul_start, mul_py);

Multiplier Mie( iw_error, K_i1, clk, reset, mie_ready, mul_start, mul_ie);

Multiplier Mdr( setpoint, K_d1, clk, reset, mdr_ready, mul_start, mul_dr);
Multiplier Mdy( feedback, K_d2, clk, reset, mdy_ready, mul_start, mul_dy);
Multiplier Mdd( derivative_prev, K_d3, clk, reset, mdd_ready, mul_start, mul_dd);

// Logic
// -----

//always @(posedge start) startFlag <= 1'b1;

initial begin
    state <= IDLE;
    ready <= 1'b1;
    mul_start <= 1'b0;
    
    integral_prev <= 0;
    derivative_prev <= 0;
	dw_error_prev <= 0;
end

always @(posedge clk) begin
	if (reset == 1) begin
		// Set all states to 0
		state <= IDLE;
		ready <= 1;
		
		integral_prev <= 0;
		derivative_prev <= 0;
		dw_error_prev <= 0;
		
		mul_start <= 0;
	end
	else begin
		if (state == IDLE) begin
			if (start) begin
				mul_start <= 1'b1;
				state <= MULTIPLY;
				ready <= 1'b0;
//				startFlag <= 1'b0;
			end
		end
		else if (state == MULTIPLY) begin
			// Multiplication has started, don't need start signal active any more
			if (mul_start) mul_start <= 0;
			
			// Wait until multiplication has finished
			if (mul_ready & ~mul_start) state <= COMBINE;
		end
		else if (state == COMBINE) begin		
			ctrl_sig_unsat <= proportional + integral + derivative;
			
			integral_prev <= integral;		
			dw_error_prev <= dw_error;
			derivative_prev <= derivative;
			
			state <= SATURATE;
		end
		else if (state == SATURATE) begin
		    if (ctrl_sig_unsat > 32'sh000C_0000)
		        ctrl_sig <= 32'sh000C_0000;
		    else if (ctrl_sig_unsat < -32'sh000C_0000)
		        ctrl_sig <= -32'sh000C_0000;
		    else
		        ctrl_sig <= ctrl_sig_unsat;
		        
		    if (integral_prev > 32'sh000C_0000)
		        integral_prev <= 32'sh000C_0000;
		    else if (integral_prev < -32'sh000C_0000)
		        integral_prev <= -32'sh000C_0000;
		        
		    state <= IDLE;
		    ready <= 1;
		end

	end
end

endmodule
