import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

file = open("info.txt", "r");
lines = file.readlines()

x = [float(x) for x in lines[0].split()]

file.close()
f, ax = plt.subplots(1, 2)
a = ["10 звонков", "100 звонков", "1000 звонков", "5000 звонков"]

cl = ["red", "green", "black", "blue"]
for i in range(1, 5):
    A1 = [float(x) for x in lines[i].split()]
    ax[0].plot(x, A1, c=cl[i - 1], label=a[i - 1])

file = open("info1.txt", "r");
lines = file.readlines()

x = [float(x) for x in lines[0].split()]

for i in range(1, 5):
    A1 = [float(x) for x in lines[i].split()]
    ax[1].plot(x, A1, c=cl[i - 1], label=a[i - 1])


ax[0].set_xlabel("Вероятность потери CDR записи")
ax[0].set_ylabel("Процент восстановленных путей звонков")
ax[0].legend()
ax[1].set_xlabel("Граниченое значение для сопоставления соседних CDR, мсек")
ax[1].set_ylabel("Процент восстановленных путей звонков")
ax[1].legend()
plt.show()
