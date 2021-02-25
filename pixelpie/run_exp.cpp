#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

#include "PoissonDiskSampler.hpp"
#include "Timer.hpp"

std::vector<float> runExp(size_t w, size_t h, size_t nd, float r) {

  PoissonDiskSampler *oglr = new PoissonDiskSampler(w, h, nd, r);
  size_t usedmem = oglr->init();

  size_t emptypixels = 0;
  size_t itr = 0;
  Timer timer;
  Timer t1, t2, t3;
  vector<GLfloat> res;

  for (int i = 0; i < 1; i++) {
    double p1 = 0, p2 = 0, p3 = 0;
    oglr->reset();
    glFinish();
    itr = 0;
    timer.start();
    do {
      t1.start();
      oglr->throwDarts();
      glFinish();
      p1 += t1.stop();
      t2.start();
      oglr->removeConflict();
      glFinish();
      p2 += t2.stop();
      t3.start();
      emptypixels = oglr->collectEmptyPixels();
      glFinish();
      p3 += t3.stop();
      itr++;
    } while (emptypixels > 0 && itr < 200);
    double elapsed = timer.stop();

    // get the results

    oglr->downloadResults(res);
    size_t npts = res.size() / 2;
  }
  delete oglr;
  return vector<float>(res.begin(), res.end());
}

float computeR(size_t n) { return 0.7766 * sqrt(2.0 / (sqrt(3.0) * n)); }

size_t computeN(float r) { return 2.0 / (sqrt(3.0) * pow(r / 0.7766, 2)); }

std::vector<float> run_cpp(int w, int h, float r) {
  if (!glfwInit()) {
    cerr << "glfw init failed" << endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
  GLFWwindow *window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewInit();

  return runExp(w, h, computeN(r) / 2, r);
}