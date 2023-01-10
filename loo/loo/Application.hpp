/**
 * Application.cpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */

#ifndef LOO_LOO_APPLICATION_HPP
#define LOO_LOO_APPLICATION_HPP
#include <string>

#include "predefs.hpp"

struct GLFWwindow;

namespace loo {

/// Application class:
/// * init OpenGL
/// * provide:
///   * getWidth()
///   * getHeight()
///   * getFrameDeltaTime()
///   * getWindowRatio()
///   * windowDimensionChanged()
/// * let the user define the "loop" function.

class LOO_EXPORT Application {
   public:
    Application(int width = 640, int height = 480);

    // get the window id
    GLFWwindow* getWindow() const;

    // window control
    void exit();

    // delta time between frame and time from beginning
    float getFrameDeltaTime() const;
    float getTime() const;

    // application run
    void run();

    // Application informations
#ifdef __APPLE__
    // MacOS has different default window size and framebuffer size
    int getFramebufferWidth();
    int getFramebufferHeight();
    int getWindowWidth();
    int getWindowHeight();
#else
    int getWidth();
    int getHeight();
#endif /* LOO_LOO_APPLICATION_HPP */

    float getWindowRatio();
    bool windowDimensionChanged();

   private:
    enum State { stateReady, stateRun, stateExit };

    State state;

    Application& operator=(const Application&) { return *this; }

    GLFWwindow* window;

    // Time:
    float time;
    float deltaTime;

    // Dimensions:
    int width;
    int height;
    bool dimensionChanged;
    void detectWindowDimensionChange();

   protected:
    Application(const Application&){};

    std::string title;

    virtual void loop();

   private:
    void initGLFW();
    void initImGUI();
};
}  // namespace loo

#endif /* end of include guard: OPENGL_CMAKE_SKELETON_APPLICATION_HPP */
