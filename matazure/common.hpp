﻿#pragma once

#include <matazure/tensor.hpp>

#ifdef MATAZURE_CUDA
#include <matazure/cuda/tensor.hpp>
#define MATAZURE_IS_D_LAMBDA(X) __nv_is_extended_device_lambda_closure_type(X)
#define MATAZURE_IS_HD_LAMBDA(X) __nv_is_extended_host_device_lambda_closure_type(X)
#endif

namespace matazure {

#ifndef MATAZURE_CUDA

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> extent, _Func fun)->lambda_tensor<_Dim, _Func>{
	return lambda_tensor<_Dim, _Func>(extent, fun);
}

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> extent, _Func fun, host_t)->lambda_tensor<_Dim, _Func>{
	return lambda_tensor<_Dim, _Func>(extent, fun);
}

#else

template <int_t _Dim, typename _Func>
inline auto make_host_lambda(pointi<_Dim> extent, _Func fun)->lambda_tensor<_Dim, _Func>{
	return lambda_tensor<_Dim, _Func>(extent, fun);
}

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, enable_if_t<!MATAZURE_IS_D_LAMBDA(_Func) && !MATAZURE_IS_HD_LAMBDA(_Func)>* = nullptr)->decltype(make_host_lambda(ext, fun)){
	return make_host_lambda(ext, fun);
}

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, host_t, enable_if_t<!MATAZURE_IS_D_LAMBDA(_Func) && !MATAZURE_IS_HD_LAMBDA(_Func)>* = nullptr)->decltype(make_host_lambda(ext, fun)) {
	return make_host_lambda(ext, fun);
}

///TODO: not support device struct operator
template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, device_t, enable_if_t<!MATAZURE_IS_D_LAMBDA(_Func) && !MATAZURE_IS_HD_LAMBDA(_Func)>* = nullptr)->decltype(cuda::make_general_lambda(ext, fun)) {
	return cuda::make_general_lambda(ext, fun);
}

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, enable_if_t<MATAZURE_IS_HD_LAMBDA(_Func)>* = nullptr)->decltype(cuda::make_general_lambda(ext, fun)) {
	return cuda::make_general_lambda(ext, fun);
}

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, device_t, enable_if_t<MATAZURE_IS_HD_LAMBDA(_Func)>* = nullptr)->decltype(cuda::make_general_lambda(ext, fun)) {
	return cuda::make_general_lambda(ext, fun);
}

template <int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, host_t, enable_if_t<MATAZURE_IS_HD_LAMBDA(_Func)>* = nullptr)->decltype(make_host_lambda(ext, fun)) {
	return make_host_lambda(ext, fun);
}

template <typename _ValueType, typename _Access, int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, enable_if_t<MATAZURE_IS_D_LAMBDA(_Func)>* = nullptr)->decltype(cuda::make_device_lambda<_ValueType, _Access>(ext, fun)) {
	return cuda::make_device_lambda<_ValueType, _Access>(ext, fun);
}

template <typename _ValueType, typename _Access, int_t _Dim, typename _Func>
inline auto make_lambda(pointi<_Dim> ext, _Func fun, device_t, enable_if_t<MATAZURE_IS_D_LAMBDA(_Func)>* = nullptr)->decltype(cuda::make_device_lambda<_ValueType, _Access>(ext, fun)) {
	return cuda::make_device_lambda<_ValueType, _Access>(ext, fun);
}

#endif

namespace _internal{

template <typename _Tensor, typename _Func>
struct linear_map_op {
private:
	const _Tensor ts_;
	const _Func fun_;

public:
	linear_map_op(_Tensor ts, _Func fun) : ts_(ts), fun_(fun) {}

	MATAZURE_GENERAL auto operator()(int_t i) const->decltype(this->fun_(this->ts_[i])){
		return fun_(ts_[i]);
	}
};

template <typename _Tensor, typename _Func>
struct array_map_op {
private:
	const _Tensor ts_;
	const _Func fun_;

public:
	array_map_op(_Tensor ts, _Func fun) : ts_(ts), fun_(fun) {}

