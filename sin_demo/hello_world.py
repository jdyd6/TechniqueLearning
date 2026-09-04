from pathlib import Path

import numpy as np
import tensorflow as tf

MODEL_PATH = Path(r"E:\Codes\TinyML\sin_demo\hello_world.tflite")

interpreter = tf.lite.Interpreter(model_path=str(MODEL_PATH))
interpreter.allocate_tensors()

inp = interpreter.get_input_details()[0]
out = interpreter.get_output_details()[0]

print("input :", inp["name"], inp["shape"], inp["dtype"], inp["quantization"])
print("output:", out["name"], out["shape"], out["dtype"], out["quantization"])


def quantize(value: float, detail: dict) -> np.ndarray:
    """按 tensor 的 scale / zero-point 把 float 变成模型要的整数。"""
    scale, zero_point = detail["quantization"]
    if detail["dtype"] == np.float32:
        data = np.array(value, dtype=np.float32)
    else:
        q = np.round(value / scale) + zero_point
        data = np.clip(q, np.iinfo(detail["dtype"]).min, np.iinfo(detail["dtype"]).max)
        data = data.astype(detail["dtype"])
    return data.reshape(detail["shape"])


def dequantize(tensor: np.ndarray, detail: dict) -> float:
    """把 INT8 输出还原成 float。"""
    scale, zero_point = detail["quantization"]
    if detail["dtype"] == np.float32:
        return float(tensor.squeeze())
    return float((int(tensor.squeeze()) - zero_point) * scale)


print(f"{'x':>8} {'model':>10} {'sin(x)':>10} {'error':>10}")
for x in np.linspace(0.0, 2.0 * np.pi, 13):
    interpreter.set_tensor(inp["index"], quantize(float(x), inp))
    interpreter.invoke()
    y = dequantize(interpreter.get_tensor(out["index"]), out)
    y_true = float(np.sin(x))
    print(f"{x:8.3f} {y:10.4f} {y_true:10.4f} {abs(y - y_true):10.4f}")



