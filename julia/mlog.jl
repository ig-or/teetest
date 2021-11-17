

using xqdata
using QWTWPlot
using Statistics
using Libdl

startXqLoader(0, libFile = "/home/igor/space/teetest/lib/libxqloader.so")
qstart()
#qstart(debug = true, qwtw_test = true, libraryName = raw"C:\Users\isandler\space\qwtw\build1\qwtw\proclib\Release\qwtw")
qsmw()

dir = joinpath(pwd(), "../../teedata")

cu = xqget(joinpath(dir, "test11.cu"), 1000000.0);

function motors(file::String)
	mtr = xqget(file, 1000.0);
	nm = size(mtr)[1]
	nw = size(mtr)[2]
	n = [0, 0]
	for i=1:nm
		if Int32(round(mtr[i, 2])) == 1
			n[1] += 1
		else
			n[2] += 1
		end
	end
	n
	m = [zeros(n[i], nw) for i=1:2]
	n = [0, 0]
	for i=1:nm
		if Int32(round(mtr[i, 2])) == 1
			n[1] += 1
			m[1][n[1], :] = mtr[i, :]
		else
			n[2] += 1
			m[2][n[2], :] = mtr[i, :]
		end
	end
	return m
end

m = motors(joinpath(dir, "test11.mtr"))




cuOffset = mean(cu[1:100000, 2:3], dims=1)
fcu = cu[:, 2:3] .- cuOffset

qfigure()
qplot1(cu[:, 1], fcu[:, 1], "cu 1", " eb", 3)
qtitle("cu 1")

qfigure()
qplot1(cu[:, 1], fcu[:, 2], "cu 2", " em", 3)
qtitle("cu 2")

encNumber = 1920.0
encTicks2Radians = 2.0* π / encNumber
encTicks2Degrees = 360.0 / encNumber

qfigure()
qimportant(0)
qplot1(m[1][:, 1],  m[1][:, 7], "target enc", "-eg", 5)
qimportant(1)
qplot1(m[1][:, 1],  m[1][:, 6], "enc", "-eb", 3)
qtitle("m1 enc")

qfigure()
qimportant(0)
qplot1(m[1][:, 1],  m[1][:, 7] .* encTicks2Degrees, "target", "-eg", 5)
qimportant(1)
qplot1(m[1][:, 1],  m[1][:, 6] .* encTicks2Degrees, "wheel angle", "-eb", 3)
qxlabel("[seconds]"); qylabel("[degrees]")
qtitle("rotation angle ")

P = 0.8
I = 1.0
D = 0.035
u1 = m[1][:, 4] .* I + m[1][:, 3] .* P - m[1][:, 8] .* D

qfigure()
qplot1(m[1][:, 1],  m[1][:, 4] .* I, "pid eint *I", "-ey", 3)
qplot1(m[1][:, 1],  m[1][:, 3] .* P, "angle error * P", "-eb", 3)
qplot1(m[1][:, 1],  m[1][:, 8] .* D, "speed_fb", "-ek", 3)

qplot1(m[1][:, 1],  m[1][:, 5], "control", "-er", 3)
qplot1(m[1][:, 1],  u1, "control 1", "-ed", 3)
qplot1(m[1][:, 1],  m[1][:, 8], "enc speed", "-em", 3)
qplot1(m[1][:, 1],  m[1][:, 9], "enc spimple speed", "-eY", 3)

qtitle("m1 PID")



