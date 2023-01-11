
#include <iostream>
#include <loo/loo.hpp>

#include "DSSApplication.hpp"
int main(int argc, char *argv[]) {
    loo::initialize(argv[0]);
    DSSApplication app(800, 600);
    app.run();
}