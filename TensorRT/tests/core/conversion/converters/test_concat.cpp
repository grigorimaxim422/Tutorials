#include <string>
#include "core/compiler.h"
#include "gtest/gtest.h"
#include "tests/util/util.h"
#include "torch/csrc/jit/ir/irparser.h"

TEST(Converters, ATenCatPureTensorConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Tensor):
        %2 : Tensor[] = prim::ListConstruct(%0, %1)
        %3 : int = prim::Constant[value=0]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5}, {at::kCUDA});
  auto in2 = at::randint(1, 10, {5}, {at::kCUDA});

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1, in2});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1, in2});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0].reshape_as(jit_results[0])));
}

TEST(Converters, ATenCatFloatIntConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Tensor):
        %2 : Tensor[] = prim::ListConstruct(%0, %1)
        %3 : int = prim::Constant[value=0]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kFloat);
  auto in2 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kInt);

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1, in2});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1, in2});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0]));
}

TEST(Converters, ATenCatIntHalfIntHalfConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Tensor,
            %2 : Tensor,
            %3 : Tensor):
        %2 : Tensor[] = prim::ListConstruct(%0, %1, %2, %3)
        %3 : int = prim::Constant[value=0]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kInt);
  auto in2 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kHalf);
  auto in3 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kInt);
  auto in4 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kHalf);

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1, in2, in3, in4});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto trt_results =
      torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1, in2, in3, in4}, nvinfer1::DataType::kHALF);

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0]));
}

TEST(Converters, ATenCatHalfIntFloatConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Tensor,
            %2 : Tensor):
        %2 : Tensor[] = prim::ListConstruct(%0, %1, %2)
        %3 : int = prim::Constant[value=0]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kInt);
  auto in2 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kHalf);
  auto in3 = at::randint(1, 10, {5}, {at::kCUDA}).to(at::kFloat);

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1, in2, in3});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1, in2, in3});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0]));
}

TEST(Converters, ATenCatDiffTensorConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Float(5)):
        %2 : Tensor[] = prim::ListConstruct(%0, %1)
        %3 : int = prim::Constant[value=0]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5}, {at::kCUDA});
  auto in2 = at::randint(1, 10, {5}, {at::kCUDA});

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {in2});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {in2});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0].reshape_as(jit_results[0])));
}
TEST(Converters, ATenCatPureTensorNegDimConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Tensor):
        %2 : Tensor[] = prim::ListConstruct(%0, %1)
        %3 : int = prim::Constant[value=-1]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5, 5}, {at::kCUDA});
  auto in2 = at::randint(1, 10, {5, 5}, {at::kCUDA});

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1, in2});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1, in2});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0].reshape_as(jit_results[0])));
}

TEST(Converters, ATenCatDiffTensorNegDimConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor,
            %1 : Float(5)):
        %2 : Tensor[] = prim::ListConstruct(%0, %1)
        %3 : int = prim::Constant[value=-1]()
        %4 : Tensor = aten::cat(%2, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {5, 5}, {at::kCUDA});
  auto in2 = at::randint(1, 10, {5, 5}, {at::kCUDA});

  auto params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {in2});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1});

  params = torch_tensorrt::core::ir::get_static_params(g->inputs(), {in2});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0].reshape_as(jit_results[0])));
}
