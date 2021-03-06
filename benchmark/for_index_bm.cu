#include <benchmark/benchmark.h>
#include <matazure/tensor>
#include <cmath>

using namespace matazure;

template <typename _ExecutionPolicy>
void BM_for_index(benchmark::State& state) {
	tensor<float, 1> ts1(state.range(0));
	tensor<float, 1> ts2(ts1.shape());
	while (state.KeepRunning()) {
		for_index(_ExecutionPolicy{}, 0, ts1.size(), [=](int_t i) {
			ts2[i] = std::sin(ts1[i]) / ts2[i] + std::sqrt(ts2[i]) * ts1[i];
		});
	}

	state.SetItemsProcessed(ts1.size());
}

BENCHMARK_TEMPLATE1(BM_for_index, sequence_policy)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE1(BM_for_index, sequence_vectorized_policy)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE1(BM_for_index, omp_policy)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE1(BM_for_index, omp_vectorized_policy)->Range(1<<10, 1 << 28)->UseRealTime();

template <typename _ExecutionPolicy, int_t _Rank>
void BM_for_index_array(benchmark::State& state) {
	pointi<_Rank> shape{};
	fill(shape, state.range(0));
	tensor<float, _Rank> ts1(shape);
	tensor<float, _Rank> ts2(ts1.shape());
	while (state.KeepRunning()) {
		for_index(_ExecutionPolicy{}, pointi<_Rank>{0}, ts1.shape(), [=](pointi<_Rank> idx) {
			ts2[idx] = std::sin(ts1[idx]) / ts2[idx] + std::sqrt(ts2[idx]) * ts1[idx];
		});
	}

	state.SetItemsProcessed(ts1.size());
}

BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_policy, 1)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_policy, 2)->Range(1<<5, 1 << 14)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_policy, 3)->Range(1<<3, 1 << 8)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_policy, 4)->Range(1<<2, 1 << 7)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_vectorized_policy, 1)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_vectorized_policy, 2)->Range(1<<5, 1 << 14)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_vectorized_policy, 3)->Range(1<<3, 1 << 8)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, sequence_vectorized_policy, 4)->Range(1<<2, 1 << 7)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_policy, 1)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_policy, 2)->Range(1<<5, 1 << 14)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_policy, 3)->Range(1<<3, 1 << 8)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_policy, 4)->Range(1<<2, 1 << 7)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_vectorized_policy, 1)->Range(1<<10, 1 << 28)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_vectorized_policy, 2)->Range(1<<5, 1 << 14)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_vectorized_policy, 3)->Range(1<<3, 1 << 8)->UseRealTime();
BENCHMARK_TEMPLATE2(BM_for_index_array, omp_vectorized_policy, 4)->Range(1<<2, 1 << 7)->UseRealTime();


