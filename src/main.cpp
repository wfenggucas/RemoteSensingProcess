#include "global.h"
#include "common/cmdoptions.h"
#include "cmdline.h"
#include "analysis/map.h"

#define LISA_VERSION "Large Image Spatial Analysis v1.5.1 (c) 2014-2016 - Sebastian Lehmann"

void PrintVersion(int mode=0)
{
  cout << LISA_VERSION << endl;
  if (mode) {
  cout << "compiled ";
  #ifdef __GNUC__
    cout << "with GNU " << __VERSION__;
  #endif
  #if __x86_64__
    cout << " (64-bit)";
  #else
    cout << " (32-bit)";
  #endif
  cout << " on " << __DATE__ << endl;
  }
  #ifdef TIFF_SUPPORT
    cout << endl << TIFFGetVersion() << endl;
  #endif
}

const std::string LISA_USAGE={
"lisa [-/--options]\n\n"
"-a,--analyze       analyze connected components of a raster file (#-area cells)\n"
"  -f,--fragment    statistics using minimum fragment size in [ha] (default: 0)\n"
"  -s,--save        save results to .csv file, 1=small, 2=large\n"
"  --edge-distance  minimum cardinal non forest distance for edge detection, default: 50m\n"
"  -w,--write       write clusterlabel data (bin/lab), 1=clusters+labels, 2=labels\n"
"  -b               mean biomass\n"
"  --agb-file       agb biomass file [t/ha]\n"
"  --bthres         biomass threshold [t/ha] (default: 0 t/ha)\n"
"  --raster-file    intersection file for analyzation\n"
"  --raster-mask    integer vector of intersection mask\n"
"  --check          check consistency of component analysis\n"
"  --rloss          relative carbon loss in edge areas, default: 0.5\n"
"  --flush          flush clusters to use fixed amount of memory\n"
"  --surfacearea    calculate total surface area (slow)\n"
"-c,--convert       convert raster file into [bri] file (g=globcover)\n"
"-m,--map           produce a density map, out of bin/lab file\n"
"  --map-type       0=closs, 1=core/area\n"
"  -e,--extend      top,left,right,bottom\n"
"--reduce           reduce classified image\n"
"--classify         classify density map and datamask map\n"
"  --map-scale      number of output classes 1...256\n"
"  --map-class      0=4 classes, 1=--map-scale classes\n"
"--map-reduction    reduction factor, default: 500\n"
"-t,--test          test consistence of lisa\n"
"-v,--verbose       verbosity level [0-2] (default: 1)\n"
"-d,--dept          edge effect dept of d [m], default: 100\n"
"--info             info about raster file\n"
"--input            inputfile\n"
"--output           outputfile\n"
"--version          print version info\n"
"--threshold        forest cover threshold for forest/nonforest map\n"
"--force            force overwrite of files\n"
"supported raster file formats: asc, pgm, tiff, bri\n"
};

