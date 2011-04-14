/*
 *  MessagedException.hpp
 *  aomdd
 *
 *  Created by William Lam on 2/12/11.
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef GENERICEXCEPTION_HPP_
#define GENERICEXCEPTION_HPP_

#include <exception>

namespace aomdd {
    
    class GenericException : public std::exception {
    protected:
        std::string msg;
        
    public:
        GenericException(std::string reason) throw()
        : msg(reason) {
        }
        
        virtual ~GenericException() throw() {
        }
        
        virtual const char *what() throw() {
            return msg.c_str();
        }
        
    };
    
}


#endif /* GENERICEXCEPCTION_HPP_*/
