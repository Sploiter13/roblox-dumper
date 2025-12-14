#pragma once
#include <Windows.h>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct RttiInfo {
    std::string name;
    uintptr_t type_descriptor;
    uintptr_t class_hierarchy_descriptor;
    std::vector<std::string> base_classes;
};

// so sorry for people who hate forward declaring :(
class Memory;

struct TypeDescriptor {
    uintptr_t pVFTable;
    uintptr_t spare;
    char name[256];
};

struct PMD {
    int mdisp;
    int pdisp;
    int vdisp;
};

struct RTTIBaseClassDescriptor {
    uintptr_t pTypeDescriptor;
    DWORD numContainedBases;
    PMD where;
    DWORD attributes;
};

struct RTTIClassHierarchyDescriptor {
    DWORD signature;
    DWORD attributes;
    DWORD numBaseClasses;
    uintptr_t pBaseClassArray;
};

struct RTTICompleteObjectLocator {
    DWORD signature;
    DWORD offset;
    DWORD cdOffset;
    uintptr_t pTypeDescriptor;
    uintptr_t pClassDescriptor;
    uintptr_t pSelf;
};