	MATAZURE_GENERAL auto operator()(pointi<_Tensor::dim> idx) const->decltype(this->fun_(this->ts_(idx))) {
		return fun_(ts_(idx));
	}
};

template <typename _Tensor, typename _Func>
struct device_linear_map_op {
private:
	const _Tensor ts_;
	const _Func fun_;

public:
	device_linear_map_op(_Tensor ts, _Func fun) : ts_(ts), fun_(fun) {}

	MATAZURE_DEVICE auto operator()(int_t i) const->decltype(this->fun_(this->ts_[i])) {
		return fun_(ts_[i]);
	}
};

template <typename _Tensor, typename _Func>
struct device_array_map_op {
private:
	const _Tensor ts_;
	const _Func fun_;

public:
	device_array_map_op(_Tensor ts, _Func fun) : ts_(ts), fun_(fun) {}

	MATAZURE_DEVICE auto operator()(pointi<_Tensor::dim> idx) const->decltype(this->fun_(this->ts_(idx))) {
		return fun_(ts_(idx));
	}
};

template <typename _OutValueType>
struct cast_op {
	template <typename _InValueType>
	MATAZURE_GENERAL auto operator()(_InValueType v) const->decltype(static_cast<_OutValueType>(v)) {
		return static_cast<_OutValueType>(v);
	}
};

template <typename _Tensor>
struct section_op {
private:
	_Tensor ts_;
	typename _Tensor::index_type origin_;

public:
	section_op(_Tensor ts, typename _Tensor::index_type origin):
		ts_(ts), origin_(origin)
	{}

	MATAZURE_GENERAL typename _Tensor::value_type operator()(pointi<_Tensor::dim> idx) const {
		return ts_(idx + origin_);
	}
};

template <typename _Tensor, typename _StrideType, typename _PhaseType>
struct stride_op {
private:
	_Tensor ts_;
	_StrideType stride_;
	_PhaseType phase_;

public:
	stride_op(_Tensor ts, _StrideType stride, _PhaseType phase):
		ts_(ts), stride_(stride), phase_(phase)
	{}

	MATAZURE_GENERAL typename _Tensor::value_type operator()(pointi<_Tensor::dim> idx) const {
		return ts_(idx * stride_ + phase_);
	}
};

template <typename _Tensor0, typename _Tensor1>
struct zip_op{
private:
	_Tensor0 ts0_;
	_Tensor1 ts1_;

public:
	zip_op(_Tensor0 ts0, _Tensor1 ts1): ts0_(ts0), ts1_(ts1){}

	MATAZURE_GENERAL auto operator()(int_t i) const->decltype(tie(ts0_[0], ts1_[0])){
		return tie(ts0_[i], ts1_[i]);
	}
};

//template <typename _Tensor, int_t _DimI>
//struct splice_op {
//	splice_op(_Tensor ts, int_t splic_i):
//		ts_(ts), splice_i_(splic_i)
//	{}
//
//	MATAZURE_GENERAL auto operator()(pointi<_Tensor::dim-1> idx) const {
//		return ts_();
//	}
//
//private:
//	_Tensor ts_;
//	int_t splic_i_;
//};

}

#ifndef MATAZURE_CUDA

template <typename _Tensor, typename _Func>
inline auto map(_Tensor ts, _Func fun, enable_if_t<is_same<linear_access_t, typename _Tensor::access_type>::value>* = 0)->decltype(make_lambda(ts.extent(), _internal::linear_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{}))
{
	return make_lambda(ts.extent(), _internal::linear_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{});
}

template <typename _Tensor, typename _Func>
inline auto map(_Tensor ts, _Func fun, enable_if_t<is_same<array_access_t, typename _Tensor::access_type>::value>* = 0)->decltype(make_lambda(ts.extent(), _internal::array_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{}))
{
	return make_lambda(ts.extent(), _internal::array_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{});
}

