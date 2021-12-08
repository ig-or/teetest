
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



##### pendulum
some of the info is taken from http://spin7ion.ru/ru/blog/balancerBuildSteps

lets consider "flat" (2D) model.
![](pic/wheele.png)

$m_p$ robot mass (without wheels)
$m_w$ mass of the wheels
$l$ length (height?) 
$r$ wheel radius
$\theta$ angle between vertical and pendulum
$\varphi$ wheel rotation angle
$M$ DC motor moment
$\omega=\.\theta$ some (part of) angular velo from the IMU
$\omega_w=\.\varphi$ wheel rotation rate


wheel center
$$ x = r\varphi $$
$x_p, y_p$ - coordinates of the "upper point"
$$  x_p=x+lsin(\theta) \\ y_p=r+lcos(\theta) $$


the motion eq:
$$ M=rlm_pcos(\theta)\.\omega+r^2(m_p+2m_w)\.\omega_w-rlm_psin(\theta)\theta^2  $$





