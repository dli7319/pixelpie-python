import numpy as np
from libcpp.vector cimport vector
# distutils: language=c++
cdef extern from "run_exp.hpp":
    vector[float] run_cpp(int w, int h, float r)

def run(int w, int h, float r):
    arr = run_cpp(w, h, r)
    return np.asarray(arr).reshape(-1, 2)