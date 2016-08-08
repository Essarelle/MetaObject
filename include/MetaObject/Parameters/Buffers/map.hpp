/*
Copyright (c) 2015 Daniel Moodie.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
advertising materials, and other materials related to such
distribution and use acknowledge that the software was developed
by the Daniel Moodie. The name of
Daniel Moodie may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

https://github.com/dtmoodie/parameters
*/
#pragma once

#include "MetaObject/Parameters/ITypedInputParameter.hpp"
#include "IBuffer.hpp"
#include <map>

namespace mo
{
    class Context;
    namespace Buffer
    {
        template<typename T> class Map: public ITypedInputParameter<T>, public IBuffer
        {
        public:
            typedef T ValueType;
            Map(const std::string& name = "");

            T*   GetDataPtr(long long ts = -1, Context* ctx = nullptr);
            bool GetData(T& value, long long ts = -1, Context* ctx = nullptr);
            T    GetData(long long ts = -1, Context* ctx = nullptr);
            
            ITypedParameter<T>* UpdateData(T& data_, long long ts = -1, Context* ctx = nullptr);
            ITypedParameter<T>* UpdateData(const T& data_, long long ts = -1, Context* ctx = nullptr);
            ITypedParameter<T>* UpdateData(T* data_, long long ts = -1, Context* ctx = nullptr);

            bool Update(IParameter* other, Context* ctx = nullptr);
            std::shared_ptr<IParameter> DeepCopy() const;

            void SetSize(long long size);
            long long GetSize();
            void GetTimestampRange(long long& start, long long& end);
        protected:
            std::map<long long, T> _data_buffer;
            virtual void onInputUpdate(Context* ctx, IParameter* param);
        };
    }

#define MO_METAPARAMETER_INSTANCE_MAP_(N) \
    template<class T> struct MetaParameter<T, N>: public MetaParameter<T, N-1> \
    { \
        static ParameterConstructor<Buffer::Map<T>, T, Map_e> _map_parameter_constructor; \
        static BufferConstructor<Buffer::Map<T>, Buffer::BufferFactory::map> _map_constructor;  \
        MetaParameter<T, N>(const char* name): \
            MetaParameter<T, N-1>(name) \
        { \
            (void)&_map_parameter_constructor; \
            (void)&_map_constructor; \
        } \
    }; \
    template<class T> ParameterConstructor<Buffer::Map<T>, T, Map_e> MetaParameter<T, N>::_map_parameter_constructor; \
    template<class T> BufferConstructor<Buffer::Map<T>, Buffer::BufferFactory::map> MetaParameter<T, N>::_map_constructor;

    MO_METAPARAMETER_INSTANCE_MAP_(__COUNTER__)
}
#include "detail/MapImpl.hpp"
