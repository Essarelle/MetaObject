#pragma once
#include "MetaObject/Detail/HelperMacros.hpp"
#include "MetaObject/Parameters/Demangle.hpp"
namespace mo
{
    template<class T, int N> struct MetaParameter: public MetaParameter<T, N-1>
    {
        MetaParameter(const char* name):
            MetaParameter<T, N-1>(name){}
    };
    template<class T> struct MetaParameter<T, 0>
    {
        MetaParameter(const char* name)
        {
            mo::Demangle::RegisterName(mo::TypeInfo(typeid(T)), name);
        }
    };
}

#define INSTANTIATE_META_PARAMETER(TYPE) \
static MetaParameter<TYPE, __COUNTER__> COMBINE(g_meta_parameter, __LINE__)(#TYPE);