int main(int argc,char *argv[])
{
    AnalyzeOptions AnalyzeOptions;

    int verbosity_level=1;
    ComandLine::METHOD cmode=ComandLine::ANALYZE;

    bool globcover=false;
    int reduction_factor=500;
    int map_scale=256;
    int map_type=0;
    int map_class=0;
    bool force_overwrite=false;
    std::string str_ifile,str_ofile;
    std::string str_bmfile,str_maskfile;
    std::string str_shapefile;
    std::vector<int>shape_mask;
    geoExtend myGeoExtend;

    if (argc < 2) {
      cout << LISA_USAGE << endl;
      return 1;
    }

    CmdOptions myCmdOpt(argc,argv);

    if (myCmdOpt.searchOption("-e","--extend")) {
      std::vector <double>vextend;
      myCmdOpt.getOpt(vextend);
      if (vextend.size()!=4) cerr << "warning: unexpected size of extend=" << vextend.size() << endl;
      else {
        myGeoExtend.top=vextend[0];
        myGeoExtend.left=vextend[1];
        myGeoExtend.right=vextend[2];
        myGeoExtend.bottom=vextend[3];
      }
    }
    if (myCmdOpt.searchOption("-a","--analyze")) cmode=ComandLine::ANALYZE;
    if (myCmdOpt.searchOption("","--info")) cmode=ComandLine::INFO;
    if (myCmdOpt.searchOption("-c","--convert")) cmode=ComandLine::CONVERT;
    if (myCmdOpt.searchOption("-t","--test")) cmode=ComandLine::TEST;
    if (myCmdOpt.searchOption("-m","--map")) cmode=ComandLine::MAP;
    if (myCmdOpt.searchOption("","--reduce")) cmode=ComandLine::REDUCE;
    if (myCmdOpt.searchOption("","--classify")) cmode=ComandLine::CLASSIFY;
    if (myCmdOpt.searchOption("","--version")) cmode=ComandLine::VERSION;
    if (myCmdOpt.searchOption("","--force")) force_overwrite=true;
    if (myCmdOpt.searchOption("","--surfacearea")) AnalyzeOptions.calc_surface_area=true;
    if (myCmdOpt.searchOption("-v","--verbose")) myCmdOpt.getOpt(verbosity_level);
    if (myCmdOpt.searchOption("-d","--dept")) {myCmdOpt.getOpt(AnalyzeOptions.edge_dept);};
    if (myCmdOpt.searchOption("","--threshold")) {myCmdOpt.getOpt(AnalyzeOptions.forest_cover_threshold);};
    if (myCmdOpt.searchOption("","--map-mask")) {myCmdOpt.getOpt(str_maskfile);};
    if (myCmdOpt.searchOption("","--flush")) {AnalyzeOptions.flush_clusters=true;};
    if (myCmdOpt.searchOption("-f","--fragment")) {myCmdOpt.getOpt(AnalyzeOptions.min_fragment_size);};
    if (myCmdOpt.searchOption("-w","--write")) myCmdOpt.getOpt(AnalyzeOptions.write_mode);
    if (myCmdOpt.searchOption("-s","--save")) myCmdOpt.getOpt(AnalyzeOptions.save_mode);
    if (myCmdOpt.searchOption("-b","")) myCmdOpt.getOpt(AnalyzeOptions.mean_biomass);
    if (myCmdOpt.searchOption("","--edge-distance")) myCmdOpt.getOpt(AnalyzeOptions.edge_distance);
    if (myCmdOpt.searchOption("-i","--input")) myCmdOpt.getOpt(str_ifile);
    if (myCmdOpt.searchOption("-o","--output")) myCmdOpt.getOpt(str_ofile);
    if (myCmdOpt.searchOption("","--agb-file")) myCmdOpt.getOpt(str_bmfile);
    if (myCmdOpt.searchOption("","--raster-file")) myCmdOpt.getOpt(str_shapefile);
    if (myCmdOpt.searchOption("","--raster-mask")) myCmdOpt.getOpt(shape_mask);
    if (myCmdOpt.searchOption("","--map-scale")) {myCmdOpt.getOpt(map_scale);map_scale=std::max(std::min(map_scale,256),1);};
    if (myCmdOpt.searchOption("","--map-type")) myCmdOpt.getOpt(map_type);
    if (myCmdOpt.searchOption("","--map-class")) myCmdOpt.getOpt(map_class);
    if (myCmdOpt.searchOption("","--map-reduction")) myCmdOpt.getOpt(reduction_factor);
    if (myCmdOpt.searchOption("","--bthres")) myCmdOpt.getOpt(AnalyzeOptions.bthres);
    if (myCmdOpt.searchOption("","--rloss")) myCmdOpt.getOpt(AnalyzeOptions.relative_carbon_loss);
    if (myCmdOpt.searchOption("","--check")) AnalyzeOptions.check_consistency=true;

    if (verbosity_level>1) {
       cout << "mode:          " << cmode << endl;
       cout << "edge dept:     " << AnalyzeOptions.edge_dept << endl;
       cout << "mean biosmass: " << AnalyzeOptions.mean_biomass << endl;
       cout << "min fragment:  " << AnalyzeOptions.min_fragment_size << endl;
       cout << "edge distance: " << AnalyzeOptions.edge_distance << endl;
       cout << "rel. c-loss:   " << AnalyzeOptions.relative_carbon_loss << endl;
       cout << "savemode:      " << AnalyzeOptions.save_mode << endl;
       cout << "writemode:     " << AnalyzeOptions.write_mode << endl;
       cout << "bthres:        " << AnalyzeOptions.bthres << endl;
       cout << "extend:        " << myGeoExtend.top<<","<<myGeoExtend.left<<","<<myGeoExtend.right<<","<<myGeoExtend.bottom<<endl;
       cout << "infile:        '" << str_ifile << "'" << endl;
       cout << "outfile:       '" << str_ofile << "'" << endl;
       cout << "agb-file:      '" << str_bmfile << "'" << endl;
       cout << "surface-area:  " << AnalyzeOptions.calc_surface_area << std::endl;
       cout << "raster-file:   '" << str_shapefile << "'" << endl;
       cout << "raster-mask:   [";for (auto x:shape_mask) std::cout << x << " ";std::cout << "]" << std::endl;
    }
    ComandLine myCmdLine;
    switch (cmode) {
      case ComandLine::ANALYZE:myCmdLine.Analyze(str_ifile,str_bmfile,str_shapefile,shape_mask,AnalyzeOptions,myGeoExtend);break;
      case ComandLine::CONVERT:
      case ComandLine::INFO:myCmdLine.Convert(str_ifile,str_ofile,cmode,globcover,force_overwrite,myGeoExtend);break;
      case ComandLine::TEST:myCmdLine.TestConsistency();break;
      case ComandLine::MAP:myCmdLine.Create(str_ifile,str_ofile,reduction_factor,map_type,AnalyzeOptions.edge_dept,myGeoExtend);break;
      case ComandLine::REDUCE:myCmdLine.Reduce(str_ifile,str_ofile,reduction_factor);break;
      case ComandLine::CLASSIFY:myCmdLine.Classify(str_ifile,str_maskfile,str_ofile,map_scale,map_class);break;
      case ComandLine::VERSION:PrintVersion(1);break;
      default: cout << "unknown mode: " << cmode << endl;break;
    }
    return 0;
}
