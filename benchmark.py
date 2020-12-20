import matplotlib.pyplot as plt
import numpy as np
import subprocess
import os
from tqdm import tqdm


cur_dir = os.path.dirname(os.path.abspath(__file__))

exec_file = "/build/bin/lmeca2710_project"

gl_timing = np.loadtxt(os.path.join(cur_dir, "static/gilles_and_louis_timing.txt"))
je_timing = np.loadtxt(os.path.join(cur_dir, "static/jerome_timing.txt"))


if __name__ == "__main__":

    ns = np.logspace(2, 6, num=50, dtype=int)
    ts = np.zeros_like(ns, dtype=float)

    args = ["." + os.path.join(cur_dir, exec_file)]

    for i, n in tqdm(enumerate(ns), total=ns.size):
        process = subprocess.Popen(args + ["-x", str(n)], stdout=subprocess.PIPE)
        process.wait()
        stdout = process.communicate()[0]
        ts[i] = float(stdout.decode().strip())

    plt.loglog(je_timing[:, 0], je_timing[:, 1], label=r"Divide and conquer (on my pc)")
    plt.loglog(ns, ns * np.log(ns), label=r"$n\log(n)$")
    plt.loglog(ns, ns, label=r"$n$")
    plt.loglog(gl_timing[:, 0], gl_timing[:, 1], label=r"Fortune's algo. from Gilles and Louis (on my pc)")
    plt.loglog(ns, ts, "--", label=r"Divide and conquer (on your pc)")

    plt.legend()

    plt.xlabel("$n$ [-]")
    plt.ylabel("time [us]")
    plt.grid("on")
    plt.show()
