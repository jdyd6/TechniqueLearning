"""
LCD 紫外光场均匀性标定主流程
整合 Doc/通用版.docx 标定算法与 test2.py 测试图生成
"""
import os
import argparse

import numpy as np
from PIL import Image
import matplotlib

# 无显示器环境下也能保存图片
if os.environ.get("MPLBACKEND") is None:
    matplotlib.use("Agg")
import matplotlib.pyplot as plt

from test2 import generate_dark_12bit, generate_light_12bit

# 解决中文显示问题
plt.rcParams["font.sans-serif"] = ["SimHei", "Microsoft YaHei", "DejaVu Sans"]
plt.rcParams["axes.unicode_minus"] = False


def read_12bit_tiff(file_path: str) -> np.ndarray:
    """读取 12bit 无压缩 TIFF，返回 float32 数组（值域 0~4095）"""
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"文件不存在：{file_path}")
    img = Image.open(file_path)
    return np.array(img, dtype=np.float32)


def dark_correct(light_img: np.ndarray, dark_img: np.ndarray) -> np.ndarray:
    """暗场校正：亮场减暗场，剔除负值"""
    corrected = light_img - dark_img
    corrected[corrected < 0] = 0
    return corrected


def auto_select_roi(raw_image: np.ndarray, margin_ratio: float = 0.1) -> np.ndarray:
    """
    自动裁切中心有效区域（用于测试图/无人值守运行）
    margin_ratio: 四边各裁掉的比例
    """
    h, w = raw_image.shape
    mx = int(w * margin_ratio)
    my = int(h * margin_ratio)
    if mx * 2 >= w or my * 2 >= h:
        raise ValueError("margin_ratio 过大，ROI 无效")
    return raw_image[my : h - my, mx : w - mx].copy()


def select_roi(raw_image: np.ndarray) -> np.ndarray:
    """手动框选有效发光区域（回车确认，ESC 取消）"""
    try:
        import cv2
    except ImportError as exc:
        raise ImportError(
            "手动 ROI 需要 opencv-python，请执行：pip install opencv-python"
        ) from exc

    display_img = cv2.normalize(
        raw_image, None, 0, 65535, cv2.NORM_MINMAX, dtype=cv2.CV_16U
    )
    roi = cv2.selectROI(
        "框选有效发光区域（回车确认，ESC取消）", display_img, showCrosshair=True
    )
    cv2.destroyAllWindows()
    x, y, w, h = roi
    if w == 0 or h == 0:
        raise ValueError("未框选有效区域")
    return raw_image[y : y + h, x : x + w]


def calc_block_ki(
    roi_img: np.ndarray,
    row_num: int = 5,
    col_num: int = 12,
    ki_min: float = 0.6,
    ki_max: float = 1.5,
) -> tuple[list[float], list[float]]:
    """划分网格，计算每区平均灰度与补偿系数 Ki（行优先）"""
    h, w = roi_img.shape
    block_w = w // col_num
    block_h = h // row_num
    if block_w == 0 or block_h == 0:
        raise ValueError("ROI 尺寸过小，无法按当前行列数分区")

    block_avg_list: list[float] = []
    for r in range(row_num):
        for c in range(col_num):
            x1 = c * block_w
            y1 = r * block_h
            x2 = x1 + block_w
            y2 = y1 + block_h
            block = roi_img[y1:y2, x1:x2]
            block_avg_list.append(float(np.mean(block)))

    global_avg = float(np.mean(block_avg_list))
    ki_list: list[float] = []
    for avg_val in block_avg_list:
        if avg_val < 1e-6:
            ki = 1.0
        else:
            ki = global_avg / avg_val
        ki_list.append(round(float(np.clip(ki, ki_min, ki_max)), 4))

    return ki_list, block_avg_list


def plot_roi_3d_surface(
    roi_img: np.ndarray, downsample: int = 8, save_path: str | None = None
) -> None:
    """绘制 ROI 灰度 3D 曲面图"""
    img_ds = roi_img[::downsample, ::downsample]
    h, w = img_ds.shape
    x = np.arange(w)
    y = np.arange(h)
    X, Y = np.meshgrid(x, y)
    Z = img_ds

    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot(111, projection="3d")
    surf = ax.plot_surface(X, Y, Z, cmap="jet", edgecolor="none", alpha=0.85)
    ax.set_xlabel("横向像素")
    ax.set_ylabel("纵向像素")
    ax.set_zlabel("灰度值（12bit）")
    ax.set_title("屏幕光场灰度3D分布")
    fig.colorbar(surf, shrink=0.5, aspect=10, label="灰度值")
    ax.view_init(elev=30, azim=-45)
    plt.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=120, bbox_inches="tight")
        print(f"已保存：{save_path}")
    else:
        plt.show()
    plt.close(fig)


def plot_block_3d(
    value_list: list[float],
    row_num: int = 5,
    col_num: int = 12,
    title: str = "分区数值分布",
    zlabel: str = "数值",
    save_path: str | None = None,
) -> None:
    """绘制分区 3D 柱状图"""
    value_matrix = np.array(value_list).reshape(row_num, col_num)
    x = np.arange(col_num)
    y = np.arange(row_num)
    X, Y = np.meshgrid(x, y)
    Z_base = np.zeros_like(value_matrix)
    dx, dy = 0.8, 0.8

    fig = plt.figure(figsize=(12, 7))
    ax = fig.add_subplot(111, projection="3d")
    ax.bar3d(
        X.ravel(),
        Y.ravel(),
        Z_base.ravel(),
        dx,
        dy,
        value_matrix.ravel(),
        cmap="jet",
        alpha=0.8,
    )
    ax.set_xlabel("列号")
    ax.set_ylabel("行号")
    ax.set_zlabel(zlabel)
    ax.set_title(title)
    ax.set_xticks(np.arange(col_num) + dx / 2)
    ax.set_xticklabels([f"{i + 1}" for i in range(col_num)])
    ax.set_yticks(np.arange(row_num) + dy / 2)
    ax.set_yticklabels([f"{i + 1}" for i in range(row_num)])
    ax.view_init(elev=30, azim=-50)
    plt.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=120, bbox_inches="tight")
        print(f"已保存：{save_path}")
    else:
        plt.show()
    plt.close(fig)


