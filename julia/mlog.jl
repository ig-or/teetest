

using xqdata
using QWTWPlot
using Statistics

startXqLoader(0, 0)
qstart()
#qstart(debug = true, qwtw_test = true, libraryName = raw"C:\Users\isandler\space\qwtw\build1\qwtw\proclib\Release\qwtw")
qsmw()

cu = xqget(raw"C:\Users\isandler\space\tee\test2.cu", 1000000.0);

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

m = motors(raw"C:\Users\isandler\space\tee\test2.mtr")




cuOffset = mean(cu[1:100000, 2:3], dims=1)
fcu = cu[:, 2:3] .- cuOffset

qfigure()
qplot1(cu[:, 1], fcu[:, 1], "cu 1", " eb", 3)
qtitle("cu 1")

qfigure()
qplot1(cu[:, 1], fcu[:, 2], "cu 2", " em", 3)
qtitle("cu 2")


qfigure()
qplot1(m[1][:, 1],  m[1][:, 7], "target enc", "-eg", 5)
qplot1(m[1][:, 1],  m[1][:, 6], "enc", "-eb", 3)
qtitle("m1 enc")

P = 0.16
I = 0.5

qfigure()
qplot1(m[1][:, 1],  m[1][:, 4], "pid eint", "-ey", 3)
qplot1(m[1][:, 1],  m[1][:, 3], "angle error", "-eb", 5)
qplot1(m[1][:, 1],  m[1][:, 5], "control", "-er", 3)
qplot1(m[1][:, 1],  m[1][:, 8], "enc speed", "-em", 3)
qplot1(m[1][:, 1],  m[1][:, 9], "enc spimple speed", "-eY", 3)

qtitle("m1 PID")



