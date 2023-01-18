/**
 * glError.hpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */

#ifndef LOO_LOO_GL_ERROR_HPP
#define LOO_LOO_GL_ERROR_HPP
#include <glad/glad.h>
#include <glog/logging.h>

#include <iostream>

#include "predefs.hpp"

namespace loo {

#define logPossibleGLError()                                          \
    if (auto e = glGetError(); e != GL_NO_ERROR) {                    \
        LOG(ERROR) << "Error code " << e << ": " << errCode2String(e) \
                   << std::endl;                                      \
    }
// throw a fatal when met possible opengl error
#define panicPossibleGLError()                                        \
    if (auto e = glGetError(); e != GL_NO_ERROR) {                    \
        LOG(FATAL) << "Error code " << e << ": " << errCode2String(e) \
                   << "; Exit now!" << std::endl;                     \
    }
#define NOT_IMPLEMENTED()                 \
    LOG(FATAL) << "Function " << __func__ \
               << "() hasn't been implemented yet!\n";
LOO_EXPORT const char* errCode2String(GLenum err);

}  // namespace loo

#endif /* LOO_LOO_GL_ERROR_HPP */
