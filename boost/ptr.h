#ifndef BOOST_PTR_PTR_H
#define BOOST_PTR_PTR_H

#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class parent;
class children;

typedef boost:shared_ptr<parent> parent_ptr;
typedef boost:shared_ptr<children> children_ptr;

class parent {
public:
    ~parent() { std::cout << "Destroying parent\n";}

public:
    children_ptr children;
};

class children {
public:
    ~children() {std::cout << "Destroying children\n";}

public:
    parent_ptr parent;
};
#endif
