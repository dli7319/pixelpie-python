import numpy as np
from libcpp.vector cimport vector
# distutils: language=c++
cdef extern from "run_exp.hpp":
    vector[float] run_cpp(int w, int h, float r)

def run(int width, int height, float r):
    arr = run_cpp(width, height, r)
    return np.asarray(arr).reshape(-1, 2)
