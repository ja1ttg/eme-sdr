import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as anm
import itertools
from scipy.fftpack import fft

def _update(frame):
    line = input()
    print("==INPUT ==")
    str_data = line.split(",")
    data = list(map(float, str_data))
    N = len(data)
    T = 1 / N
    yf = fft(data)
    xf = np.linspace(0.0, 1.0/(2.0*T), N//2)
    plt.cla()
    plt.plot(xf, 2.0/N * np.abs(yf[0:N//2]))

params = {
    'fig': plt.figure(figsize=(6, 3)),
    'func': _update,
    'interval': 30, # mSec
    'frames': itertools.count(0, 1),  # Frame number iterator
}

anim = anm.FuncAnimation(**params)
data = []
plt.plot(data)   
plt.show()