def ensure_test_images(
    dark_path: str, light_path: str, size: tuple[int, int], force: bool
) -> None:
    """若测试图不存在或 force=True，则调用 test2 生成"""
    if force or not os.path.exists(dark_path):
        generate_dark_12bit(save_path=dark_path, size=size)
    if force or not os.path.exists(light_path):
        generate_light_12bit(save_path=light_path, size=size)


def run_calibration(
    dark_path: str = "dark_12bit.tif",
    light_path: str = "light_12bit.tif",
    row_count: int = 5,
    col_count: int = 12,
    ki_min: float = 0.6,
    ki_max: float = 1.5,
    auto_roi: bool = True,
    margin_ratio: float = 0.1,
    enable_3d: bool = True,
    output_dir: str = "output",
    generate_if_missing: bool = True,
    image_size: tuple[int, int] = (2048, 2048),
) -> tuple[list[float], list[float]]:
    """执行完整标定流程并输出结果"""
    os.makedirs(output_dir, exist_ok=True)

    if generate_if_missing:
        ensure_test_images(dark_path, light_path, image_size, force=False)

    # 1. 读取原始图像
    dark_raw = read_12bit_tiff(dark_path)
    light_raw = read_12bit_tiff(light_path)
    if dark_raw.shape != light_raw.shape:
        raise ValueError(
            f"暗场与亮场尺寸不一致：{dark_raw.shape} vs {light_raw.shape}"
        )

    # 2. 暗场校正
    corrected_img = dark_correct(light_raw, dark_raw)

    # 3. 框选 ROI
    if auto_roi:
        screen_roi = auto_select_roi(corrected_img, margin_ratio=margin_ratio)
        print(f"自动 ROI 模式：裁切后尺寸 {screen_roi.shape[1]}×{screen_roi.shape[0]}")
    else:
        screen_roi = select_roi(corrected_img)
        print(f"手动 ROI 模式：裁切后尺寸 {screen_roi.shape[1]}×{screen_roi.shape[0]}")

    # 4. 计算 Ki
    ki_result, gray_avg_result = calc_block_ki(
        roi_img=screen_roi,
        row_num=row_count,
        col_num=col_count,
        ki_min=ki_min,
        ki_max=ki_max,
    )

    # 5. 控制台输出
    print("=" * 70)
    print(f"【{row_count}行×{col_count}列 分区补偿系数 Ki（行优先）】")
    print("=" * 70)
    for row_idx in range(row_count):
        start = row_idx * col_count
        end = start + col_count
        row_data = ki_result[start:end]
        print(f"第{row_idx + 1}行：{row_data}")
    print("\n【一维完整数组（可直接写入设备/标签）】")
    print(ki_result)

    # 6. 保存文本结果
    result_txt = os.path.join(output_dir, "calibration_result.txt")
    with open(result_txt, "w", encoding="utf-8") as f:
        f.write("分区补偿系数Ki（行优先）：\n")
        f.write(str(ki_result))
        f.write("\n\n分区平均灰度：\n")
        f.write(str(gray_avg_result))
    print(f"\n计算完成，结果已保存到 {result_txt}")

    # 7. 3D 可视化（保存为图片）
    if enable_3d:
        plot_roi_3d_surface(
            screen_roi,
            save_path=os.path.join(output_dir, "roi_3d_surface.png"),
        )
        plot_block_3d(
            gray_avg_result,
            row_count,
            col_count,
            "分区平均灰度分布",
            "平均灰度值",
            save_path=os.path.join(output_dir, "block_gray_3d.png"),
        )
        plot_block_3d(
            ki_result,
            row_count,
            col_count,
            "分区补偿系数Ki分布",
            "Ki系数",
            save_path=os.path.join(output_dir, "block_ki_3d.png"),
        )

    return ki_result, gray_avg_result


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="LCD 光场均匀性标定")
    parser.add_argument("--dark", default="dark_12bit.tif", help="暗场图路径")
    parser.add_argument("--light", default="light_12bit.tif", help="亮场图路径")
    parser.add_argument("--rows", type=int, default=5, help="分区行数")
    parser.add_argument("--cols", type=int, default=12, help="分区列数")
    parser.add_argument("--ki-min", type=float, default=0.6, help="Ki 下限")
    parser.add_argument("--ki-max", type=float, default=1.5, help="Ki 上限")
    parser.add_argument(
        "--manual-roi",
        action="store_true",
        help="启用手动框选 ROI（默认自动裁切中心区域）",
    )
    parser.add_argument("--no-3d", action="store_true", help="跳过 3D 图输出")
    parser.add_argument(
        "--regenerate",
        action="store_true",
        help="强制重新生成测试 TIFF",
    )
    parser.add_argument("--output", default="output", help="结果输出目录")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    if args.regenerate:
        ensure_test_images(args.dark, args.light, (2048, 2048), force=True)

    run_calibration(
        dark_path=args.dark,
        light_path=args.light,
        row_count=args.rows,
        col_count=args.cols,
        ki_min=args.ki_min,
        ki_max=args.ki_max,
        auto_roi=not args.manual_roi,
        enable_3d=not args.no_3d,
        output_dir=args.output,
        generate_if_missing=True,
    )
