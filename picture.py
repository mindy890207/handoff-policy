from matplotlib import pyplot as plt

bestPolicy = open("bestPolicy.txt", "r")
thresholdPolicy = open("thresholdPolicy.txt", "r")
entropyPolicy = open("entropyPolicy.txt", "r")
myPolicy = open("myPolicy.txt", "r")

x = []
y = []

for i in bestPolicy:
    dot = i.split()
    x.append(int(dot[0]))
    y.append(int(dot[1]))

plt.plot(x, y, color="r", label="best poloicy")

x = []
y = []

for i in thresholdPolicy:
    dot = i.split()
    x.append(int(dot[0]))
    y.append(int(dot[1]))

plt.plot(x, y, color="g", label="threshold policy")

x = []
y = []

for i in entropyPolicy:
    dot = i.split()
    x.append(int(dot[0]))
    y.append(int(dot[1]))

plt.plot(x, y, color="b", label="entropy policy")

x = []
y = []

for i in myPolicy:
    dot = i.split()
    x.append(int(dot[0]))
    y.append(int(dot[1]))

plt.plot(x, y, color="k", label="my policy")
plt.xlabel("second")
plt.ylabel("#handoff")
plt.legend(loc="best")
plt.show()
