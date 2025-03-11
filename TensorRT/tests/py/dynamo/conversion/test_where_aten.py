import torch
import torch.nn as nn
from parameterized import parameterized
from torch.testing._internal.common_utils import run_tests
from torch_tensorrt import Input

from .harness import DispatchTestCase


class TestWhereConverter(DispatchTestCase):
    @parameterized.expand(
        [
            ("2d_condition_xshape_yshape", (2, 2), (2, 2)),
            ("2d_broadcast_condition_xshape_yshape", (2, 2), (2, 1)),
            ("3d_condition_xshape_yshape", (2, 2, 1), (2, 2, 1)),
            ("2d_3d_condition_xshape_yshape", (2, 2), (1, 2, 2)),
            ("3d_2d_condition_xshape_yshape", (1, 2, 2), (2, 2)),
        ]
    )
    def test_(self, _, x_size, y_size):
        class Where(nn.Module):
            def forward(self, condition, x, y):
                return torch.ops.aten.where.self(condition, x, y)

        inputX = torch.randn(*x_size)
        inputOther = torch.randn(*y_size)
        condition = inputX < 0
        self.run_test(
            Where(),
            (condition, inputX, inputOther),
        )

    def test_0D_input(self):
        class Where(nn.Module):
            def forward(self, condition, x, y):
                return torch.ops.aten.where.self(condition, x, y)

        inputX = torch.randn((5, 6, 7, 1, 3))
        inputOther = torch.tensor(8.0, dtype=torch.float)
        condition = inputX < 0
        self.run_test(
            Where(),
            (condition, inputX, inputOther),
        )

    def test_const_input(self):
        class Where(nn.Module):
            def __init__(self, *args, **kwargs) -> None:
                super().__init__(*args, **kwargs)
                self.inputY = torch.randn((5, 6, 7))
                self.inputX = torch.randn((5, 6, 7))

            def forward(self, condition):
                return torch.ops.aten.where.self(condition, self.inputX, self.inputY)

        input1 = torch.randn((5, 6, 7))
        condition = input1 < 0
        self.run_test(
            Where(),
            (condition,),
        )

    def test_const_input_with_broadcast(self):
        class Where(nn.Module):
            def __init__(self, *args, **kwargs) -> None:
                super().__init__(*args, **kwargs)
                self.inputY = torch.randn((1,))
                self.inputX = torch.randn((1,))

            def forward(self, condition):
                return torch.ops.aten.where.self(condition, self.inputX, self.inputY)

        input1 = torch.randn((5, 6, 7))
        condition = input1 < 0
        self.run_test(
            Where(),
            (condition,),
        )

    # min/opt/max shape for condition/x/y input
    @parameterized.expand(
        [
            (
                "3d_condition_3d_xshape_3d_yshape",
                (1, 1, 1),
                (1, 2, 3),
                (3, 3, 3),
                (1, 1, 1),
                (1, 2, 3),
                (3, 3, 3),
                (1, 1, 1),
                (1, 2, 3),
                (3, 3, 3),
            ),
            (
                "1d_condition_3d_xshape_2d_yshape",
                (1,),
                (2,),
                (4,),
                (1, 1, 1),
                (3, 2, 2),
                (3, 2, 4),
                (1, 1),
                (2, 2),
                (2, 4),
            ),
            (
                "2d_condition_3d_xshape_2d_yshape",
                (4, 1),
                (4, 2),
                (5, 4),
                (1, 1, 1),
                (3, 1, 2),
                (3, 1, 4),
                (1, 1),
                (1, 2),
                (1, 4),
            ),
        ]
    )
    def test_with_dynamic_shape(self, *args):
        class Where(nn.Module):
            def forward(self, condition, x, y):
                return torch.ops.aten.where.self(condition, x, y)

        input_specs = [
            Input(
                min_shape=args[1],
                opt_shape=args[2],
                max_shape=args[3],
                dtype=torch.bool,
            ),
            Input(
                min_shape=args[4],
                opt_shape=args[5],
                max_shape=args[6],
                dtype=torch.float32,
            ),
            Input(
                min_shape=args[7],
                opt_shape=args[8],
                max_shape=args[9],
                dtype=torch.float32,
            ),
        ]
        self.run_test_with_dynamic_shape(Where(), input_specs)


if __name__ == "__main__":
    run_tests()
