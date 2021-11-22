module teelog

greet() = print("Hello World!")

using xqdata
using QWTWPlot
using Statistics
using Printf
using LinearAlgebra
#using Libdl

function __init__()
	startXqLoader(0, libFile = "/home/igor/space/teetest/lib/libxqloader.so")
	qstart()
	#qstart(debug = true, qwtw_test = true, libraryName = raw"C:\Users\isandler\space\qwtw\build1\qwtw\proclib\Release\qwtw")
	qsmw()
end


"""
parce mtr file and divide on two motors..
return array of the two motors
"""
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


function plotCurrent(file::String)
	cu = xqget(file, 1000000.0);
	cuOffset = mean(cu[1:100000, 2:3], dims=1)
	fcu = cu[:, 2:3] .- cuOffset

	qfigure()
	qplot1(cu[:, 1], fcu[:, 1], "cu 1", " eb", 3)
	qtitle("cu 1")

	qfigure()
	qplot1(cu[:, 1], fcu[:, 2], "cu 2", " em", 3)
	qtitle("cu 2")
end
export plotCurrent

function plotMotor(file::String)
	m = motors(file)
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
end
export plotMotor

#     imu  

function plotIMU(file::String)
	imu =  xqget(file, 1000.0);
	size(imu)
	ti = imu[:, 1]
	qfigure()
	qplot(ti, imu[:, 2], "ax", "-b", 2)
	qplot(ti, imu[:, 3], "ay", "-k", 2)
	qplot(ti, imu[:, 4], "az", "-m", 2)
	qxlabel("seconds"); qylabel("m/s^2");
	qtitle("acc data")

	qfigure()
	qplot(ti, imu[:, 5] .* (180.0/π), "wx", "-b", 2)
	qplot(ti, imu[:, 6] .* (180.0/π), "wy", "-k", 2)
	qplot(ti, imu[:, 7] .* (180.0/π), "wz", "-m", 2)
	qxlabel("seconds"); qylabel("deg/sec");
	qtitle("gyro data")

	imA = mapslices(norm, imu[:, 2:4], dims=[2])
	imW = mapslices(norm, imu[:, 5:7], dims=[2])
	qfigure()
	qplot1(ti, vec(imA), "ma", "-eb", 3)
	qplot1(ti, vec(imW), "mw", "-ek", 3)
	qtitle("a amd w modules")

	qfigure()
	qplot(ti[2:end], diff(ti), "IMU time diff", "-b", 2)
	qxlabel("seconds"); qylabel("seconds");
	qtitle("IMU time diff")
end
export plotIMU

# alg 1

function plotAlg(file::String)
	d12 =  xqget(file, 1000.0);
	w = size(d12)[2]
	mn = Dict()
	mIndex = 0
	n = size(d12)[1]
	for i=1:n
		id = Int32(d12[i, 2])
		if id > mIndex
			mIndex = id
		end
		if haskey(mn, id)
			mn[id] =  mn[id] + 1
		else
			mn[id] = 1
		end
	end
	size(d12)
	md = Dict()
	for id in mn
		#@printf "id=%d (%d)\n" id[1] id[2]
		md[id[1]] = zeros(id[2], w)
	end
	nx = zeros(Int32, mIndex)
	for i=1:n
		id = Int32(d12[i, 2])
		nx[id] += 1
		md[id][nx[id], :] = d12[i, :]
	end

	m = md[1]
	tm = m[:, 1]
	qfigure()
	qplot1(tm, m[:, 3], "a", "-eb", 3)
	qplot1(tm, m[:, 4], "wry", "-ek", 2)
	qplot1(tm, m[:, 5], "nia.angle", "-em", 3)
	qtitle("alg nia")


end
export plotAlg


end # module
