
#ifndef LOSS_HPP
#define LOSS_HPP

#include "util/gdal_raster.h"
#include "util/zip_range.h"
#include <functional>
#include <tuple>
#include <cmath>

const double CELL_AREA = 0.09; //unit: hectares

void calcLoss(raster_util::gdal_raster<int> & landuse,
              raster_util::gdal_raster<double> & proportion,
              raster_util::gdal_raster<double> & depth,
              raster_util::gdal_raster<double> & loss_raster,
              std::map<int, double> & loss_by_class,
              double & net_loss)
{
    double loss = 0;
    auto zip = raster_util::make_zip_range(std::ref(landuse), std::ref(proportion), std::ref(depth), std::ref(loss_raster));
    for (auto i : zip)
    {
        const int & landuse_i = std::get<0>(i);
        double proportion_i = std::get<1>(i);
        double depth_i = std::get<2>(i);
        auto& loss_raster_i = std::get<3>(i);
        
        if (std::isnan(proportion_i)) proportion_i = 0;
        if (std::isnan(depth_i)) depth_i = 0;
        
        if (depth_i != 0)
        {
            std::cout << "no zero\n";
        }
        
        loss = 0.0;
        switch(landuse_i)
        {
            case 0: // high density residential
                loss = -0.0484 * pow(depth_i,4) + 0.5509 * pow(depth_i,3) - 1.907 * pow(depth_i,2) + 3.5721 * depth_i;
                break;
            case 1: // low density residential
                loss = -0.0271 * pow(depth_i,4) + 0.2921 * pow(depth_i,3) - 0.9387 * pow(depth_i,2) + 1.5621 * depth_i;
                break;
            case 2:   //inudstrial --- using commercial   (would infrastructure be better?, probably not)
                loss = -0.0218 * pow(depth_i,4) + 0.2764 * pow(depth_i,3) - 0.8716 * pow(depth_i,2) + 1.36685 * depth_i;
                break;
            case 3:   // commercial
                loss = -0.0218 * pow(depth_i,4) + 0.2764 * pow(depth_i,3) - 0.8716 * pow(depth_i,2) + 1.36685 * depth_i;
                break;
            case 4:   // Abandoned
                loss = 0.0;
                break;
            case 5:   // Green, recereation, nbature and arable and horticulture were very similar. Using one of these.
                loss = 0.0007 * pow(depth_i,4) - 0.0049 * pow(depth_i,3) - 0.0034 * pow(depth_i,2) + 0.1215 * depth_i;
                break;
            case 6:   // Facilities - again, no loss curves appropriate, will use commercial?
                loss = -0.0218 * pow(depth_i,4) + 0.2764 * pow(depth_i,3) - 0.8716 * pow(depth_i,2) + 1.36685 * depth_i;
                break;
            case 7:   // Airport - again, no loss curves appropriate, what to do, as large part of area is ok if under inundation, but large secondary losses should airport not be able to be used? Should check Attorney Generals report for their economic evaluation.
                loss = 0;
                break;
            case 8:   // Port - again, no loss curves appropriate...
                loss = 0;
                break;
            case 9:   // Water - Loss 0 as this is where the water should be! :)
                loss = 0;
                break;
        }
        
        
        loss = loss * CELL_AREA * proportion_i;
        loss_raster_i = loss;
        loss_by_class[landuse_i] += loss;
        net_loss += loss;
        
    }

    
}




#endif //LOSS_HPP