#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
int main(){
    std::cout << "starting it\n";
    ve::FirstApp app;

    try{
        app.run();
    }catch(const std::exception &e){

        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}