module System
(
    input clk,
    input resetBtn,
    input chAPin,
    input chBPin,
    input controlPin,
    output wire pwmPin,
    output wire timingPin
);

// System clock generator (100Mhz -> 50Mhz)
reg       systemClk;

// Calculation Constants
wire signed [31:0] tickCoeff        = 32'h0000_8C9A;           // 0.549227
wire signed [31:0] voltageOffset    = 32'h0024_0000;           // 36.0
wire signed [31:0] voltageCoeff     = 32'h000823_55;           // 2083.33

// Sampling clock signal
reg [20:0] sampleClkCounter;
reg        sampleClk;

// Controller ready
wire pidReady;

assign timingPin = pidReady;

reg signed [31:0] setpoint = 32'h000A_0000;

wire direction;

wire signed [31:0] voltageControlSignal;
wire signed [64:0] pwmCount_long = voltageCoeff * (voltageControlSignal + voltageOffset);
wire signed [31:0] pwmCount = {pwmCount_long[64], pwmCount_long[54:24]};

wire signed [31:0] quadratureTickCount;
wire signed [64:0] rpmFeedback_long = tickCoeff * quadratureTickCount;
wire signed [31:0] rpmFeedback = {rpmFeedback_long[64], rpmFeedback_long[46:16]};

Controller #(32, 16) pidController(
    .clk(systemClk),
    .reset(resetBtn),
    .start(sampleClk),
    .setpoint(setpoint),
    .feedback(rpmFeedback),
    .ready(pidReady),
    .ctrl_sig(voltageControlSignal)
);

PWM #(32) pwmModule(
    .sig_in(pwmCount),
    .clk(systemClk),
    .reset(resetBtn),
    .pwm_out(pwmPin)
);

QuadratureDecoder #(32) quadratureModule(
    .clk(systemClk),
    .reset(resetBtn),
    .chA(chAPin),
    .chB(chBPin),
    .update(sampleClk),
    .direction(direction),
    .counter(quadratureTickCount)
);

initial begin
    sampleClkCounter    <= 21'b0;
    sampleClk           <= 1'b0;
    systemClk           <= 1'b0;
    setpoint            <= 32'h000A_0000;
end

// Generate sampling clock signal
always @(posedge systemClk) begin
    sampleClkCounter <= sampleClkCounter + 1'b1;
    if (sampleClkCounter == 1000000) begin
        sampleClkCounter <= 21'b0;
        sampleClk <= 1'b1;
    end else sampleClk <= 1'b0;
end

always @(posedge clk)
    systemClk <= ~systemClk;
    
always @(posedge clk)
    if (controlPin)
        setpoint <= 32'h0014_0000;
    else
        setpoint <= 20'h000A_0000;
    

endmodule