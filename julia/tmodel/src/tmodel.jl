module tmodel

using DifferentialEquations

function test1()
	f(u,p,t) = -0.89*u
	u0 = 1/2
	tspan = (0.0,1.0)
	prob = ODEProblem(f,u0,tspan)
	sol = solve(prob, Tsit5(), reltol=1e-8, abstol=1e-8)

	return sol
end

export test1


end # module
