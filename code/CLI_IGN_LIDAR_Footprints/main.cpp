/*
Copyright (C) 2017  Liangliang Nan
https://3d.bk.tudelft.nl/liangliang/ - liangliang.nan@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../model/point_set.h"
#include "../model/map.h"
#include "../model/map_io.h"
#include "../model/point_set_io.h"
#include "../method/reconstruction.h"
#include "../basic/file_utils.h"

std::string getFileName(const std::string& s) {

   char sep = '/';

#ifdef _WIN32
   sep = '\\';
#endif

   size_t i = s.rfind(sep, s.length());
   if (i != std::string::npos) {
      return(s.substr(i+1, s.length() - i));
   }

   return("");
}

int main(int argc, char **argv) {
    // const std::string directory = std::string(CITY3D_ROOT_DIR) + "/../data/";

    const std::string ply_directory = std::string(CITY3D_ROOT_DIR) + "/../data/IGN/lidar_crops_as_ply/";
    const std::string obj_directory = std::string(CITY3D_ROOT_DIR) + "/../data/IGN/bdtopo_footprints_as_obj/";
    const std::string results_directory = std::string(CITY3D_ROOT_DIR) + "/../data/IGN/results/";

    std::vector<std::string> all_file_names;
    FileUtils::get_files(ply_directory, all_file_names, false);

    for (std::size_t i = 0; i < all_file_names.size(); ++i) {
        std::cout << "processing " << i + 1 << "/" << all_file_names.size() << " file..." << std::endl;
        const std::string &file_path = all_file_names[i];
        const std::string file_name = getFileName(file_path);

        // input point cloud file name
        const std::string input_cloud_file = file_path;
        // input footprint file name
        const std::string input_footprint_file = obj_directory + file_name.substr(0,file_name.find_last_of('.'))+".obj";
        // output mesh file name
        const std::string output_file = results_directory + file_name.substr(0,file_name.find_last_of('.'))+"_result.obj";

        // load input point cloud
        std::cout << "loading input point cloud data..." << std::endl;
        PointSet *pset = PointSetIO::read(input_cloud_file);
        if (!pset) {
            std::cerr << "failed loading point cloud data from file " << input_cloud_file << std::endl;
            return EXIT_FAILURE;
        }

        // load input footprint data
        std::cout << "loading input footprint data..." << std::endl;
        Map *footprint = MapIO::read(input_footprint_file);
        if (!footprint) {
            std::cerr << "failed loading footprint data from file " << input_footprint_file << std::endl;
            return EXIT_FAILURE;
        }

        Reconstruction recon;

        // Step 1: segmentation to obtain point clouds of individual buildings
        std::cout << "segmenting individual buildings..." << std::endl;
        recon.segmentation(pset, footprint);

        // Step 2: extract planes from the point cloud of each building (for all buildings)
        std::cout << "extracting roof planes..." << std::endl;
        recon.extract_roofs(pset, footprint);

        // Step 3: reconstruction of all the buildings in the scene
        Map *result = new Map;
    #ifdef HAS_GUROBI
        std::cout << "reconstructing the buildings (using the Gurobi solver)..." << std::endl;
        bool status = recon.reconstruct(pset, footprint, result, LinearProgramSolver::GUROBI);
    #else
        std::cout << "reconstructing the buildings (using the SCIP solver)..." << std::endl;
        
        bool status = recon.reconstruct(pset, footprint, result, LinearProgramSolver::SCIP);

    #endif

        if (status && result->size_of_facets() > 0) {
            if (MapIO::save(output_file, result)) {
                std::cout << "reconstruction result saved to file " << output_file << std::endl;
            } else
                std::cerr << "failed to save reconstruction result to file " << output_file << std::endl;
        } else
            std::cerr << "reconstruction failed" << std::endl;

        delete pset;
        delete footprint;
        delete result;
    
    }
    return EXIT_SUCCESS;
}