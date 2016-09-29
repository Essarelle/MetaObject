#include "MetaObject/Parameters/MetaParameter.hpp"
#include "MetaObject/Parameters/UI/Qt/OpenCV.hpp"
#include "MetaObject/Parameters/UI/Qt/Containers.hpp"
#include "MetaObject/Parameters/UI/Qt/TParameterProxy.hpp"
#include "MetaObject/Parameters/Buffers/CircularBuffer.hpp"
#include "MetaObject/Parameters/Buffers/StreamBuffer.hpp"
#include "MetaObject/Parameters/Buffers/map.hpp"

INSTANTIATE_META_PARAMETER(std::vector<int>);
INSTANTIATE_META_PARAMETER(std::vector<ushort>);
INSTANTIATE_META_PARAMETER(std::vector<uint>);
INSTANTIATE_META_PARAMETER(std::vector<char>);
INSTANTIATE_META_PARAMETER(std::vector<uchar>);
INSTANTIATE_META_PARAMETER(std::vector<float>);
INSTANTIATE_META_PARAMETER(std::vector<double>);