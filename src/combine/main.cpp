#include <iostream>
#include <merge.h>
#include "args.hxx"


int main(int argc, char * argv[]){
  args::ArgumentParser parser("Combine MCPL files created with denormalize-mcpl",
                              "");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag verbose(parser, "verbose", "Print additional information", {'v', "verbose"});

  args::ValueFlag<std::string> output_file(parser, "output", "Output filename", {'o', "output"});
  args::PositionalList<std::string> input_files(parser, "filename", "Filenames to combine");

  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch (const args::Help&)
  {
    std::cout << parser;
    return 0;
  }
  catch (const args::ParseError& e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  if (!input_files){
    std::cerr << "No input files provided" << std::endl;
    return 1;
  }
  auto files = args::get(input_files);
  if (files.size() < 2){
    std::cerr << "At least two input files are required" << std::endl;
    return 1;
  }
  std::string output = output_file ? args::get(output_file) : "combined.mcpl";
  if (verbose) {
    std::cout << "Combining files: ";
    for (const auto& file : files) {
      std::cout << file << " ";
    }
    std::cout << "into " << output << std::endl;
  }

  auto signal = merge_files(output, files);

  if (signal == -1){
    std::cerr << "Error: No files to merge" << std::endl;
    return 1;
  }
  if (signal < 0){
    int unopened = -10 - signal;
    std::cerr << "Error: No particle counts found in " << files.size() - unopened << " files";
    if (unopened){
      std::cerr << ", " << unopened << " files were not opened";
    }
    std::cerr << std::endl;
    return 1;
  }
  if (signal > 0){
    std::cerr << "Warning: Particles from " << signal << " of the files were not merged" << std::endl;
  }

  return EXIT_SUCCESS;
}