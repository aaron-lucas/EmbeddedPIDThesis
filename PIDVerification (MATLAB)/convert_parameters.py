"""
convert_parameters.py

Script to convert PID parameters between different forms.

Author: Aaron Lucas
Date Created: 2020/04/03

Written for the Off-World Robotics Team
"""

from sys import argv

def print_help():
    print('Usage:\tpython3 convert_parameters.py [Sample Time] [Kp] [Ki] [Kd]')
    print()
    print('Note: Sample time is in seconds')

if __name__ == '__main__':
    try:
        (Ts, Kp, Ki, Kd) = map(float, argv[1:])
    except ValueError:
        print('Invalid Input')
        print_help();
    else:
        Ti = Kp / Ki
        Td = Kd / Kp

        q0 = Kp * (1 + Td / Ts)
        q1 = -Kp * (1 - Ts / Ti + 2 * Td / Ts)
        q2 = Kp * Td / Ts

        print(f'Kp = {Kp:.6f}\tKi = {Ki:.6f}\tKd = {Kd:.6f}')
        print(f'Kp = {Kp:.6f}\tTi = {Ti:.6f}\tTd = {Td:.6f}')
        print(f'q0 = {q0:.6f}\tq1 = {q1:.6f}\tq2 = {q2:.6f}')
