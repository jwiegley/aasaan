#include "arabic.h"

#include <vector>
#include <map>

class dictionary_t
{
  struct substr_t {
    const char * ptr;
    int len;
    int page;
  };

  typedef std::vector<arabic::element_t> element_vector;
  
  typedef std::map<element_vector, substr_t>  dict_map;
  typedef std::pair<element_vector, substr_t> dict_pair;

  const char * data;
  dict_map     entries;
};

void build_index(const std::string& path)
{
}
