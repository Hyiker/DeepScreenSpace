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

#define logPossibleGLError()                                     \
    if (glGetError() != GL_NO_ERROR) {                           \
        LOG(ERROR) << errCode2String(glGetError()) << std::endl; \
    }

LOO_EXPORT const char* errCode2String(GLenum err);

}  // namespace loo

#endif /* LOO_LOO_GL_ERROR_HPP */
