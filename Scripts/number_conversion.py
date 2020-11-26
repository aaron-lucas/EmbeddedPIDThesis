# Timing Parameters
fs      = 50        # Sampling frequency
Ts      = 1 / fs    # Sampling period

fclk    = 50e6      # System clock frequency

# Encoder Parameters
Nt = 5462.22        # Number of ticks per revolution (encoder)

# Controller Parameters
Kp      = 0.0165    # Proportional gain
Ki      = 1.6452    # Integral gain
Kd      = 0         # Derivative gain

b       = 1         # Proportional setpoint weight
c       = 1         # Derivative setpoint weight

Nfilt   = 100       # Derivative filter order

Vsat    = 12        # Maximum (and minimum) output voltage from controller

# Constant calculation
constants = {
    "K_p1" : Kp * b,
    "K_p2" : Kp,

    "K_i1" : Ki * Ts,

    "K_d1" : Kd * Nfilt *c / (1 + Nfilt * Ts),
    "K_d2" : Kd * Nfilt / (1 + Nfilt * Ts),
    "K_d3" : 1 / (1 + Nfilt * Ts),


    "tickCoeff"     : 60 * fs / Nt,
    "voltageOffset" : 3 * Vsat,
    "voltageCoeff"  : 0.0005 * fclk / Vsat
}

# Output code
for name, val in constants.items():
    power   = 8 if name == "voltageCoeff" else 16

    hex_val = hex(round(val * 2**power))[2:]
    hex_val = hex_val.zfill(8).upper()
    hex_val = f'{hex_val[0:4]}_{hex_val[4:]}'

    size = "W-1" if "_" in name else "31" 

    print(f"wire signed [{size}:0] {name} = 32'h{hex_val};\t\t// {float(val):.6}");

