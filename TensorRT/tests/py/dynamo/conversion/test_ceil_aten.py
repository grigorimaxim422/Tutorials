import torch
import torch.nn as nn
from parameterized import parameterized
from torch.testing._internal.common_utils import run_tests
from torch_tensorrt import Input

from .harness import DispatchTestCase


class TestCeilConverter(DispatchTestCase):
    @parameterized.expand(
        [
            ((10,), torch.float),
            ((1, 20), torch.float),
            ((2, 3, 4), torch.float),
            ((2, 3, 4, 5), torch.float),
        ]
    )
    def test_ceil_float(self, input_shape, dtype):
        class ceil(nn.Module):
            def forward(self, input):
                return torch.ops.aten.ceil.default(input)

        inputs = [torch.randn(input_shape, dtype=dtype)]
        self.run_test(
            ceil(),
            inputs,
        )

    @parameterized.expand(
        [
            ((10,), torch.int, 0, 5),
            ((1, 20), torch.int32, -10, 10),
            ((2, 3, 4), torch.int, -5, 5),
        ]
    )
    def test_ceil_int(self, input_shape, dtype, low, high):
        class ceil(nn.Module):
            def forward(self, input):
                return torch.ops.aten.ceil.default(input)

        inputs = [torch.randint(low, high, input_shape, dtype=dtype)]
        self.run_test(
            ceil(),
            inputs,
            check_dtype=False,
        )

    @parameterized.expand(
        [
            (
                "2d_dim_dtype_half",
                (1, 1),
                (2, 2),
                (4, 4),
                torch.half,
                torch.half,
            ),
            (
                "3d_dim_dtype_float",
                (1, 1, 1),
                (1, 2, 3),
                (3, 3, 3),
                torch.float,
                torch.float,
            ),
        ]
    )
    def test_dynamic_shape_ceil(
        self, _, min_shape, opt_shape, max_shape, type, output_type
    ):
        class ceil(nn.Module):
            def forward(self, input):
                return torch.ops.aten.ceil.default(input)

        input_specs = [
            Input(
                min_shape=min_shape,
                opt_shape=opt_shape,
                max_shape=max_shape,
                dtype=type,
            ),
        ]
        self.run_test_with_dynamic_shape(
            ceil(), input_specs, output_dtypes=[output_type]
        )


if __name__ == "__main__":
    run_tests()
