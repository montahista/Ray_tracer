#include <iostream>
#include "Parser.h"
#include "Renderer.h"

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage:\n"
                  << "RayTracer file.dat\n";
        return 0;
    }

    Scene scene;
    Parser parser;

    if(!parser.loadScene(argv[1], scene))
    {
        std::cout << "Scene loading failed\n";
        return 0;
    }

    Renderer renderer;
    renderer.initialize(argc, argv, scene);

    return 0;
}