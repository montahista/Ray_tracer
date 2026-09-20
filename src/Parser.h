#pragma once
#include <string>
#include "Scene.h"

class Parser {
public:
    bool loadScene(const std::string& filename, Scene& scene);
};