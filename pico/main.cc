#include "application.h"
#include <random>
#include <time.h>

int main(int argc, char* argv[]) {
    srand(time(NULL));

    pico::Application app;
    if (app.init(argc, argv)) { return app.run(); }

    return 0;
}
