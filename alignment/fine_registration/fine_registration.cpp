/*
 * Copyright (c) 2019, Eberty Alves
 */

// C++ standard library
#include <bits/stdc++.h>

// Point cloud library
#include <pcl/io/ply_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>

// Boost library
#include <boost/program_options.hpp>

// Typedefs
typedef pcl::PointXYZRGBNormal PointT;
typedef pcl::PointCloud<PointT> PointC;

int main(int argc, char* argv[]) {
  try {
    // Declaration of variables
    std::string cloud_src_file_name;
    std::string cloud_tgt_file_name;
    std::string output_file_name;

    std::string accumulated_file_name;
    bool b_accumulated_file;

    int max_iterations;
    double distance_threshold;
    double transformation_epsilon;
    double euclidean_fitness_epsilon;

    // Parse command-line options
    namespace po = boost::program_options;

    // Define command-line options
    po::options_description desc("Options");
    desc.add_options()
    ("help,h", "Print help message")
    ("cloud,c", po::value<std::string>(&cloud_src_file_name)->required(), "Input cloud file (.ply)")
    ("target,t", po::value<std::string>(&cloud_tgt_file_name)->required(), "Input target file (.ply)")
    ("output,o", po::value<std::string>(&output_file_name)->required(), "Output file (.ply)")
    ("accumulated,a", po::value<std::string>(&accumulated_file_name), "Saves the accumulated point cloud (aligned + target) in a .ply file")
    ("distance_threshold", po::value<double>(&distance_threshold)->default_value(200), "The maximum distance threshold between two correspondent points in source <-> target")
    ("max_iterations", po::value<int>(&max_iterations)->default_value(100), "The maximum number of iterations the internal optimization should run for")
    ("transformation_epsilon", po::value<double>(&transformation_epsilon)->default_value(1e-8), "Maximum allowable difference between two consecutive transformations in order for an optimization to be considered as having converged to the final solution")
    ("euclidean_fitness_epsilon", po::value<double>(&euclidean_fitness_epsilon)->default_value(0.5), "Maximum allowed Euclidean error between two consecutive steps in the ICP loop, before the algorithm is considered to have converged");

    // Use a parser to evaluate the command line
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    // Store the command-line options evaluated by the parser
    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    if (vm.count("cloud") && vm.count("target") && vm.count("output")) {
      cloud_src_file_name = vm["cloud"].as<std::string>();
      cloud_tgt_file_name = vm["target"].as<std::string>();
      output_file_name = vm["output"].as<std::string>();
    } else {
      throw std::string("Correct mode of use: " + std::string(argv[0]) + " -i input_cloud.ply -t input_target.ply -o output.ply [opts]");
    }

    max_iterations = vm["max_iterations"].as<int>();
    if (max_iterations <= 0) {
      throw std::string("max_iterations needs to be greater than zero.");
    }

    distance_threshold = vm["distance_threshold"].as<double>();
    transformation_epsilon = vm["transformation_epsilon"].as<double>();
    euclidean_fitness_epsilon = vm["euclidean_fitness_epsilon"].as<double>();
    
    b_accumulated_file = vm.count("accumulated");
    if (b_accumulated_file) {
      accumulated_file_name = vm["accumulated"].as<std::string>();
    }
    
    PointC::Ptr cloud_src(new PointC);
    PointC::Ptr cloud_tgt(new PointC);
    PointC::Ptr registered(new PointC);

    // Load point clouds data from disk
    if (pcl::io::loadPLYFile<pcl::PointXYZRGBNormal>(cloud_src_file_name, *cloud_src) == -1) {
      throw std::string("Couldn't load input cloud file");
    }
    std::cout << "Loaded " << cloud_src->size() << " data points from " << cloud_src_file_name << std::endl;

    if (pcl::io::loadPLYFile<pcl::PointXYZRGBNormal>(cloud_tgt_file_name, *cloud_tgt) == -1) {
      throw std::string("Couldn't load input target file");
    }
    std::cout << "Loaded " << cloud_tgt->size() << " data points from " << cloud_tgt_file_name << std::endl;

    // More exact feature alignment
    pcl::IterativeClosestPoint<PointT, PointT> icp;

    // Set the input source and target
    icp.setInputCloud(cloud_src);
    icp.setInputTarget(cloud_tgt);

    // Set the max correspondence distance (e.g., correspondences with higher distances will be ignored)
    icp.setMaxCorrespondenceDistance(distance_threshold);
    // Set the maximum number of iterations (criterion 1)
    icp.setMaximumIterations(max_iterations);
    // Set the transformation epsilon (criterion 2)
    icp.setTransformationEpsilon(transformation_epsilon);
    // Set the euclidean distance difference epsilon (criterion 3)
    icp.setEuclideanFitnessEpsilon(euclidean_fitness_epsilon);

    // Perform the alignment
    icp.align(*registered);

    // Obtain the transformation that aligned "cloud_src" to "registered"
    Eigen::Matrix4f transformation = icp.getFinalTransformation();
    std::cout << "Has converged:" << (icp.hasConverged() ? "True" : "False") << " score: " << icp.getFitnessScore() << std::endl;
    std::cout << transformation << std::endl;

    pcl::io::savePLYFileBinary(output_file_name, *registered);

    if (b_accumulated_file) {
      PointC::Ptr accumulated(new PointC);
      pcl::copyPointCloud(*registered, *accumulated);
      *accumulated = *accumulated + *cloud_tgt;
      pcl::io::savePLYFileBinary(accumulated_file_name, *accumulated);
    }

    return 0;
  } catch (boost::program_options::error& msg) {
    std::cerr << "ERROR: " << msg.what() << std::endl;
  } catch (std::string msg) {
    std::cerr << msg << std::endl;
  }

  return -1;
}
