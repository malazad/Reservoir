#include "falconn/core/hyperplane_hash.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <vector>
#include <iostream>
#include <random>

using namespace std;


typedef falconn::DenseVector<float> falVec;
vector<uint32_t> GetHash(std::vector<float> &data, int k, int l, int seed)
{

    int dim = data.size();


    falconn::core::HyperplaneHashDense<float> hash(dim, k, l, seed);

    falVec v(dim);
    for (int jj = 0; jj < dim; ++jj) 
    {
        v[jj] = data[jj];
    }
    v.normalize();
    
    vector<uint32_t> result;
    hash.hash(v, &result);
    //return result[0];
    return result;
}

//c++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` getHash.cpp -o HyperplaneHash`python3-config --extension-suffix`
PYBIND11_MODULE(HyperplaneHash, m) {
    m.doc() = "A python wrapper for hyperplane hash";

    m.def("GetHash", &GetHash, "");
}