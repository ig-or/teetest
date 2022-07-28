module tmodel

using DifferentialEquations
using StaticArrays

function test1()
	f(u,p,t) = -0.89*u
	u0 = 1/2
	tspan = (0.0,1.0)
	prob = ODEProblem(f,u0,tspan)
	sol = solve(prob, Tsit5(), reltol=1e-8, abstol=1e-8)
	return sol
end
export test1

struct RP1
	l::Float64 # distance from the wheel center to the mass center of the robot
	r::Float64 # wheel radius
	ri::Float64  # 'inertial radius' if the wheel. Iw = mw * ri^2
	mp::Float64 # robot mass without wheels
	mw::Float64 # mass of the wheels


end
function RP1() 
	l = 0.35
	r = 0.1
	ri = 0.1
	mp = 2.5
	mw = 0.2
	return RP1(l, r, ri, mp, mw)
end




end # module
