#include "Parser.h"
#include <fstream>
#include <iostream>

bool Parser::loadScene(const std::string& filename, Scene& scene)
{
    std::ifstream file(filename);
    if(!file) { std::cerr << "Cannot open: " << filename << "\n"; return false; }

    std::string token;
    while(file >> token)
    {
        if(token == "RESOLUTION")
        {
            file >> scene.resolution.width >> scene.resolution.height;
        }
        else if(token == "BACKGROUND")
        {
            int r, g, b;
            file >> r >> g >> b;
            scene.background = Color(r, g, b);
        }
        else if(token == "CAMERA")
        {
            file >> scene.camera.position.x
                 >> scene.camera.position.y
                 >> scene.camera.position.z
                 >> scene.camera.viewDir.x
                 >> scene.camera.viewDir.y
                 >> scene.camera.viewDir.z
                 >> scene.camera.up.x
                 >> scene.camera.up.y
                 >> scene.camera.up.z;
        }
        else if(token == "SPHERE")
        {
            Sphere sphere;
            int r, g, b;
            file >> sphere.center.x >> sphere.center.y >> sphere.center.z
                 >> sphere.radius
                 >> r >> g >> b
                 >> sphere.shininess       
                 >> sphere.reflectivity
                 >> sphere.transparency;
          
            sphere.color = Color(r, g, b);
            scene.spheres.push_back(sphere);
        }
        
        else if(token == "PLANE")
        {
            Plane plane;
            int r, g, b;
            file >> plane.point.x  >> plane.point.y  >> plane.point.z
                 >> plane.normal.x >> plane.normal.y >> plane.normal.z
                 >> r >> g >> b
                 >> plane.shininess     
                 >> plane.reflectivity;  
            plane.color  = Color(r, g, b);
            plane.normal = plane.normal.normalize();
            scene.planes.push_back(plane);
        }
        else if(token == "LIGHT")
        {
            Light light;
            int r, g, b;
            file >> light.position.x >> light.position.y >> light.position.z
                 >> r >> g >> b       
                 >> light.intensity;
            light.color = Color(r, g, b);
            scene.lights.push_back(light);
        }
        else
        {
            std::cerr << "Unknown token: " << token << "\n";
        }
    }
    return true;
}