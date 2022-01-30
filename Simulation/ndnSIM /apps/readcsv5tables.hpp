#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include <memory>
using namespace std;
class ReadCSV
{
    public:
    vector<string> getMatch(string query, string filePath)
    {
        ifstream _file;

        _file.open(filePath);
        string nonce;
        string name;
        string hash;
        string bin;
        string match_temp;
        vector<string> match;
        vector<int> distance;
        string distance_temp;
        std::string temp;

        //std::string query = "mnist/training/1/img_22854.jpg";
        getline(_file, nonce, ',');
        //int count = 0;
        while(nonce != query)
        {
            //std::cout << count++ << std::endl;
            getline(_file, temp);
            getline(_file, nonce , ',');
        }
        getline(_file, name, ',');
        getline(_file, hash, ',');
        getline(_file, bin, ',');
        //std::cout << name << ", "  << hash << ", " << bin ;
        

        getline(_file, match_temp, ',');
        while(match_temp != "endline")
        {
            getline(_file, distance_temp, ',');
            //std::cout << "  " << match << " " << bin;
            match.push_back(match_temp);
            distance.push_back(std::stoi(distance_temp));
            getline(_file, match_temp, ',');
        }
        vector<string> aa;
        while(distance.size() != 0)
        {
            int ind = std::distance(std::begin(distance), std::min_element(std::begin(distance), std::end(distance)));
            aa.push_back(match[ind]);
            distance.erase(distance.begin()+ind);
            match.erase(match.begin()+ind);
        }

        /*
        for(int i=0; i < aa.size();i++)
        {
            std::cout << aa[i] << std::endl;
        }
        */

        return aa;

        


        /*
        for(int i; i < 5; i++)
        {
            getline(_file, name, ',');
            getline(_file, hash, ',');
            getline(_file, bin, ',');
            std::cout << name << ", "  << hash << ", " << bin ;
            getline(_file, match, ',');
            while(match != "endline")
            {
                getline(_file, bin, ',');
                std::cout << "  " << match << " " << bin;
                getline(_file, match, ',');
            }
            std::cout << std::endl;

        }
        */
    }

    string getImageName(string query, string filePath)
    {
        ifstream _file;

        _file.open(filePath);
        string nonce;
        string name;
        std::string temp;
        getline(_file, nonce, ',');
        while(nonce != query)
        {
            getline(_file, temp);
            getline(_file, nonce , ',');
        }
        getline(_file, name, ',');

        return name;
    }

    void writeNonce(int n)
    {
        std::ofstream out("globalNonce.txt", std::ios_base::app);
        out << std::to_string(n) << ",\n";

    }

    bool globalMatch(int n)
    {
        bool foundmatch = false;
        std::string temp;
        ifstream _file;
        _file.open("globalNonce.txt");
        getline(_file, temp,',');
        while(_file.eof() != true)
        {
            if (std::stoi(temp)  == n)
            {
                foundmatch = true;
                std::cout << "Match found "<< std::endl;
            }
            getline(_file, temp,',');
        }
        

        return foundmatch;

        

    }

};
