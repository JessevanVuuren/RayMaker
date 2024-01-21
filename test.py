import math
step = 10

start = 0
stop = 1

for i in range(step):
    point = math.pi * (i / step)
    print(math.cos(point))