#include"readcsv5tables.hpp"
using namespace std;

int main()
{
    ReadCSV r;

    r.writeNonce(787);
    r.writeNonce(33);
    //r.globalMatch(787);
    r.globalMatch(33);
    remove("/home/malazad@unomaha.edu/ndnSIM_rFIB/ns-3/src/ndnSIM/apps/globalNonce.txt");
    return 0;
}