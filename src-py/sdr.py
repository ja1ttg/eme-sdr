import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as anm
import itertools

def _update(frame):
    line = input()
    print("==INPUT ==")
    str_data = line.split(",")
    data = list(map(float, str_data))
    plt.cla()
    plt.plot(data[0:100])   

params = {
    'fig': plt.figure(figsize=(6, 3)),
    'func': _update,
    'interval': 10, # mSec
    'frames': itertools.count(0, 1),  # Frame number iterator
}

anim = anm.FuncAnimation(**params)
data = []
plt.plot(data)   
plt.show()
