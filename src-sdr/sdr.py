import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as anm
import itertools
from scipy.fftpack import fft

def _update(frame):
    data = list(map(float, input().split(",")))
    N = len(data)
    T = 1 / N
    yf = fft(data)
    xf = np.linspace(0.0, 1.0/(2.0*T), N//2)
    plt.cla()
    plt.title('FFT')
    plt.xlabel('Frequency')
    plt.ylabel('Signal level')
    plt.yscale('log')
    plt.plot(xf, 2.0/N * np.abs(yf[0:N//2]))
    plt.grid()

params = {
    'fig': plt.figure(figsize=(6, 3)),
    'func': _update,
    'interval': 10, # mSec
    'frames': itertools.count(0, 1),  # Frame number iterator
}

anim = anm.FuncAnimation(**params)

plt.show()
