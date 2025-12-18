#include "Engine.hpp"

int main(int argc, char** argv) {
    Engine engine(argc, argv);

    while (engine.IsRunning()) {
        engine.ProcessEvents();

        engine.Update();

        engine.Render();
    }

    return 0;
}