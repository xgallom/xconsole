/*
 * util.h
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#ifndef UTIL_H_
#define UTIL_H_

template<typename T>
T sgn(T x) { return x < 0 ? -1 : 1; }

template<typename T>
T abs(T x) { return x < 0 ? -x : x; }

template<typename T>
T min(T x, T y) { return x < y ? x :  y; }

template<typename T>
T max(T x, T y) { return x > y ? x :  y; }

#endif /* UTIL_H_ */
