#pragma once
#ifndef __CALLABLE_H__
#define __CALLABLE_H__
#include "core/string/string_name.h"
#include "core/object/object_id.h"
namespace lain {

	class Object;
	class Variant; 
	class CallableCustom;
	// This is an abstraction of things that can be called.
// It is used for signals and other cases where efficient calling of functions
// is required. It is designed for the standard case (object and method)
// but can be optimized or customized.

// Enforce 16 bytes with `alignas` to avoid arch-specific alignment issues on x86 vs armv7.

	class Callable {
		alignas(8) StringName method;
		union {
			uint64_t object = 0;
			CallableCustom* custom;
		};
	public :
		struct CallError {
			enum Error {
				CALL_OK,
				CALL_ERROR_INVALID_METHOD,
				CALL_ERROR_INVALID_ARGUMENT, // expected is variant type
				CALL_ERROR_TOO_MANY_ARGUMENTS, // expected is number of arguments
				CALL_ERROR_TOO_FEW_ARGUMENTS, // expected is number of arguments
				CALL_ERROR_INSTANCE_IS_NULL,
				CALL_ERROR_METHOD_NOT_CONST,
			};
			Error error = Error::CALL_OK;
			int argument = 0;
			int expected = 0;
		};
		void callp(const Variant** p_arguments, int p_argcount, Variant& r_return_value, CallError& r_call_error) const;
		void call_deferredp(const Variant** p_arguments, int p_argcount) const;
		Variant callv(const Array& p_arguments) const;

		template <typename... VarArgs>
		void call_deferred(VarArgs... p_args) const {
			Variant args[sizeof...(p_args) + 1] = { p_args..., 0 }; // +1 makes sure zero sized arrays are also supported.
			const Variant* argptrs[sizeof...(p_args) + 1];
			for (uint32_t i = 0; i < sizeof...(p_args); i++) {
				argptrs[i] = &args[i];
			}
			return call_deferredp(sizeof...(p_args) == 0 ? nullptr : (const Variant**)argptrs, sizeof...(p_args));
		}
	// @todo read
		Error rpcp(int p_id, const Variant** p_arguments, int p_argcount, CallError& r_call_error) const;

		_FORCE_INLINE_ bool is_null() const {
			return method == StringName() && object == 0;
		}
		_FORCE_INLINE_ bool is_custom() const {
			return method == StringName() && custom != nullptr;
		}
		_FORCE_INLINE_ bool is_standard() const {
			return method != StringName();
		}
		bool is_valid() const;

		template <typename... VarArgs>
		Callable bind(VarArgs... p_args);
		Callable bindv(const Array& p_arguments);

		Callable bindp(const Variant** p_arguments, int p_argcount) const;
		Callable unbind(int p_argcount) const;

		Object* get_object() const;
		ObjectID get_object_id() const;
		StringName get_method() const;
		CallableCustom* get_custom() const;
		int get_bound_arguments_count() const;
		void get_bound_arguments_ref(Vector<Variant>& r_arguments, int& r_argcount) const; // Internal engine use, the exposed one is below.
		Array get_bound_arguments() const;

		uint32_t hash() const;

		const Callable* get_base_comparator() const; //used for bind/unbind to do less precise comparisons (ignoring binds) in signal connect/disconnect

		bool operator==(const Callable& p_callable) const;
		bool operator!=(const Callable& p_callable) const;
		bool operator<(const Callable& p_callable) const;

		void operator=(const Callable& p_callable);

		operator String() const;

		Callable(const Object* p_object, const StringName& p_method);
		Callable(ObjectID p_object, const StringName& p_method);
		Callable(CallableCustom* p_custom);
		Callable(const Callable& p_callable);
		Callable() {}
		~Callable();
	};

	class CallableCustom {
		friend class Callable;
		SafeRefCount ref_count;
		bool referenced = false;

	public:
		typedef bool (*CompareEqualFunc)(const CallableCustom* p_a, const CallableCustom* p_b);
		typedef bool (*CompareLessFunc)(const CallableCustom* p_a, const CallableCustom* p_b);

		//for every type that inherits, these must always be the same for this type
		virtual uint32_t hash() const = 0;
		virtual String get_as_text() const = 0;
		virtual CompareEqualFunc get_compare_equal_func() const = 0;
		virtual CompareLessFunc get_compare_less_func() const = 0;
		virtual bool is_valid() const;
		virtual StringName get_method() const;
		virtual ObjectID get_object() const = 0;
		virtual void call(const Variant** p_arguments, int p_argcount, Variant& r_return_value, Callable::CallError& r_call_error) const = 0;
		virtual Error rpc(int p_peer_id, const Variant** p_arguments, int p_argcount, Callable::CallError& r_call_error) const;
		virtual const Callable* get_base_comparator() const;
		virtual int get_bound_arguments_count() const;
		virtual void get_bound_arguments(Vector<Variant>& r_arguments, int& r_argcount) const;

		CallableCustom() {
			ref_count.init();
		}
		virtual ~CallableCustom() {}
	};
}
#endif // !__CALLABLE_H__
