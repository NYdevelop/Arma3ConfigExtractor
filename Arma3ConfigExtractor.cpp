#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include <algorithm>
#include <cctype>

#include "rapidxml-1.13\\rapidxml.hpp"
#include "rapidxml-1.13\\rapidxml_utils.hpp"

using namespace std;


std::vector<std::string> split(std::string str, char del) {
    int first = 0;
    int last = str.find_first_of(del);

    std::vector<std::string> result;

    while (first < str.size()) {
        std::string subStr(str, first, last - first);

        result.push_back(subStr);

        first = last + 1;
        last = str.find_first_of(del, first);

        if (last == std::string::npos) {
            last = str.size();
        }
    }

    return result;
}

string removeChar(const string& src, char del)
{
    auto ret = src;
    auto pos = ret.find(del);
    while (pos != string::npos)
    {
        ret = ret.erase(pos, 1);
        pos = ret.find(del);
    }
    return ret;
}

enum ELoadState
{
    none = 0,
    foundClass,
    inClass,
    inClassFoundArray,
    inClassArray,
};

int main(int argc, char *argv[])
{
    string configFileName;
    string stringTableName("");
    string outputName("output");
    if (argc >= 2) configFileName  = argv[1];
    if (argc >= 3) stringTableName = argv[2];
    if (argc >= 4) outputName = argv[3];

    // êÊÇ…stringtableÇâêÕ
    map<string, string> stringTableMap;
    if (stringTableName != "")
    {
        rapidxml::file<> input(stringTableName.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<rapidxml::parse_trim_whitespace>(input.data());

        for (auto pakageNode = doc.first_node()->first_node(); pakageNode != nullptr; pakageNode = pakageNode->next_sibling())
        {
            cout << pakageNode->first_attribute()->value() << endl;
            for (auto node = pakageNode->first_node()->first_node(); node != nullptr; node = node->next_sibling())
            {
string key(node->first_attribute()->value());
std::transform(key.begin(), key.end(), key.begin(), std::toupper);
stringTableMap["$" + key] = node->first_node()->value();
            }
        }

        //for each (auto item in stringTableMap)
        //{
        //    cout << "[" << item.first << "] = " << item.second << endl;
        //}
    }

    // configÇâêÕ
    map<string, map<string, string> > classConfigMap;
    map<string, string> classMap;
    ifstream configFile(configFileName);
    string line;
    string className;
    string parentName;
    ELoadState state = ELoadState::none;
    while (getline(configFile, line))
    {

        switch (state)
        {
        default:
        {
            auto pos = line.find("class ");
            if (pos != string::npos)
            {
                string inClassNameStr = line;
                parentName = "";
                auto elems = split(line, ':');
                if (elems.size() >= 2)
                {
                    inClassNameStr = elems[0];
                    parentName = removeChar(elems[1], ' ');
                }

                auto startPos = pos + 6;
                className = inClassNameStr.substr(startPos, inClassNameStr.size() - startPos);

                if (className == "CfgMagazines" || className == "CfgWeapons")
                {
                    break;
                }

                state = foundClass;
            }

        }
        break;

        case foundClass:
            if (line.find("{") != string::npos)
            {
                if (parentName.size() > 0) classMap["parent"] = parentName;

                state = inClass;
                break;
            }

            state = none;
            className.clear();

            break;

        case inClass:
        {
            if (line.find("};") != string::npos)
            {
                classConfigMap[className] = classMap;
                classMap = map<string, string>();

                state = none;
                break;
            }

            auto elems = split(line, '=');
            if (elems.size() == 1)
            {
                state = inClassFoundArray;
                break;
            }
            if (elems[0].find("class ") != string::npos)
            {
                if (elems[0].find(";") == string::npos) state = inClassFoundArray;
                break;
            }

            auto valueStr = removeChar(elems[1], '"');
            valueStr = valueStr.erase(valueStr.length() - 1);


            // íuä∑
            auto key = removeChar(elems[0], '\t');
            if (key == "displayName")
            {
                std::transform(valueStr.begin(), valueStr.end(), valueStr.begin(), std::toupper);
                if (stringTableMap.find(valueStr) != stringTableMap.end())
                {
                    valueStr = stringTableMap[valueStr];
                }
            }
            classMap[key] = valueStr;
        }
            break;

        case inClassFoundArray:
            if (line.find("{") == string::npos)
            {
                throw "error";
            }
            state = inClassArray;

            break;

        case inClassArray:
            if (line.find("};") != string::npos)
            {
                state = inClass;
            }
            break;
        }
    }

    // èoóÕ
    //for each (auto item in classConfigMap)
    //{
    //    cout << item.first << endl;
    //    for each (auto i in item.second)
    //    {
    //        cout << "  " << i.first << " = " << i.second << endl;
    //    }
    //}

    ofstream csv(outputName + ".csv");
    for each (auto item in classConfigMap)
    {
        if (item.second.find("displayName") != item.second.end() && item.second.find("picture") != item.second.end())
        {
            csv << item.second["displayName"] << "," << item.first << "," << item.second["picture"];
            if (item.second.find("parent") != item.second.end())
            {
                csv << "," << item.second["parent"];
            }
            csv << endl;
        }
    }

    return 0;
}

