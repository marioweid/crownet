//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package crownet.common;


moduleinterface ICoordConverter {


}

// locally defined converter. Use configuration of parameters
simple OsgCoordConverterLocal like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterLocal);
        string srs_code = default("EPSG:32632");
        double offset_x = default(0.0);
        double offset_y = default(0.0);
        double xBound;
        double yBound;
}


simple OsgCoordConverterVadere like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterVadere);
        string coreModule = default("traci.core");
}

// provide network xml to parse needed location information from the location element.
simple OsgCoordConverterSumo like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterSumo);
        xml netFile;        
}

// Manually provide lcation inforation present in sumo net file.
simple OsgCoordConverterSumoLocal like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterSumo);
        string netOffset =  default("-686820.86,-5334157.15");
        string convBoundary= "0.00,0.00,7910.48,6016.93"; // 4 comma seperated double values
        //string origBoundary = default("11.512107,48.117927,11.624853,48.186458");
        string projParameter = default("+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
}















