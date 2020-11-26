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
	
    output reg signed [W-1:0] ctrl_sig
);

// Controller Paramters (fixed-point representation)
// -------------------------------------------------
wire signed [W-1:0] K_p1 = 32'h0000_06DC;			// Kp * b                    = 0.0268
wire signed [W-1:0] K_p2 = 32'h0000_06DC;			// Kp                        = 0.0268

wire signed [W-1:0] K_i1 = 32'h0000_078E;			// Ki * Ts = 0.0285375

wire signed [W-1:0] K_d1 = 32'h0000_0000;			// Kd * N * c / (1 + N * Ts) = 0
wire signed [W-1:0] K_d2 = 32'h0000_0000; 			// Kd * N / (1 + N * Ts)     = 0
wire signed [W-1:0] K_d3 = 32'h0000_8925; 			// 1 / (1 + N * Ts)          = 0.285714

// State Machine Constants
// -----------------------
parameter IDLE     = 2'b00;
parameter MULTIPLY = 2'b01;
parameter COMBINE  = 2'b10;
parameter SATURATE = 2'b11;

// Internal Variables
// ------------------

reg [1:0] state;

//reg       startFlag;

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

wire signed [2*W:0] mul_pr_long     = setpoint * K_p1;		// Multiply setpoint
wire signed [W-1:0] mul_pr;
wire signed [2*W:0] mul_py_long     = feedback * K_p2;  	// Multiply feedback
wire signed [W-1:0] mul_py;

assign mul_pr = {mul_pr_long[2*W], mul_pr_long[W-2+Q:Q]};
assign mul_py = {mul_py_long[2*W], mul_py_long[W-2+Q:Q]};

// Calculation variables - integral term
wire signed [W-1:0] integral;

wire signed [2*W:0] mul_ie_long     = iw_error * K_i1;   	// Multiply error
wire signed [W-1:0] mul_ie;

assign mul_ie = {mul_ie_long[2*W], mul_ie_long[W-2+Q:Q]};

// Calculation variables - derivative term
wire signed [W-1:0] derivative;

wire signed [2*W:0] mul_dr_long      = setpoint * K_d1;		// Multiply setpoint
wire signed [W-1:0] mul_dr;
wire signed [2*W:0] mul_dy_long      = feedback * K_d2;		// Multiply feedback
wire signed [W-1:0] mul_dy;
wire signed [2*W:0] mul_dd_long      = derivative_prev * K_d3;		// Multiply previous derivative
wire signed [W-1:0] mul_dd;

assign mul_dr = {mul_dr_long[2*W], mul_dr_long[W-2+Q:Q]};
assign mul_dy = {mul_dy_long[2*W], mul_dy_long[W-2+Q:Q]};
assign mul_dd = {mul_dd_long[2*W], mul_dd_long[W-2+Q:Q]};

// Weighted Errors
assign pw_error = mul_pr - mul_py;
assign iw_error = setpoint - feedback;
assign dw_error = mul_dr - mul_dy;

// PID terms
assign proportional = pw_error;
assign integral = iw_error + integral_prev;
assign derivative = dw_error - dw_error_prev + mul_dd;

// Logic
// -----

initial begin
    startFlag   <= 0;
    state       <= IDLE;
end

//always @(posedge start) startFlag <= 1'b1;

always @(posedge clk) begin
	// Synchronous Reset
	if (reset == 1) begin
		// Set all states to 0
		state <= IDLE;
		
		integral_prev <= 0;
		derivative_prev <= 0;
		dw_error_prev <= 0;
	end
	else begin
		if (state == IDLE)
		    if (start == 1'b1) begin
		        state <= MULTIPLY;
//		        startFlag <= 1'b0;
		    end
		else if (state == MULTIPLY) begin
		    mul_pr_long <= setpoint * K_p1;
		    mul_py_long <= feedback * K_p2;
		    
		    mul_ie_long <= iw_error * K_i1;
		    
		    mul_dr_long <= setpoint * K_d1;
		    mul_dy_long <= feedback * K_d2;
		    mul_dd_long <= derivative_prev * K_d3;
		    
		    state       <= COMBINE;
		end
		else if (state == COMBINE) begin		
			ctrl_sig_unsat   <= proportional + integral + derivative;
			
			integral_prev    <= integral;		
			dw_error_prev    <= dw_error;
			derivative_prev  <= derivative;
			
			state            <= SATURATE;
		end
		else if (state == SATURATE) begin
		    if (ctrl_sig_unsat > 32'sh000C_0000)
		        ctrl_sig <= 32'sh000C_0000;
		    else if (ctrl_sig_unsat < -32'sh000C_0000)
		        ctrl_sig <= -32'sh000C_0000;
		    else
		        ctrl_sig <= ctrl_sig_unsat;
		        
		    state <= IDLE;
		    ready <= 1;
		end
	end
end

endmodule
