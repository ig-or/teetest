
##### DC motor
###### formulas

$$ 
U-E=R_mI+L_m\frac{dI}{dt}
\tag{1}
$$
U - motor voltage
E - voltage created by motor
$R_m$ - motor resistance
$I$ - current
$$
E=K_e\omega_m
\tag{2}
$$
$\omega_m$ - angular velocity of the motor
$K_e$ hopefully a constant
$$
M=K_m(I-I_0)
\tag{3}
$$
$M$ - moment from the motor
$K_m$ - hopefully a constant
$I_0$ - no-load current (0.15A)

In case $L_m\approx0$, 
$$
M=\frac{K_m}{R_m}(U - K_e\omega)
$$
$$
U=\frac{R_m}{K_m}M+K_e\omega
$$

###### Motor parameters (from the plot)

from the specs of the motor : https://www.pololu.com/product/4691/specs
https://www.pololu.com/file/0J1736/pololu-37d-metal-gearmotors-rev-1-2.pdf
![](pic/motor.png)


$I_0$ - no-load current 0.15A
$K_m\approx0.03$
$K_e\approx0.34$
$R_m\approx2.18$ Om




