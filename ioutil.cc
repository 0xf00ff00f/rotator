#include "ioutil.h"

#include <fstream>

namespace Util
{
std::optional<std::vector<unsigned char>> readFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return {};

    auto *buf = file.rdbuf();

    const std::size_t size = buf->pubseekoff(0, file.end, file.in);
    buf->pubseekpos(0, file.in);

    std::vector<unsigned char> data(size);
    buf->sgetn(reinterpret_cast<char *>(data.data()), size);

    file.close();

    return data;
}

} // namespace Util
