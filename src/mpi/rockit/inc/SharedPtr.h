/*
 * SharedPtr.h
 *
 *  Created on: 2015年11月21日
 *      Author: terry
 */

#ifndef SHAREDPTR_H_
#define SHAREDPTR_H_


#include <memory>

#if __cplusplus >= 201103L

#else

#if defined(_MSC_VER) && (_MSC_VER >= 1500)
    namespace std
    {
        using std::tr1::shared_ptr;
        using std::tr1::enable_shared_from_this;
    }
    #define FOUND_SHARED_PTR

#endif //_MSC_VER


#if defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)
#include <tr1/memory>
namespace std
{
	using std::tr1::shared_ptr;
	using std::tr1::enable_shared_from_this;
}
#define FOUND_SHARED_PTR
#endif //


#endif //








#endif /* SHAREDPTR_H_ */
