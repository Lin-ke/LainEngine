#pragma once
 
#ifndef TUPLE_H
#define TUPLE_H
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
	// �̳�
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
    // ���̳�
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
        private _Head_warp<_Idx, _Head>         // ����
    {
        typedef _Tuple_impl<_Idx + 1, _Tail...>  _Inherited;  // ����
        typedef _Head_warp<_Idx, _Head>         _Base;        // ��ǰ�ڵ�����        

        // Ĭ�Ϲ��캯��
         _Tuple_impl() : _Inherited(), _Base() // �����ȸ��࣬���Խڵ�Խ����Խ�ȷ���
        { }

        // ��ڵ㸴��
        explicit  _Tuple_impl(const _Head& __head, const _Tail &...__tail)
            : _Inherited(__tail...), _Base(__head)
        { }

        // ��ڵ��ƶ�
        template <typename _UHead,
            typename... _UTail,
            typename = typename std::enable_if<sizeof...(_Tail) == sizeof...(_UTail)>::type> // Ԫ�ظ���Ҫһ��
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

    // ���һ���ڵ㣬_Head �����һ���ڵ������
    template <int32_t _Idx, typename _Head>
    struct _Tuple_impl<_Idx, _Head> : private _Head_warp<_Idx, _Head>
    {
        template<int32_t, typename...> friend struct _Tuple_impl;

        typedef _Head_warp<_Idx, _Head>         _Base;       // ���ڹ��쵱ǰ�ڵ�   


        // ���캯��
         _Tuple_impl() : _Base()
        { }

        // ���Ƶ�ǰ�ڵ�
        explicit  _Tuple_impl(const _Head& __head) : _Base(__head)
        { }

        // �ƶ���ǰ�ڵ�
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

    

    template <typename... _Elements>
    class Tuple : public _Tuple_impl<0, _Elements...> {
        typedef _Tuple_impl<0, _Elements...> _Inherited;
    public:

         Tuple():_Inherited(){}

        
        explicit  Tuple(const _Elements&... __elements)
            : _Inherited(__elements...) { }
         Tuple(const Tuple&) = default;
         Tuple(Tuple&&) = default;

         template<typename... _UElements>
         constexpr Tuple(const Tuple<_UElements...>& __in)
             : _Inherited(static_cast<const _Tuple_impl<0, _UElements...>&>(__in))
         { }
         template<typename... _UElements>
         constexpr Tuple(Tuple<_UElements...>&& __in)
             : _Inherited(static_cast<_Tuple_impl<0, _UElements...>&&>(__in)) { }
        
        
    };

    template<typename... _Elements>
     Tuple<typename _Elements...>
        make_tuple(_Elements&&... __args)
    {
        typedef Tuple<typename _Elements...>
            __result_type;
        return __result_type(std::forward<_Elements>(__args)...);
    }

    template<int32_t __i, typename _Head, typename... _Tail>
    struct tuple_element;

    template<int32_t __i, typename _Head, typename... _Tail>
    struct tuple_element<__i, Tuple<_Head, _Tail...> >
        : tuple_element<__i - 1, Tuple<_Tail...> > { };

    template<typename _Head, typename... _Tail>
    struct tuple_element<0, Tuple<_Head, _Tail...> >
    {
        typedef _Head type; // public�̳�
    };
    template<size_t __i>
    struct tuple_element<__i, Tuple<>>
    {
        static_assert(__i < 0,
            "tuple index is in range");
    };


    template<int32_t __i, typename _Head, typename... _Tail>
     _Head&
        __tuple_get_helper(_Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }

    template<int32_t __i, typename _Head, typename... _Tail>
     const _Head&
        __tuple_get_helper(const _Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }
    template<typename _Head, int32_t __i, typename... _Tail>
     _Head&
        __tuple_get_helper2(_Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }

    template<typename _Head, int32_t __i, typename... _Tail>
     const _Head&
        __tuple_get_helper2(const _Tuple_impl<__i, _Head, _Tail...>& __t) noexcept
    {
        return _Tuple_impl<__i, _Head, _Tail...>::_M_head(__t);
    }
    /// <summary>
    /// get function
    /// </summary>
    /// <typeparam name="_Tp">index</typeparam>
    /// <typeparam name="..._Types">ͨ�������Ƶ�</typeparam>
    /// <param name="__t">tuple</param>
    /// <returns></returns>
    template <typename _Tp, typename... _Types>
     _Tp&
        __tuple_get(Tuple<_Types...>& __t) noexcept
    {
        return __tuple_get_helper2<_Tp>(__t);
    }
    template <typename _Tp, typename... _Types>
     const _Tp&
        __tuple_get(const Tuple<_Types...>& __t) noexcept
    {
        return __tuple_get_helper2<_Tp>(__t);
    }

    template <typename _Tp, typename... _Types>
     _Tp&&
        __tuple_get(Tuple<_Types...>&& __t) noexcept
    {
        return std::forward<_Tp&&>(__tuple_get_helper2<_Tp>(__t));
    }


    /// Return a reference to the ith element of a tuple.
    template<int32_t __i, typename... _Elements>
     typename tuple_element<__i, Tuple<_Elements...>>::type&
        __tuple_get(Tuple<_Elements...>& __t) noexcept
    {
        return __tuple_get_helper<__i>(__t);
    }

    /// Return a const reference to the ith element of a const tuple.
    template<int32_t __i, typename... _Elements>
     const typename tuple_element<__i, Tuple<_Elements...>>::type&
        __tuple_get(const Tuple<_Elements...>& __t) noexcept
    {
        return __tuple_get_helper<__i>(__t);
    }

    /// Return an rvalue reference to the ith element of a tuple rvalue.
    template<int32_t __i, typename... _Elements>
     typename tuple_element<__i, Tuple<_Elements...>>::type&&
        __tuple_get(Tuple<_Elements...>&& __t) noexcept
    {
        typedef tuple_element<__i, Tuple<_Elements...>> __element_type;
        return std::forward<__element_type&&>(std::get<__i>(__t));
    }

    /**
  * Error case for tuple_element: invalid index.
  */

    template<typename _Tp, typename _Up, size_t __i, size_t __size>
    struct __tuple_compare
    {
        static  bool
            __eq(const _Tp& __t, const _Up& __u)
        {
            return bool(get<__i>(__t) == get<__i>(__u))
                && __tuple_compare<_Tp, _Up, __i + 1, __size>::__eq(__t, __u);
        }
        static  bool
            __less(const _Tp& __t, const _Up& __u)
        {
            return bool(get<__i>(__t) < get<__i>(__u))
                || (!bool(get<__i>(__u) < get<__i>(__t))
                    && __tuple_compare<_Tp, _Up, __i + 1, __size>::__less(__t, __u));
        }
    };


    template<typename... _TElements, typename... _UElements>
     bool
        operator==(const Tuple<_TElements...>& __t,
            const Tuple<_UElements...>& __u)
    {
        static_assert(sizeof...(_TElements) == sizeof...(_UElements),
            "tuple objects can only be compared if they have equal sizes.");
        using __compare = __tuple_compare<tuple<_TElements...>,
            tuple<_UElements...>,
            0, sizeof...(_TElements)>;
        return __compare::__eq(__t, __u);
    }

    template<typename... _TElements, typename... _UElements>
     bool
        operator<(const Tuple<_TElements...>& __t,
            const Tuple<_UElements...>& __u)
    {
        static_assert(sizeof...(_TElements) == sizeof...(_UElements),
            "tuple objects can only be compared if they have equal sizes.");
        using __compare = __tuple_compare<tuple<_TElements...>,
            tuple<_UElements...>,
            0, sizeof...(_TElements)>;
        return __compare::__less(__t, __u);
    }
}

#endif // !TUPLE_H
