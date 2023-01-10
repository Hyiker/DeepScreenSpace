/**
 * glError.hpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */

#ifndef LOO_LOO_GL_ERROR_HPP
#define LOO_LOO_GL_ERROR_HPP
#include "predefs.hpp"
namespace loo {

#define checkError() glCheckError(__FILE__, __LINE__)

// Ask Opengl for errors:
// Result is printed on the standard output
// usage :
//      glCheckError(__FILE__,__LINE__);
LOO_EXPORT void glCheckError(const char* file, unsigned int line);

}  // namespace loo

#endif /* LOO_LOO_GL_ERROR_HPP */
