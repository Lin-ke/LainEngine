#pragma once
 
#ifndef Tuple_H
#define Tuple_H
#include "base.h"
#include <type_traits>
namespace lain {
	template<typename _tp>
	using is_heritable = typename std::conditional<std::is_final<_tp>::value, std::true_type, std::is_fundamental<_tp>>::type;
	template <int32_t _Idx, typename ... Elements>
	struct _Tuple_impl;
	template <int32_t _Idx,
		typename _Head, bool = is_heritable<_Head>::value>
	struct _Head_warp;
	// 继承
    template<int32_t _Idx, typename _Head>
    struct _Head_warp <_Idx, _Head, true>
    {
         _Head_warp()
            : _M_head_impl() { }

         _Head_warp(const _Head& __h)
            : _M_head_impl(__h) { }

         _Head_warp(const _Head_warp&) = default; 
         _Head_warp(_Head_warp&&) = default;
        static  _Head&
            _M_head(_Head_warp& __b) noexcept { return __b._M_head_impl; }

        static  const _Head&
            _M_head(const _Head_warp& __b) noexcept { return __b._M_head_impl; }

        _Head _M_head_impl;
    };
    // 不继承
    template<int32_t _Idx, typename _Head>
    struct _Head_warp<_Idx, _Head, false>
        : public _Head
    {
         _Head_warp()
            : _Head() { }

         _Head_warp(const _Head& __h)
            : _Head(__h) { }

         _Head_warp(const _Head_warp&) = default;
         _Head_warp(_Head_warp&&) = default;

        template<typename _UHead>
         _Head_warp(_UHead&& __h)
            : _Head(std::forward<_UHead>(__h)) { }

        static  _Head&
            _M_head(_Head_warp& __b) noexcept { return __b; }

        static  const _Head&
            _M_head(const _Head_warp& __b) noexcept { return __b; }
    };

    template <int32_t _Idx, typename _Head, typename... _Tail>
    struct _Tuple_impl<_Idx, _Head, _Tail...>
        : public _Tuple_impl<_Idx + 1, _Tail...>, // father
        private _Head_warp<_Idx, _Head>         // 本级
    {
        typedef _Tuple_impl<_Idx + 1, _Tail...>  _Inherited;  // 父类
        typedef _Head_warp<_Idx, _Head>         _Base;        // 当前节点类型        

        // 默认构造函数
         _Tuple_impl() : _Inherited(), _Base() // 倒序，先父类，所以节点越往后越先分配
        { }

        // 逐节点复制
        explicit  _Tuple_impl(const _Head& __head, const _Tail &...__tail)
            : _Inherited(__tail...), _Base(__head)
        { }

        // 逐节点移动
        template <typename _UHead,
            typename... _UTail,
            typename = typename std::enable_if<sizeof...(_Tail) == sizeof...(_UTail)>::type> // 元素个数要一致
        explicit  _Tuple_impl(_UHead&& __head, _UTail &&...__tail)
            : _Inherited(std::forward<_UTail>(__tail)...),
            _Base(std::forward<_UHead>(__head))
        { }

        static  _Head&
            _M_head(_Tuple_impl& __t) noexcept { return _Base::_M_head(__t); }

        static  const _Head&
            _M_head(const _Tuple_impl& __t) noexcept { return _Base::_M_head(__t); }

        static  _Inherited&
            _M_tail(_Tuple_impl& __t) noexcept { return __t; }

        static  const _Inherited&
            _M_tail(const _Tuple_impl& __t) noexcept { return __t; }
        _Tuple_impl&
            operator=(const _Tuple_impl& __in)
        {
            _M_head(*this) = _M_head(__in);
            _M_tail(*this) = _M_tail(__in);
            return *this;
        }

        _Tuple_impl&
            operator=(_Tuple_impl&& __in)
            noexcept(__and_<is_nothrow_move_assignable<_Head>,
                is_nothrow_move_assignable<_Inherited>>::value)
        {
            _M_head(*this) = std::forward<_Head>(_M_head(__in));
            _M_tail(*this) = std::move(_M_tail(__in));
            return *this;
        }

        template<typename... _UElements>
        _Tuple_impl&
            operator=(const _Tuple_impl<_Idx, _UElements...>& __in)
        {
            _M_head(*this) = _Tuple_impl<_Idx, _UElements...>::_M_head(__in);
            _M_tail(*this) = _Tuple_impl<_Idx, _UElements...>::_M_tail(__in);
            return *this;
        }

        template<typename _UHead, typename... _UTails>
        _Tuple_impl&
            operator=(_Tuple_impl<_Idx, _UHead, _UTails...>&& __in)
        {
            _M_head(*this) = std::forward<_UHead>
                (_Tuple_impl<_Idx, _UHead, _UTails...>::_M_head(__in));
            _M_tail(*this) = std::move
            (_Tuple_impl<_Idx, _UHead, _UTails...>::_M_tail(__in));
            return *this;
        }

    };

