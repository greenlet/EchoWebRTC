#ifndef base_object_hpp
#define base_object_hpp

#include "stdafx.hpp"

class BaseObject {
public:
    BaseObject(const std::string &tag);
    
protected:
    void log(const char *format, ...);
    
private:
    std::string tag_;
};

#endif /* base_object_hpp */
