#include "terran.h"
#define FRAME_SAVE_FOLDER "frame_save_folder"

int main(int argc, char* argv[]){
    // test command line parameter
    if (argc != 2) {
        std::cout << "The arguments are incorrect." << std::endl;
        std::cout << "usage: ./hw1 <heightmap file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    // read height images
    LoadJPG* heightmapImage = new LoadJPG();
    if (heightmapImage->load(argv[1]) != LoadJPG::OK) {
        std::cout << "Error reading image " << argv[1] << "." << std::endl;
        exit(EXIT_FAILURE);
    }
    // read background image
    LoadJPG* bgImage = new LoadJPG();
    if (bgImage->load("bg.jpg") == LoadJPG::OK) {
        terran::render(heightmapImage, bgImage);
    }
    else {
        terran::render(heightmapImage);
    }
}
