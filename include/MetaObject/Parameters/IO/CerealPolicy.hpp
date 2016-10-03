#pragma once
#include <MetaObject/Parameters/IParameter.hpp>
#include "SerializationFunctionRegistry.hpp"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>

#include <functional>

namespace cereal
{
    class BinaryInputArchive;
    class BinaryOutputArchive;
    class XMLOutputArchive;
    class XMLInputArchive;
}

namespace mo
{
    template<class T> class ITypedParameter;
    template<class T, int N, typename Enable = void> struct MetaParameter;
    namespace IO
    {
    namespace Cereal
    {   
        template<class T> struct Policy
        {
            Policy()
            {
                SerializationFunctionRegistry::Instance()->SetBinarySerializationFunctions(
                    TypeInfo(typeid(T)), 
                    std::bind(&Policy<T>::SerializeBinary, std::placeholders::_1, std::placeholders::_2),
                    std::bind(&Policy<T>::DeSerializeBinary, std::placeholders::_1, std::placeholders::_2));

                SerializationFunctionRegistry::Instance()->SetXmlSerializationFunctions(
                    TypeInfo(typeid(T)), 
                    std::bind(&Policy<T>::SerializeXml, std::placeholders::_1, std::placeholders::_2),
                    std::bind(&Policy<T>::DeSerializeXml, std::placeholders::_1, std::placeholders::_2));
            }
            static bool SerializeBinary(IParameter* param, cereal::BinaryOutputArchive& ar)
            {
                ITypedParameter<T>* typed = dynamic_cast<ITypedParameter<T>*>(param);
                T* ptr = typed->GetDataPtr();
                if (ptr)
                    ar(cereal::make_nvp(param->GetName().c_str(), *ptr));
                return true;
            }
            static bool DeSerializeBinary(IParameter* param, cereal::BinaryInputArchive& ar)
            {
                ITypedParameter<T>* typed = dynamic_cast<ITypedParameter<T>*>(param);
                T* ptr = typed->GetDataPtr();
                if(ptr)
                {
                    ar(cereal::make_nvp(param->GetName(), *ptr));
                }
                return true;
            }
            static bool SerializeXml(IParameter* param, cereal::XMLOutputArchive& ar)
            {
                ITypedParameter<T>* typed = dynamic_cast<ITypedParameter<T>*>(param);
                T* ptr = typed->GetDataPtr();
                if (ptr)
                {
                    ar(cereal::make_nvp(param->GetName(), *ptr));
                }
                return true;
            }
            static bool DeSerializeXml(IParameter* param, cereal::XMLInputArchive& ar)
            {
                ITypedParameter<T>* typed = dynamic_cast<ITypedParameter<T>*>(param);
                T* ptr = typed->GetDataPtr();
                if (ptr)
                {
                    ar(cereal::make_nvp(param->GetName(), *ptr));
                }
                return true;
            }
        };
    } // namespace Cereal
    } // namespace IO
#define PARAMETER_SERIALIZATION_POLICY_INST_(N) \
  template<class T> struct MetaParameter<T, N>: public MetaParameter<T, N - 1, void>, public IO::Cereal::Policy<T> \
    { \
        MetaParameter(const char* name): \
            MetaParameter<T, N-1, void>(name){} \
    };

    PARAMETER_SERIALIZATION_POLICY_INST_(__COUNTER__)
}