#else

template <typename _Tensor, typename _Func>
inline auto map(_Tensor ts, _Func fun, enable_if_t<is_same<linear_access_t, typename _Tensor::access_type>::value>* = 0, enable_if_t<!MATAZURE_IS_D_LAMBDA(_Func)>* = nullptr)->decltype(make_lambda(ts.extent(), _internal::linear_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{}))
{
	return make_lambda(ts.extent(), _internal::linear_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{});
}

template <typename _Tensor, typename _Func>
inline auto map(_Tensor ts, _Func fun, enable_if_t<is_same<array_access_t, typename _Tensor::access_type>::value>* = 0, enable_if_t<!MATAZURE_IS_D_LAMBDA(_Func)>* = nullptr)->decltype(make_lambda(ts.extent(), _internal::array_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{}))
{
	return make_lambda(ts.extent(), _internal::array_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{});
}

template <typename _Tensor, typename _Func>
inline auto map(_Tensor ts, _Func fun, enable_if_t<is_same<linear_access_t, typename _Tensor::access_type>::value>* = 0, enable_if_t<MATAZURE_IS_D_LAMBDA(_Func)>* = nullptr)->decltype(make_lambda(ts.extent(), _internal::device_linear_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{}))
{
	return make_lambda(ts.extent(), _internal::device_linear_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{});
}

template <typename _Tensor, typename _Func>
inline auto map(_Tensor ts, _Func fun, enable_if_t<is_same<array_access_t, typename _Tensor::access_type>::value>* = 0, enable_if_t<MATAZURE_IS_D_LAMBDA(_Func)>* = nullptr)->decltype(make_lambda(ts.extent(), _internal::device_array_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{}))
{
	return make_lambda(ts.extent(), _internal::device_array_map_op<_Tensor, _Func>(ts, fun), typename _Tensor::memory_type{});
}

#endif

template <typename _ValueType, typename _Tensor>
inline auto tensor_cast(_Tensor tensor)->decltype(map(tensor, _internal::cast_op<_ValueType>())) {
	return map(tensor, _internal::cast_op<_ValueType>());
}

template <typename _Tensor>
inline auto section(_Tensor ts, typename _Tensor::index_type origin, typename _Tensor::extent_type ext)->decltype(make_lambda(ext, _internal::section_op<_Tensor>(ts, origin), typename _Tensor::memory_type{})) {
	return make_lambda(ext, _internal::section_op<_Tensor>(ts, origin), typename _Tensor::memory_type{});
}

template <typename _Tensor, typename _StrideType, typename _PhaseType>
inline auto stride(_Tensor ts, _StrideType stride, _PhaseType phase)->decltype(make_lambda(ts.extent() / stride, _internal::stride_op<_Tensor, _StrideType, _PhaseType>(ts, stride, phase), typename _Tensor::memory_type{})) {
	return make_lambda(ts.extent() / stride, _internal::stride_op<_Tensor, _StrideType, _PhaseType>(ts, stride, phase), typename _Tensor::memory_type{});
}

///实现稍微有点难度
//template <typename _Tensor, int_t _DimIdx>
//inline auto splice(_Tensor ts, int_t i) {
//	return make_lambda()
//}

template <typename _Tensor0, typename _Tensor1>
inline auto zip(_Tensor0 ts0, _Tensor1 ts1)->decltype(make_lambda(ts0.extent(), _internal::zip_op<_Tensor0, _Tensor1>(ts0, ts1))) {
	return make_lambda(ts0.extent(), _internal::zip_op<_Tensor0, _Tensor1>(ts0, ts1));
}

// template <typename ..._Tensors>
// inline auto zip(_Tensors... tensors){
// 	auto tuple_tensors = make_tuple(tensors);
// 	auto ext = get<0>(tuple_tensors).extent();
// 	return make_lambda(ext, [=](int_t i){
// 		return tie(tensors...[i]);
// 	});
// }

