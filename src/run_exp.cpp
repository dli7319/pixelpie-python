#include <GL/glew.h>
#include <EGL/egl.h>
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

static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
};    

static const EGLint pbufferAttribs[] = {
      EGL_WIDTH, 0,
      EGL_HEIGHT, 0,
      EGL_NONE,
};

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
  // 1. Initialize EGL
  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;

  eglInitialize(eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, 
                                               pbufferAttribs);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, 
                                       NULL);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);
  
  glewInit();

  auto results = runExp(w, h, computeN(r) / 2, r);

  eglTerminate(eglDpy);
  return results;
}