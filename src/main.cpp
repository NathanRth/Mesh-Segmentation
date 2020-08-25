#include <iostream>
#include <unistd.h>

#include "mesh.hpp"

int main(int argc, char *argv[])
{
    Mesh *mesh = nullptr;
    Mesh *initial = nullptr;

    std::string in_filename;
    std::string out_filename = "out.obj";
    bool verbose = false;

    int opt;
    double step = 1.0;
    while((opt = getopt(argc, argv, "s:o:v")) != -1)
    {
        switch (opt) {
        case 's':
            step = atof(optarg);
            break;
        case 'o':
            out_filename = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << "[-o outfile] [-s step] [-v] infile" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if(optind >= argc)
    {
        std::cerr << "Expected arguments after options" << std::endl;
        std::cerr << "Usage: " << argv[0] << "[-o outfile] [-s step] [-v] infile" << std::endl;
        exit(EXIT_FAILURE);
    }

    in_filename = argv[optind];

    try {
        mesh = new Mesh(in_filename);
        initial = new Mesh(in_filename);
    } catch (const char* e) {
        std::cerr << e << std::endl;
        if(mesh != nullptr)
            delete mesh;
        exit(EXIT_FAILURE);
    }
    if(mesh)
    {
        mesh->skeletonization();
        mesh->debug();
        mesh->exportOBJ("skel_"+out_filename);
        mesh->segmentation(out_filename);
        //mesh->simpleExport(out_filename);
        delete mesh;
    }
    else
    {
        std::cerr << "Failed to initilialize mesh" <<std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