#define __MATAZURE_LINEAR_ACCESS_TENSOR_BINARY_OPERATOR(name, op) \
template <typename _T1, typename _T2> \
struct name { \
private: \
	_T1 x1_; \
	_T2 x2_; \
\
public:\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_T1, _T2); \
	MATAZURE_STATIC_ASSERT_VALUE_TYPE_MATCHED(_T1, _T2); \
\
	MATAZURE_GENERAL name(_T1 x1, _T2 x2) : x1_(x1), x2_(x2) {} \
\
	MATAZURE_GENERAL auto operator ()(int_t i) const->decltype(this->x1_[i] op this->x2_[i]){ \
		return x1_[i] op x2_[i]; \
	} \
};

#define __MATAZURE_ARRAY_ACCESS_TENSOR_BINARY_OPERATOR(name, op) \
template <typename _T1, typename _T2> \
struct name { \
private: \
	_T1 x1_; \
	_T2 x2_; \
\
public:\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_T1, _T2); \
	MATAZURE_STATIC_ASSERT_VALUE_TYPE_MATCHED(_T1, _T2); \
	MATAZURE_GENERAL name(_T1 x1, _T2 x2) : x1_(x1), x2_(x2) {} \
\
	MATAZURE_GENERAL auto operator ()(const typename _T1::index_type &idx) const->decltype(this->x1_(idx) op this->x2_(idx)){ \
		return x1_(idx) op x2_(idx); \
	} \
};

#define __MATAZURE_LINEAR_ACCESS_TENSOR_WITH_VALUE_BINARY_OPERATOR(name, op) \
template <typename _T> \
struct name { \
private: \
	typedef typename _T::value_type value_type; \
\
	_T x_; \
	value_type v_; \
\
public:\
	name(_T x, value_type v) : x_(x), v_(v) {} \
\
MATAZURE_GENERAL auto operator()(int_t i) const->decltype(this->x_[i] op this->v_){ \
		return x_[i] op v_; \
	} \
\
};

#define __MATAZURE_ARRAY_ACCESS_TENSOR_WITH_VALUE_BINARY_OPERATOR(name, op) \
template <typename _T> \
struct name { \
private: \
	typedef typename _T::value_type value_type; \
\
	_T x_; \
	value_type v_; \
\
public:\
	name(_T x, value_type v) : x_(x), v_(v) {} \
\
	MATAZURE_GENERAL auto operator ()(const typename _T::index_type &idx) const->decltype(this->x_(idx) op this->v_){ \
		return x_(idx) op v_; \
	} \
};

#define __MATAZURE_VALUE_WITH_LINEAR_ACCESS_TENSOR_BINARY_OPERATOR(name, op) \
template <typename _T> \
struct name { \
private: \
	typedef typename _T::value_type value_type; \
\
	value_type v_; \
	_T x_; \
\
public: \
	name(value_type v, _T x) : v_(v), x_(x) {} \
\
	MATAZURE_GENERAL auto operator ()(const int_t &i) const->decltype((this->_v) op (this->x_[i])){ \
		return  v_ op x_[i]; \
	} \
};

#define __MATAZURE_VALUE_WITH_ARRAY_ACCESS_TENSOR_BINARY_OPERATOR(name, op) \
template <typename _T> \
struct name { \
private: \
	typedef typename _T::value_type value_type; \
\
	value_type v_; \
	_T x_; \
\
public:\
	name(_T x, value_type v) : v_(v), x_(x) {} \
\
	MATAZURE_GENERAL auto operator ()(const typename _T::index_type &idx) const->decltype(this->v_ op this->x_(idx)){ \
		return v_ op x_(idx); \
	} \
\
};

