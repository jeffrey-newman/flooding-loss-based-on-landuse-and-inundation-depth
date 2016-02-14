
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>

#include "util/utility.h"
#include "loss.hpp"



int main(int argc, char **argv)
{
    
    
    /**********************************/
    /*        Program options         */
    /**********************************/
    // Inputs
    std::string landuse_map_file("no_file");
    std::string proportion_flooded_file("no_file");
    std::string flood_depth_file("no_file");
   //Outputs
    std::string loss_raster_file("out-raster.tif");
    std::string out_list_file("loss-by-class-list.txt");
    
    namespace prog_opt = boost::program_options;
    prog_opt::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("landuse,l", prog_opt::value<std::string>(&landuse_map_file), "path of the gdal capatible landuse file - using class symbology of CUDACCA - Blecic et al")
    ("proportion-flooded,p", prog_opt::value<std::string>(&proportion_flooded_file), "path of the gdal compatible raster giving the proportion [0,1] of the cell which is flooded")
    ("flood-depth,d", prog_opt::value<std::string>(&flood_depth_file), "path of the gdal compatible raster giving the average flood depth across the proportion flooded in the cell")
    ("out-raster,r", prog_opt::value<std::string>(&loss_raster_file), "path where output tif raster of loss is saved")
    ("out-list,o", prog_opt::value<std::string>(&out_list_file), "path where the output text file listing loss by class and total loss is saved");
    
    prog_opt::variables_map vm;
    prog_opt::store(prog_opt::parse_command_line(argc, argv, desc), vm);
    prog_opt::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }
    
    namespace fs = boost::filesystem;
    
    fs::path landuse_path(landuse_map_file);
    fs::path proportion_path(proportion_flooded_file);
    fs::path depth_path(flood_depth_file);
    fs::path loss_path(loss_raster_file);
    
    
    // Check file exists
    if (!fs::exists(landuse_path))
    {
        std::stringstream ss;
        ss << landuse_path << " does not exist";
        throw std::runtime_error(ss.str());
        return (EXIT_FAILURE);
    }
    
    if (!fs::exists(proportion_path))
    {
        std::stringstream ss;
        ss << proportion_path << " does not exist";
        throw std::runtime_error(ss.str());
        return (EXIT_FAILURE);
    }
    
    if (!fs::exists(depth_path))
    {
        std::stringstream ss;
        ss << depth_path << " does not exist";
        throw std::runtime_error(ss.str());
        return (EXIT_FAILURE);
    }

   
    // open a raster data set
    auto landuse = raster_util::open_gdal_raster<int>(landuse_path, GA_ReadOnly);
    auto proportion = raster_util::open_gdal_raster<double>(proportion_path, GA_ReadOnly);
    auto depth = raster_util::open_gdal_raster<double>(depth_path, GA_ReadOnly);
    
    // create a raster data set, with same dimensions as flood inundation maps
    auto loss_raster = raster_util::create_gdal_raster_from_model<double>(loss_path, depth);
    loss_raster.setNoDataValue(0.0);
    std::map<int, double> loss_by_class;
    double net_loss;
    
    calcLoss(landuse, proportion, depth, loss_raster, loss_by_class, net_loss);
    
    std::ofstream list_fs(out_list_file);
    if (list_fs.is_open())
    {
        list_fs << "total_loss\t" << net_loss << "\n";
        list_fs << "class\tnet_loss_for_class\n";
        for (auto& i : loss_by_class)
        {
            list_fs << i.first << "\t" << i.second << "\n";
        }
    }
    
}