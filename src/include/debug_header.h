/*
 * debug_header.h
 *
 *  Created on: Aug 6, 2016
 *      Author: garci
 */

#ifndef DEBUG_HEADER_H_
#define DEBUG_HEADER_H_

#ifndef DEBUG
#define DEBUG 0
#endif

#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)

#endif /* SRC_INCLUDE_DEBUG_HEADER_H_ */
