#include <iostream>
#include <algorithm>
#include <numeric>
#include "renormalize.h"
#include "merge.h"


const char * renormalize_key() {
  return "mccode_neutron_count";
}


///\brief Decode a 64-bit integer from a blob of data
/// \param ldata Length of the data blob -- should be sizeof(uint64_t)
/// \param data Pointer to the data blob -- should be 'network ordered' bytes
/// \return The decoded 64-bit integer
///
uint64_t decode_int64t_data(uint32_t ldata, const char * data){
  uint64_t value = 0;
  if (ldata != sizeof(uint64_t)) {
    std::cerr << "Error: data length mismatch" << std::endl;
    return -1;
  }
  for (size_t i = 0; i < sizeof(int64_t); i++) {
    value |= ((uint64_t)(unsigned char)data[i]) << ((8 - i) * 8);
  }
  return value;
}


///\brief Encode a 64-bit integer into a blob of data
/// \param ldata Length of the data blob -- should be at least sizeof(uint64_t)
/// \param data Pointer to the data blob -- will contain 'network ordered' bytes
/// \param value The 64-bit integer to encode
void encode_int64t_data(uint32_t ldata, char * data, uint64_t value){
  if (ldata != sizeof(uint64_t)) {
    std::cerr << "Error: data length mismatch" << std::endl;
    return;
  }
  for (size_t i = 0; i < sizeof(int64_t); i++) {
    data[i] = static_cast<char>((value >> ((8 - i) * 8)) & 0xFF);
  }
}



uint64_t get_renormalize_particle_count(mcpl_file_t mcpl_file){
  // Check if the file contains the mccode_neutron_count header
  auto n_blobs = mcpl_hdr_nblobs(mcpl_file);
  const auto keys = mcpl_hdr_blobkeys(mcpl_file);
  std::string needed{renormalize_key()};
  if (!std::any_of(keys,  keys+n_blobs, [&needed](const char* key) {return needed == key;})) {
    std::cerr << "File does not contain " << needed << " header"  << std::endl;
    return 0;
  }

  uint32_t ldata;
  const char * data;
  int result = mcpl_hdr_blob(mcpl_file, needed.c_str(), &ldata, &data);
  if (result == 0 || ldata != sizeof(uint64_t) || data == nullptr) {
    std::cerr << "Error reading header " << needed << " blob from file" << std::endl;
    return 0;
  }
  uint64_t count = decode_int64t_data(ldata, data);
  return count;
}


int set_renormalize_particle_count(mcpl_outfile_t file, uint64_t count){
  char *data_out = (char *)malloc(sizeof(uint64_t));
  if (data_out == nullptr) {
    std::cerr << "Memory allocation failed" << std::endl;
    return -1;
  }
  encode_int64t_data(sizeof(uint64_t), data_out, count);
  mcpl_hdr_add_data(file, renormalize_key(), sizeof(uint64_t), data_out);
  free(data_out);
  return 0;
}


int renormalize_merge_files(const char * output, const char ** input, size_t count){
  std::vector<std::string> input_files;
  input_files.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    input_files.emplace_back(input[i]);
  }
  return merge_files(output, input_files);
}


