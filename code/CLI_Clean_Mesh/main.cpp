#include <iostream>
#include <string>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/internal/repair_extra.h>
#include <CGAL/Polygon_mesh_processing/internal/Corefinement/face_graph_utils.h>
#include <CGAL/Polygon_mesh_processing/internal/Corefinement/predicates.h>
#include <CGAL/Polygon_mesh_processing/manifoldness.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup_extension.h>
#include <CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/repair_self_intersections.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> Surface_mesh;
typedef boost::graph_traits<Surface_mesh>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<Surface_mesh>::halfedge_descriptor halfedge_descriptor;
typedef boost::graph_traits<Surface_mesh>::edge_descriptor edge_descriptor;
typedef boost::graph_traits<Surface_mesh>::face_descriptor face_descriptor;

namespace PMP = CGAL::Polygon_mesh_processing;
namespace NP = CGAL::parameters;

struct Visitor : public PMP::Default_orientation_visitor
{
  void non_manifold_edge(std::size_t id1, std::size_t id2, std::size_t nb_poly)
  {
    std::cout << "\t The edge " << id1 << ", " << id2 << " is not manifold: " << nb_poly << " incident polygons." << std::endl;
  }
  void non_manifold_vertex(std::size_t id, std::size_t nb_cycles)
  {
    std::cout << "\t The vertex " << id << " is not manifold: " << nb_cycles << " connected components of vertices in the link." << std::endl;
  }
  void duplicated_vertex(std::size_t v1, std::size_t v2)
  {
    std::cout << "\t The vertex " << v1 << " has been duplicated, its new id is " << v2 << "." << std::endl;
  }
  void vertex_id_in_polygon_replaced(std::size_t p_id, std::size_t i1, std::size_t i2)
  {
    std::cout << "\t In the polygon " << p_id << ", the index " << i1 << " has been replaced by " << i2 << "." << std::endl;
  }
  void polygon_orientation_reversed(std::size_t p_id)
  {
    std::cout << "\t The polygon " << p_id << " has been reversed." << std::endl;
  }
};

int main(int argc, char *argv[])
{
  // input parameters
  const std::string filename = (argc > 1) ? argv[1] : CGAL::data_file_path("data/IGN/results/BATIMENT0000000320899175.obj");
  const char *outfilename = (argc > 2) ? argv[2] : "BATIMENT0000000320899175_clean.obj";

  std::vector<Point> points;
  std::vector<std::vector<std::size_t>> polygons;

  // read city3D result as polygon soup
  if (!CGAL::IO::read_polygon_soup(filename, points, polygons) || points.empty())
  {
    std::cerr << "Cannot open file " << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Reading city3d result as polygon soup" << std::endl;
  std::cout << "\t " << points.size() << " points imported" << std::endl;
  std::cout << "\t " << polygons.size() << " polygons imported" << std::endl;

  // repair polygon soup
  Visitor visitor;
  PMP::repair_polygon_soup(points, polygons);
  std::cout << "Trying repair_polygon_soup()" << std::endl;
  std::cout << "\t " << points.size() << " points after repair_polygon_soup" << std::endl;
  std::cout << "\t " << polygons.size() << " polygons after repair_polygon_soup" << std::endl;
  std::cout << "\t Is polygon soup a polygon mesh ? " << PMP::is_polygon_soup_a_polygon_mesh(polygons) << std::endl;

  // orient polygon soup with visitor to get details
  bool success = PMP::orient_polygon_soup(points, polygons, NP::visitor(visitor));
  std::cout << "Trying orient_polygon_soup()" << std::endl;
  std::cout << "\t Is polygon soup a polygon mesh ? " << PMP::is_polygon_soup_a_polygon_mesh(polygons) << std::endl;

  // Polygon soup to polygon mesh
  Surface_mesh mesh;
  if (PMP::is_polygon_soup_a_polygon_mesh(polygons))
  {
    std::cout << "Converting polygon_soup to polygon_mesh" << std::endl;
    PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh);
  }
  else
  {
    std::cout << "Cannot convert polygon soup to polygon mesh" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "\t Is the mesh closed ? " << CGAL::is_closed(mesh) << std::endl;
  std::cout << "\t Is the mesh valid ? " << CGAL::is_valid(mesh) << std::endl;

  // triangulate mesh faces
  PMP::triangulate_faces(mesh);
  std::cout << "=> Is the mesh closed ? " << CGAL::is_closed(mesh) << std::endl;
  std::cout << "=> Is the mesh valid ? " << CGAL::is_valid(mesh) << std::endl;

  // Check for self intersections in the mesh
  bool intersecting = PMP::does_self_intersect<CGAL::Parallel_if_available_tag>(mesh, CGAL::parameters::vertex_point_map(get(CGAL::vertex_point, mesh)));
  std::cout << (intersecting ? "There are self-intersections." : "There is no self-intersection.") << std::endl;
  std::vector<std::pair<face_descriptor, face_descriptor>> intersected_faces;
  PMP::self_intersections<CGAL::Parallel_if_available_tag>(faces(mesh), mesh, std::back_inserter(intersected_faces));
  std::cout << "\t " << intersected_faces.size() << " pairs of faces intersect." << std::endl;
  std::cout << "" << std::endl;

  std::cout << "=> Is the mesh closed ? " << CGAL::is_closed(mesh) << std::endl;
  std::cout << "=> Is the mesh valid ? " << CGAL::is_valid(mesh) << std::endl;
  
  CGAL::IO::write_polygon_mesh(outfilename, mesh, CGAL::parameters::stream_precision(17));

  return EXIT_SUCCESS;
}
