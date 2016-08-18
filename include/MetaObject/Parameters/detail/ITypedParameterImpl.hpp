#pragma once

namespace mo
{
	template<typename T> class ITypedParameter;

	template<typename T> ITypedParameter<T>::ITypedParameter(const std::string& name, ParameterType flags, long long ts, Context* ctx) :
		IParameter(name, flags, ts, ctx)
	{
	}

	template<typename T> const TypeInfo& ITypedParameter<T>::GetTypeInfo() const
	{
		return _type_info;
	}

	template<typename T> bool ITypedParameter<T>::Update(IParameter* other)
	{
		auto typedParameter = dynamic_cast<ITypedParameter<T>*>(other);
		if (typedParameter)
		{
			std::lock_guard<std::recursive_mutex> lock(typedParameter->mtx());
			UpdateData(typedParameter->GetData(), other->GetTimestamp(), other->GetContext());
			OnUpdate(other->GetContext());
			return true;
		}
		return false;
	}
    template<typename T> const TypeInfo ITypedParameter<T>::_type_info(typeid(T));
}