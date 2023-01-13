
#include <iostream>
#include <loo/loo.hpp>

#include "DSSApplication.hpp"
int main(int argc, char *argv[]) {
    loo::initialize(argv[0]);
    DSSApplication app(1920, 1280);
    app.loadObj(argv[1]);
    app.run();
}