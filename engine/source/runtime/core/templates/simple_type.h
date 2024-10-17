#pragma once
#ifndef SIMPLE_TYPE_H
#define SIMPLE_TYPE_H

/* Batch of specializations to obtain the actual simple type */
namespace lain{
template <typename T>
using GetSimpleTypeT = typename std::remove_cv_t<std::remove_reference_t<T>>;


}

#endif // SIMPLE_TYPE_H
