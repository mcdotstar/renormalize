#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <mcpl.h>
#include "renormalize.h"
#include "merge.h"

///\brief Create a file handle for use with output of MCPL data
///\param filename The name of the output file, should end with '.mcpl' or '.mcpl.gz'
///\return A tuple containing a boolean indicating whether the file should be gzipped before closing, and
///        a handle to the output file
std::tuple<bool, mcpl_outfile_t> create_output_mcpl_file(const std::string & filename){
  // check if the file should have a .mcpl extension or a .mcpl.gz extension
  if (filename.find(".mcpl") == std::string::npos && filename.find(".mcpl.gz") == std::string::npos) {
    std::cerr << "Error: Output file must have .mcpl or .mcpl.gz extension" << std::endl;
    return {false, mcpl_outfile_t{nullptr}};
  }
  bool try_gzip = filename.find(".mcpl.gz") != std::string::npos;
  std::string without_gz = filename;
  if (try_gzip) {
    without_gz = filename.substr(0, filename.find(".gz"));
  }

  mcpl_outfile_t mcpl_out_file = mcpl_create_outfile(without_gz.c_str());
  if (!mcpl_out_file.internal) {
    std::cerr << "Error creating output file: " << without_gz << std::endl;
    return {false, mcpl_out_file};
  }
  // Add any additional headers or metadata here if needed
  return {try_gzip, mcpl_out_file};
}

/// \brief Use the MCPL way to determine platform endian-ness
static int mcpl_platform_is_little_endian() {
  //Return 0 for big endian, 1 for little endian.
  volatile uint32_t i=0x01234567;
  return (*((uint8_t*)(&i))) == 0x67;
}

/// \brief Transfer metadata from an open input file to an open output file, skipping the special renormalize key
void transfer_metadata(mcpl_file_t source, mcpl_outfile_t target)
{
  //Note that MCPL format version 2 and 3 have the same meta-data in the header,
  //except of course the version number itself.

  if (mcpl_hdr_little_endian(source) != mcpl_platform_is_little_endian()) {
    std::cerr << "mcpl_transfer_metadata can only work on files with same endianness as current platform." << std::endl;
  }
  mcpl_hdr_set_srcname(target, mcpl_hdr_srcname(source));
  for (unsigned i = 0; i < mcpl_hdr_ncomments(source); ++i) {
    mcpl_hdr_add_comment(target, mcpl_hdr_comment(source, i));
  }
  if (const char** keys = mcpl_hdr_blobkeys(source); keys) {
    std::string skip{renormalize_key()};
    auto blobs = mcpl_hdr_nblobs(source);
    uint32_t ldata;
    const char * data;
    for (int blob = 0; blob < blobs; ++blob) if (skip != keys[blob]) {
        if (mcpl_hdr_blob(source, keys[blob], &ldata, &data) == 0) {
          std::cerr << "Error reading header blob " << keys[blob] << ". Not transferred" << std::endl;
          continue;
        }
        mcpl_hdr_add_data(target, keys[blob], ldata, data);
      }
  }
  if (mcpl_hdr_has_userflags(source)) {
    mcpl_enable_userflags(target);
  }
  if (mcpl_hdr_has_polarisation(source)) {
    mcpl_enable_polarisation(target);
  }
  if (mcpl_hdr_has_doubleprec(source)) {
    mcpl_enable_doubleprec(target);
  }
  if (auto universal_pdg_code = mcpl_hdr_universal_pdgcode(source); universal_pdg_code) {
    mcpl_enable_universal_pdgcode(target, universal_pdg_code);
  }
  if (auto uw = mcpl_hdr_universal_weight(source); uw != 0.0) {
    mcpl_enable_universal_weight(target, uw);
  }
}



int merge_files(const std::string & output, const std::vector<std::string> & input){
  if (input.empty()) {
    std::cerr << "No input files provided for merging." << std::endl;
    return -1;
  }

  // Create the output file
  auto [try_gz, outfile] = create_output_mcpl_file(output);

  // open all the input files:
  int unopened{0};
  std::vector<mcpl_file_t> input_files;
  for (const auto &filename : input) {
    mcpl_file_t mcpl_file = mcpl_open_file(filename.c_str());
    if (!mcpl_file.internal) {
      std::cerr << "Error opening input file: " << filename << std::endl;
      ++unopened;
      continue;
    }
    input_files.push_back(mcpl_file);
  }

  // find the total number of particles used at start of each input file's simulation
  int64_t count = std::accumulate(input_files.begin(), input_files.end(), 0LL,
                                  [](int64_t sum, const mcpl_file_t &file) {
                                    return sum + get_renormalize_particle_count(file);
                                  });

  // If no valid particle counts were found, the following normalization will not work, so skip combining files
  if (count == 0) {
    std::cerr << "No valid particle counts found in input files." << std::endl;
    for (const auto &file : input_files) {
      mcpl_close_file(file);
    }
    mcpl_close_outfile(outfile);
    return -10 - unopened;
  }

  // Copy header metadata from the first input file -- skips the particle count, since it can't be overwritten
  transfer_metadata(input_files[0], outfile);
  // Then set the particle count in the output file
  set_renormalize_particle_count(outfile, count);

  // Loop through each input file and read particles, re-normalize their weights, and add them to the output file.
  auto particle = mcpl_get_empty_particle(outfile);
  for (const auto &file : input_files) {
    auto file_norm = static_cast<double>(get_renormalize_particle_count(file)) / static_cast<double>(count);
    const mcpl_particle_t * read;
    while (nullptr != (read = mcpl_read(file))) {
      std::memcpy(particle, read, sizeof(mcpl_particle_t));
      particle->weight *= file_norm;
      mcpl_add_particle(outfile, particle);
    }
    // done with this file, so it can be closed
    mcpl_close_file(file);
  }

  if (try_gz && !mcpl_closeandgzip_outfile(outfile)){
    std::cerr << "Error closing and compressing output file; un-compressed file left alone: " << output << std::endl;
  } else if (!try_gz) {
    // only need to close the file if not compressing, because MCPL closed it already otherwise.
    mcpl_close_outfile(outfile);
  }

  return unopened;
}