

using teelog
using xqdata
using LinearAlgebra

dir = joinpath(pwd(), "lib/release")
file = "log_7"


cuFile = joinpath(dir, file*".cu")
mFile = joinpath(dir, file*".mtr")
imuFile = joinpath(dir, file*".imu")
algFile = joinpath(dir, file*".d12")

imu =  xqget(imuFile, 1000.0);
imA = mapslices(norm, imu[:, 2:4], dims=[2])
imW = mapslices(norm, imu[:, 5:7], dims=[2])
size(vec(imA))


plotIMU(imuFile)
plotAlg(algFile)