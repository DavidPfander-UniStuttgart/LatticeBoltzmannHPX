#!/usr/bin/python

fileName = "poisson_domain.sc"
x_size = 10000
y_size = 10000

f = open(fileName, "w")
f.write(str(x_size) + "\n")
f.write(str(y_size) + "\n")
for x in xrange(0, x_size):
    for y in xrange(0, y_size):
        if y > 0:
            f.write(" ")
        if x == 0 or y == 0 or x == x_size - 1 or y == y_size - 1:
            f.write("B")
        elif x == 1:
            f.write("D")
        elif x == x_size - 2:
            f.write("S")
        else:
            f.write("W")
    f.write("\n")
f.close()