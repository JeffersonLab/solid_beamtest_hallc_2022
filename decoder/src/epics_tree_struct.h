#ifndef EPICS_TREE_STRUCT_H
#define EPICS_TREE_STRUCT_H

#include <unordered_map>
#include <string>

namespace epics_struct {

struct EPICStruct
{
    int event_number;
    std::string timestamp;
    std::unordered_map<std::string, float> data;

    EPICStruct() :
        event_number(0), timestamp("")
    {
        data.clear();
    }
};

EPICStruct * __g_epic_data;

}

#endif