//host tensor operations
#define TENSOR_BINARY_OPERATOR(name, op) \
__MATAZURE_LINEAR_ACCESS_TENSOR_BINARY_OPERATOR(__##name##_linear_access_tensor__, op) \
template <typename _TS1, typename _TS2> \
inline enable_if_t< none_device_memory<_TS1, _TS2>::value && are_linear_access<_TS1, _TS2>::value, lambda_tensor<_TS1::dim, __##name##_linear_access_tensor__<_TS1, _TS2>>> operator op(const tensor_expression<_TS1> &e_lhs, const tensor_expression<_TS2> &e_rhs) { \
	return make_lambda(e_lhs().extent(), __##name##_linear_access_tensor__<_TS1, _TS2>(e_lhs(), e_rhs()), host_t{}); \
} \
__MATAZURE_ARRAY_ACCESS_TENSOR_BINARY_OPERATOR(__##name##_array_access_tensor__, op) \
template <typename _TS1, typename _TS2> \
inline enable_if_t< none_device_memory<_TS1, _TS2>::value && !are_linear_access<_TS1, _TS2>::value, lambda_tensor<_TS1::dim, __##name##_array_access_tensor__<_TS1, _TS2>>> operator op(const tensor_expression<_TS1> &e_lhs, const tensor_expression<_TS2> &e_rhs) { \
	return make_lambda(e_lhs().extent(), __##name##_array_access_tensor__<_TS1, _TS2>(e_lhs(), e_rhs())); \
}

#define TENSOR_WITH_VALUE_BINARY_OPERATOR(name, op) \
__MATAZURE_LINEAR_ACCESS_TENSOR_WITH_VALUE_BINARY_OPERATOR(__##name##_linear_access_tensor_with_value__, op) \
\
template <typename _TS> \
inline enable_if_t< none_device_memory<_TS>::value && are_linear_access<_TS>::value, lambda_tensor<_TS::dim, __##name##_linear_access_tensor_with_value__<_TS>>> operator op(const tensor_expression<_TS> &e_ts, typename _TS::value_type v) { \
	return make_lambda(e_ts().extent(), __##name##_linear_access_tensor_with_value__<_TS>(e_ts(), v)); \
} \
\
__MATAZURE_VALUE_WITH_LINEAR_ACCESS_TENSOR_BINARY_OPERATOR(__##name##_value_with_linear_access_tensor__, op) \
template <typename _TS> \
inline enable_if_t< none_device_memory<_TS>::value && are_linear_access<_TS>::value, lambda_tensor<_TS::dim, __##name##_value_with_linear_access_tensor__<_TS>>> operator op(typename _TS::value_type v, const tensor_expression<_TS> &e_ts) { \
	return make_lambda(e_ts().extent(), __##name##_value_with_linear_access_tensor__<_TS>(v, e_ts())); \
} \
\
__MATAZURE_ARRAY_ACCESS_TENSOR_WITH_VALUE_BINARY_OPERATOR(__##name##_array_access_tensor_with_value__, op) \
template <typename _TS> \
inline enable_if_t< none_device_memory<_TS>::value && !are_linear_access<_TS>::value, lambda_tensor<_TS::dim, __##name##_array_access_tensor_with_value__<_TS>>> operator op(const tensor_expression<_TS> &e_ts, typename _TS::value_type v) { \
	return make_lambda(e_ts().extent(), __##name##_array_access_tensor_with_value__<_TS>(e_ts(), v)); \
}\
\
__MATAZURE_VALUE_WITH_ARRAY_ACCESS_TENSOR_BINARY_OPERATOR(__##name##_value_with_array_access_tensor__, op) \
template <typename _TS> \
inline enable_if_t< none_device_memory<_TS>::value && !are_linear_access<_TS>::value, lambda_tensor<_TS::dim, __##name##_value_with_array_access_tensor__<_TS>>> operator op(typename _TS::value_type v, const tensor_expression<_TS> &e_ts) { \
	return make_lambda(e_ts().extent(), __##name##_value_with_array_access_tensor__<_TS>(v, e_ts())); \
}

//device tensor operations
#define CU_TENSOR_BINARY_OPERATOR(name, op) \
template <typename _TS1, typename _TS2> \
inline enable_if_t<are_device_memory<_TS1, _TS2>::value && are_linear_access<_TS1, _TS2>::value, cuda::general_lambda_tensor<_TS1::dim, __##name##_linear_access_tensor__<_TS1, _TS2>>> operator op(const tensor_expression<_TS1> &e_lhs, const tensor_expression<_TS2> &e_rhs) { \
	return make_lambda(e_lhs().extent(), __##name##_linear_access_tensor__<_TS1, _TS2>(e_lhs(), e_rhs()), device_t{}); \
} \
template <typename _TS1, typename _TS2> \
inline enable_if_t< are_device_memory<_TS1, _TS2>::value && !are_linear_access<_TS1, _TS2>::value, cuda::general_lambda_tensor<_TS1::dim, __##name##_array_access_tensor__<_TS1, _TS2>>> operator op(const tensor_expression<_TS1> &e_lhs, const tensor_expression<_TS2> &e_rhs) { \
	return make_lambda(e_lhs().extent(), __##name##_array_access_tensor__<_TS1, _TS2>(e_lhs(), e_rhs()), device_t{}); \
}

#define CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(name, op) \
\
template <typename _TS> \
inline enable_if_t< are_device_memory<_TS>::value && are_linear_access<_TS>::value, cuda::general_lambda_tensor<_TS::dim, __##name##_linear_access_tensor_with_value__<_TS>>> operator op(const tensor_expression<_TS> &e_ts, typename _TS::value_type v) { \
	return make_lambda(e_ts().extent(), __##name##_linear_access_tensor_with_value__<_TS>(e_ts(), v), device_t{}); \
} \
\
template <typename _TS> \
inline enable_if_t< are_device_memory<_TS>::value && are_linear_access<_TS>::value, cuda::general_lambda_tensor<_TS::dim, __##name##_value_with_linear_access_tensor__<_TS>>> operator op(typename _TS::value_type v, const tensor_expression<_TS> &e_ts) { \
	return make_lambda(e_ts().extent(), __##name##_value_with_linear_access_tensor__<_TS>(v, e_ts()), device_t{}); \
} \
\
template <typename _TS> \
inline enable_if_t< are_device_memory<_TS>::value && !are_linear_access<_TS>::value, cuda::general_lambda_tensor<_TS::dim, __##name##_array_access_tensor_with_value__<_TS>>> operator op(const tensor_expression<_TS> &e_ts, typename _TS::value_type v) { \
	return make_lambda(e_ts().extent(), __##name##_array_access_tensor_with_value__<_TS>(e_ts(), v), device_t{}); \
}\
\
template <typename _TS> \
inline enable_if_t< are_device_memory<_TS>::value && !are_linear_access<_TS>::value, cuda::general_lambda_tensor<_TS::dim, __##name##_value_with_array_access_tensor__<_TS>>> operator op(typename _TS::value_type v, const tensor_expression<_TS> &e_ts) { \
	return make_lambda(e_ts().extent(), __##name##_value_with_array_access_tensor__<_TS>(v, e_ts()), device_t{}); \
}

//Arithmetic
TENSOR_BINARY_OPERATOR(add, +)
TENSOR_BINARY_OPERATOR(sub, -)
TENSOR_BINARY_OPERATOR(mul, *)
TENSOR_BINARY_OPERATOR(div, / )
TENSOR_BINARY_OPERATOR(mod, %)
TENSOR_WITH_VALUE_BINARY_OPERATOR(add, +)
TENSOR_WITH_VALUE_BINARY_OPERATOR(sub, -)
TENSOR_WITH_VALUE_BINARY_OPERATOR(mul, *)
TENSOR_WITH_VALUE_BINARY_OPERATOR(div, /)
TENSOR_WITH_VALUE_BINARY_OPERATOR(mod, %)
//Bit
TENSOR_BINARY_OPERATOR(left_shift, <<)
TENSOR_BINARY_OPERATOR(right_shift, >>)
TENSOR_BINARY_OPERATOR(bit_or, |)
TENSOR_BINARY_OPERATOR(bit_and, &)
TENSOR_BINARY_OPERATOR(bit_xor, ^)
TENSOR_WITH_VALUE_BINARY_OPERATOR(left_shift, <<)
TENSOR_WITH_VALUE_BINARY_OPERATOR(right_shift, >>)
TENSOR_WITH_VALUE_BINARY_OPERATOR(bit_or, |)
TENSOR_WITH_VALUE_BINARY_OPERATOR(bit_and, &)
TENSOR_WITH_VALUE_BINARY_OPERATOR(bit_xor, ^)
//Logic
TENSOR_BINARY_OPERATOR(or , ||)
TENSOR_BINARY_OPERATOR(and, &&)
TENSOR_WITH_VALUE_BINARY_OPERATOR(or , ||)
TENSOR_WITH_VALUE_BINARY_OPERATOR(and, &&)
//Compapre
TENSOR_BINARY_OPERATOR(gt, >)
TENSOR_BINARY_OPERATOR(lt, <)
TENSOR_BINARY_OPERATOR(ge, >=)
TENSOR_BINARY_OPERATOR(le, <=)
TENSOR_BINARY_OPERATOR(equal, ==)
TENSOR_BINARY_OPERATOR(not_equal, !=)
TENSOR_WITH_VALUE_BINARY_OPERATOR(gt, >)
TENSOR_WITH_VALUE_BINARY_OPERATOR(lt, <)
TENSOR_WITH_VALUE_BINARY_OPERATOR(ge, >=)
TENSOR_WITH_VALUE_BINARY_OPERATOR(le, <=)
TENSOR_WITH_VALUE_BINARY_OPERATOR(equal, ==)
TENSOR_WITH_VALUE_BINARY_OPERATOR(not_equal, !=)

#ifdef MATAZURE_CUDA
//Arithmetic
CU_TENSOR_BINARY_OPERATOR(add, +)
CU_TENSOR_BINARY_OPERATOR(sub, -)
CU_TENSOR_BINARY_OPERATOR(mul, *)
CU_TENSOR_BINARY_OPERATOR(div, /)
CU_TENSOR_BINARY_OPERATOR(mod, %)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(add, +)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(sub, -)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(mul, *)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(div, /)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(mod, %)
//Bit
CU_TENSOR_BINARY_OPERATOR(left_shift, <<)
CU_TENSOR_BINARY_OPERATOR(right_shift, >>)
CU_TENSOR_BINARY_OPERATOR(bit_or, |)
CU_TENSOR_BINARY_OPERATOR(bit_and, &)
CU_TENSOR_BINARY_OPERATOR(bit_xor, ^)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(left_shift, <<)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(right_shift, >>)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(bit_or, |)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(bit_and, &)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(bit_xor, ^)
//Logic
CU_TENSOR_BINARY_OPERATOR(or , ||)
CU_TENSOR_BINARY_OPERATOR(and, &&)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(or , ||)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(and, &&)
//Compapre
CU_TENSOR_BINARY_OPERATOR(gt, >)
CU_TENSOR_BINARY_OPERATOR(lt, <)
CU_TENSOR_BINARY_OPERATOR(ge, >=)
CU_TENSOR_BINARY_OPERATOR(le, <=)
CU_TENSOR_BINARY_OPERATOR(equal, ==)
CU_TENSOR_BINARY_OPERATOR(not_equal, !=)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(gt, >)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(lt, <)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(ge, >=)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(le, <=)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(equal, ==)
CU_TENSOR_WITH_VALUE_BINARY_OPERATOR(not_equal, !=)

#endif

}
