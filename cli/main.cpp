
#include <popl/popl.hpp>

using namespace popl;

int main(int argc, char** argv)
{
    OptionParser op("vico [USAGE]");
    auto help_option   = op.add<Switch>("h", "help", "produce help message");
    op.parse(argc, argv);

    if (argc == 1 || help_option->is_set()) {
        std::cout << op << "\n";
    }

}
