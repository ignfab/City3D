#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/alpha_wrap_3.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Real_timer.h>
#include <iostream>
#include <string>

namespace AW3 = CGAL::Alpha_wraps_3;
namespace PMP = CGAL::Polygon_mesh_processing;

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point_3 = K::Point_3;

using Mesh = CGAL::Surface_mesh<Point_3>;

int main(int argc, char** argv){

    std::cout.precision(17);
    // Read the input
    const std::string filename = (argc > 1) ? argv[1] : CGAL::data_file_path("data/IGN/test.obj");
    std::cout << "Reading " << filename << "..." << std::endl;
    const char *outfilename = (argc > 2) ? argv[2] : "data/IGN/test_simplified.obj";

    Mesh mesh;
    if(!PMP::IO::read_polygon_mesh(filename, mesh) || is_empty(mesh) || !is_triangle_mesh(mesh))
    {
        std::cerr << "Invalid input." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Input: " << num_vertices(mesh) << " vertices, " << num_faces(mesh) << " faces" << std::endl;

    // Compute the alpha and offset values
    const double relative_alpha = 20.;
    const double relative_offset = 600.;

    CGAL::Bbox_3 bbox = CGAL::Polygon_mesh_processing::bbox(mesh);
    const double diag_length = std::sqrt(CGAL::square(bbox.xmax() - bbox.xmin()) +
                                        CGAL::square(bbox.ymax() - bbox.ymin()) +
                                        CGAL::square(bbox.zmax() - bbox.zmin()));
    const double alpha = diag_length / relative_alpha;
    const double offset = diag_length / relative_offset;

    // Construct the wrap
    CGAL::Real_timer t;
    t.start();
    Mesh wrap;
    CGAL::alpha_wrap_3(mesh, alpha, offset, wrap);

    t.stop();
    std::cout << "Result: " << num_vertices(wrap) << " vertices, " << num_faces(wrap) << " faces" << std::endl;
    std::cout << "Took " << t.time() << " s." << std::endl;

    // Save the result
    
    std::cout << "Writing to " << outfilename << std::endl;
    CGAL::IO::write_polygon_mesh(outfilename, wrap, CGAL::parameters::stream_precision(17));


    return EXIT_SUCCESS;
}