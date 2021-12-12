

using teelog
using xqdata
using LinearAlgebra
using QWTWPlot

dir = joinpath(pwd(), "../lib/release")
file = "log_15"

startXqLoader(0, libFile = "/home/igor/space/teetest/lib/release/libxqloader.so")
qstart()
qsmw()

#cuFile = joinpath(dir, file*".cu")
#mFile = joinpath(dir, file*".mtr")

imuFile = joinpath(dir, file*".imu")
algFile = joinpath(dir, file*".d16")

imu =  xqget(imuFile, 1000.0);
imA = mapslices(norm, imu[:, 2:4], dims=[2])
imW = mapslices(norm, imu[:, 5:7], dims=[2])
size(vec(imA))


plotIMU(imuFile)
plotAlg(algFile)
