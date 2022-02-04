
using tmodel
using QWTWPlot
qstart()

sol = test1()

qplot1(sol.t, sol.u, "test", "-eb", 3)
