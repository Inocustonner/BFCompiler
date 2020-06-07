#include <vector>
#include <string>

struct Bucket;
using unresd_name = std::pair<std::string, size_t>;

void CoffGenerate(FILE* fp, const Bucket* buck, const std::vector<unresd_name>& unresolved_v);