    // 最后一个节点，_Head 是最后一个节点的类型
    template <int32_t _Idx, typename _Head>
    struct _Tuple_impl<_Idx, _Head> : private _Head_warp<_Idx, _Head>
    {
        template<int32_t, typename...> friend struct _Tuple_impl;

        typedef _Head_warp<_Idx, _Head>         _Base;       // 用于构造当前节点   


        // 构造函数
         _Tuple_impl() : _Base()
        { }

        // 复制当前节点
        explicit  _Tuple_impl(const _Head& __head) : _Base(__head)
        { }

        // 移动当前节点
        template <typename _UHead>
        explicit  _Tuple_impl(_UHead&& __head)
            : _Base(std::forward<_UHead>(__head))
        { }

        static  _Head&
            _M_head(_Tuple_impl& __t) noexcept { return _Base::_M_head(__t); }

        static  const _Head&
            _M_head(const _Tuple_impl& __t) noexcept { return _Base::_M_head(__t); }
        //...
    };

    

    template < typename... _Elements>
               class Tuple : public _Tuple_impl < 0, _Elements...>
         {
         typedef _Tuple_impl<0, _Elements...> _Inherited;
         
         public:
            Tuple()
       : _Inherited() { }
 
       explicit
       Tuple(const _Elements&... __elements)
 : _Inherited(__elements...) { }
 
       template < typename... _UElements>
         explicit
         Tuple(_UElements&&... __elements)
     : _Inherited(std::forward<_UElements>(__elements)...) { }
 
       Tuple(const Tuple & __in)
 : _Inherited(static_cast<const _Inherited&>(__in)) { }
 
       Tuple(Tuple && __in)
 : _Inherited(static_cast<_Inherited&&>(__in)) { }
 
       template<typename... _UElements>
         Tuple(const Tuple<_UElements...>&__in)
     : _Inherited(static_cast<const _Tuple_impl<0, _UElements...>&>(__in))
     { }
 
       template < typename... _UElements>
         Tuple(Tuple<_UElements...> && __in)
 : _Inherited(static_cast<_Tuple_impl<0, _UElements...>&&>(__in)) { }
 
       // XXX http://gcc.gnu.org/ml/libstdc++/2008-02/msg00047.html
       template<typename... _UElements>
         Tuple(Tuple<_UElements...>&__in)
     : _Inherited(static_cast<const _Tuple_impl<0, _UElements...>&>(__in))
     { }
 
       Tuple &
       operator=(const Tuple & __in)
       {
     static_cast<_Inherited&>(*this) = __in;
     return *this;
       }
 
       Tuple &
       operator=(Tuple && __in)
       {
     static_cast<_Inherited&>(*this) = std::move(__in);
     return *this;
       }
 
       template < typename... _UElements>
         Tuple &
         operator=(const Tuple<_UElements...>&__in)
         {
       static_cast<_Inherited&>(*this) = __in;
       return *this;
     }
 
       template<typename... _UElements>
         Tuple&
         operator=(Tuple<_UElements...>&& __in)
         {
       static_cast<_Inherited&>(*this) = std::move(__in);
       return *this;
     }
    };

    template<typename... _Elements>
     Tuple<typename _Elements...>
        make_Tuple(_Elements&&... __args)
    {
        typedef Tuple<typename _Elements...>
            __result_type;
        return __result_type(std::forward<_Elements>(__args)...);
    }

    template<int32_t __i, typename _Head, typename... _Tail>
    struct Tuple_element;

    template<int32_t __i, typename _Head, typename... _Tail>
    struct Tuple_element<__i, Tuple<_Head, _Tail...> >
        : Tuple_element<__i - 1, Tuple<_Tail...> > { };

    template<typename _Head, typename... _Tail>
    struct Tuple_element<0, Tuple<_Head, _Tail...> >
    {
        typedef _Head type; // public继承
    };
    template<size_t __i>
    struct Tuple_element<__i, Tuple<>>
    {
        static_assert(__i < 0,
            "Tuple index is in range");
    };


