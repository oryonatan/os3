/*
 * exceptions.h
 *
 *  Created on: Apr 25, 2013
 *      Author: maria
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#define LIB_ERROR "Output device library error\n"
#define SYS_ERROR "system error\n"

#include <exception>
#include <iostream>
using namespace std;
class SystemErrorException : public std::exception{};
class LibraryErrorException : public std::exception{};
class FilesystemErrorException: public std::exception{};
class TidNotFoundException: public std::exception{};

#endif /* EXCEPTIONS_H_ */
