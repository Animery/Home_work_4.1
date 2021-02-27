#include "../include/engine.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string_view>

int main(int /*argc*/, char* /*argv*/[])
{
    std::unique_ptr<my_engine::engine, void (*)(my_engine::engine*)> engine(
        my_engine::create_engine(), my_engine::destroy_engine);

    engine->initialize("");

    bool continue_loop = true;
    while (continue_loop)
    {
        my_engine::event event;

        while (engine->read_input(event))
        {
            std::cout << event << std::endl;
            switch (event)
            {
                case my_engine::event::turn_off:
                    continue_loop = false;
                    break;
                case my_engine::event::select_released:
                    continue_loop = false;
                    break;
                default:
                    break;
            }
        }

        std::ifstream file("res/vertexes.txt");
        if (!file)
        {
            std::cerr << "can't open file: vertexes.txt" << std::endl;
        }
        else
        {
            my_engine::triangle tr;
            file >> tr;

            engine->render_triangle(tr);

            file >> tr;
            engine->render_triangle(tr);
        }

        engine->swap_buffers();
    }

    engine->uninitialize();

    return EXIT_SUCCESS;
}