    template<int32_t __i, typename _Head, typename... _Tail>
     _Head&
        __Tuple_get_helper(_Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }

    template<int32_t __i, typename _Head, typename... _Tail>
     const _Head&
        __Tuple_get_helper(const _Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }
    template<typename _Head, int32_t __i, typename... _Tail>
     _Head&
        __Tuple_get_helper2(_Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }

    template<typename _Head, int32_t __i, typename... _Tail>
     const _Head&
        __Tuple_get_helper2(const _Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }
    /// <summary>
    /// get function
    /// </summary>
    /// <typeparam name="_Tp">index</typeparam>
    /// <typeparam name="..._Types">通过类型推导</typeparam>
    /// <param name="__t">Tuple</param>
    /// <returns></returns>
    template <typename _Tp, typename... _Types>
     _Tp&
        __Tuple_get(Tuple<_Types...>& __t) noexcept
    {
        return __Tuple_get_helper2<_Tp>(__t);
    }
    template <typename _Tp, typename... _Types>
     const _Tp&
        __Tuple_get(const Tuple<_Types...>& __t) noexcept
    {
        return __Tuple_get_helper2<_Tp>(__t);
    }

    template <typename _Tp, typename... _Types>
     _Tp&&
        __Tuple_get(Tuple<_Types...>&& __t) noexcept
    {
        return std::forward<_Tp&&>(__Tuple_get_helper2<_Tp>(__t));
    }


    /// Return a reference to the ith element of a Tuple.
    template<int32_t __i, typename... _Elements>
     typename Tuple_element<__i, Tuple<_Elements...>>::type&
        __Tuple_get(Tuple<_Elements...>& __t) noexcept
    {
        return __Tuple_get_helper<__i>(__t);
    }

    /// Return a const reference to the ith element of a const Tuple.
    template<int32_t __i, typename... _Elements>
     const typename Tuple_element<__i, Tuple<_Elements...>>::type&
        __Tuple_get(const Tuple<_Elements...>& __t) noexcept
    {
        return __Tuple_get_helper<__i>(__t);
    }

    /// Return an rvalue reference to the ith element of a Tuple rvalue.
    template<int32_t __i, typename... _Elements>
     typename Tuple_element<__i, Tuple<_Elements...>>::type&&
        __Tuple_get(Tuple<_Elements...>&& __t) noexcept
    {
        typedef Tuple_element<__i, Tuple<_Elements...>> __element_type;
        return std::forward<__element_type&&>(std::get<__i>(__t));
    }

    /**
  * Error case for Tuple_element: invalid index.
  */

    template<typename _Tp, typename _Up, size_t __i, size_t __size>
    struct __Tuple_compare
    {
        static  bool
            __eq(const _Tp& __t, const _Up& __u)
        {
            return bool(get<__i>(__t) == get<__i>(__u))
                && __Tuple_compare<_Tp, _Up, __i + 1, __size>::__eq(__t, __u);
        }
        static  bool
            __less(const _Tp& __t, const _Up& __u)
        {
            return bool(get<__i>(__t) < get<__i>(__u))
                || (!bool(get<__i>(__u) < get<__i>(__t))
                    && __Tuple_compare<_Tp, _Up, __i + 1, __size>::__less(__t, __u));
        }
    };


    template<typename... _TElements, typename... _UElements>
     bool
        operator==(const Tuple<_TElements...>& __t,
            const Tuple<_UElements...>& __u)
    {
        static_assert(sizeof...(_TElements) == sizeof...(_UElements),
            "Tuple objects can only be compared if they have equal sizes.");
        using __compare = __Tuple_compare<Tuple<_TElements...>,
            Tuple<_UElements...>,
            0, sizeof...(_TElements)>;
        return __compare::__eq(__t, __u);
    }

    template<typename... _TElements, typename... _UElements>
     bool
        operator<(const Tuple<_TElements...>& __t,
            const Tuple<_UElements...>& __u)
    {
        static_assert(sizeof...(_TElements) == sizeof...(_UElements),
            "Tuple objects can only be compared if they have equal sizes.");
        using __compare = __Tuple_compare<Tuple<_TElements...>,
            Tuple<_UElements...>,
            0, sizeof...(_TElements)>;
        return __compare::__less(__t, __u);
    }
}

#endif // !Tuple_H
